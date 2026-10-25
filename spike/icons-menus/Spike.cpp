// Phase 0 spike for images/icons, column lists and menus filled while open.
//
// The main thread plays the JS thread of the add-on: BApplication runs on a
// spawned thread and windows are changed from the main thread under their
// lock. Usage:
//   icons-menus-spike            automated checks, exit code = failures
//   icons-menus-spike --manual   menu "Lazy" for testing with the mouse

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <atomic>
#include <string>
#include <vector>

#include <Application.h>
#include <Bitmap.h>
#include <File.h>
#include <FindDirectory.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <MimeType.h>
#include <NodeInfo.h>
#include <OS.h>
#include <Path.h>
#include <Resources.h>
#include <StringView.h>
#include <TranslationUtils.h>
#include <Window.h>

#include <ColumnListView.h>
#include <ColumnTypes.h>


constexpr uint32 kMsgSyncItems = 'Sync';
constexpr const char *kLoadingLabel = "Loading…";

static int32 sFailures = 0;
static sem_id sReadySem = -1;
static sem_id sMenuOpenSem = -1;
static sem_id sMenuClosedSem = -1;


static void Report(bool ok, const char *check, const std::string &detail)
{
	printf("%s  %-44s %s\n", ok ? "PASS" : "FAIL", check, detail.c_str());
	if (!ok) {
		sFailures++;
	}
}


static bool AcquireWithTimeout(sem_id sem, bigtime_t timeout)
{
	status_t status;
	do {
		status = acquire_sem_etc(sem, 1, B_RELATIVE_TIMEOUT, timeout);
	} while (status == B_INTERRUPTED);
	return status == B_OK;
}


//#pragma mark - Icons and images

static int32 CountOpaquePixels(const BBitmap &bitmap)
{
	const auto *bits = static_cast<const uint8*>(bitmap.Bits());
	int32 count = 0;
	for (size_t i = 0; i < static_cast<size_t>(bitmap.BitsLength()); i += 4) {
		if (bits[i + 3] != 0) {
			count++;
		}
	}
	return count;
}


static std::string Describe(const BBitmap &bitmap)
{
	const BRect bounds = bitmap.Bounds();
	return std::to_string(bounds.IntegerWidth() + 1) + "x" + std::to_string(bounds.IntegerHeight() + 1)
		+ ", " + std::to_string(CountOpaquePixels(bitmap)) + " opaque pixels";
}


static void CheckSystemIcons()
{
	BPath path;
	find_directory(B_SYSTEM_LIB_DIRECTORY, &path);
	path.Append("libbe.so");
	BFile file(path.Path(), B_READ_ONLY);
	BResources resources(&file);

	std::vector<std::string> names;
	int32 flags = 0;
	int32 id = 0;
	const char *name = nullptr;
	size_t size = 0;
	for (int32 i = 0; resources.GetResourceInfo(B_VECTOR_ICON_TYPE, i, &id, &name, &size); i++) {
		if (strncmp(name, "flag-", 5) == 0) {
			flags++;
		} else {
			names.push_back(name);
		}
	}
	std::string list;
	for (const std::string &item : names) {
		list += item + " ";
	}
	Report(!names.empty(), "system icons (libbe VICN resources)", list + "+ " + std::to_string(flags) + " flags");

	for (const float iconSize : {16.0f, 32.0f, 64.0f}) {
		BBitmap bitmap(BRect(0, 0, iconSize - 1, iconSize - 1), B_RGBA32);
		const status_t status = BIconUtils::GetSystemIcon("dialog-warning", &bitmap);
		Report(
			status == B_OK && CountOpaquePixels(bitmap) > 0,
			("GetSystemIcon(\"dialog-warning\") at " + std::to_string(int(iconSize))).c_str(),
			Describe(bitmap)
		);
	}
	// Not calling GetSystemIcon() with an unknown name: it deletes the
	// caller's bitmap before returning B_ENTRY_NOT_FOUND.
}


static void CheckMimeIcon(const char *type)
{
	BMimeType mimeType(type);
	uint8 *data = nullptr;
	size_t size = 0;
	status_t status = mimeType.GetIcon(&data, &size);
	if (status != B_OK) {
		// Fall back to the supertype, as Tracker does.
		BMimeType superType;
		if (mimeType.GetSupertype(&superType) == B_OK) {
			status = superType.GetIcon(&data, &size);
		}
	}
	BBitmap bitmap(BRect(0, 0, 31, 31), B_RGBA32);
	if (status == B_OK) {
		status = BIconUtils::GetVectorIcon(data, size, &bitmap);
	}
	delete[] data;
	Report(status == B_OK && CountOpaquePixels(bitmap) > 0,
		(std::string("MIME icon ") + type).c_str(), status == B_OK ? Describe(bitmap) : strerror(status));
}


static void CheckTrackerIcon(const char *filePath)
{
	entry_ref ref;
	status_t status = get_ref_for_path(filePath, &ref);
	BBitmap bitmap(BRect(0, 0, 47, 47), B_RGBA32);
	if (status == B_OK) {
		status = BNodeInfo::GetTrackerIcon(&ref, &bitmap, static_cast<icon_size>(48));
	}
	Report(status == B_OK && CountOpaquePixels(bitmap) > 0,
		(std::string("Tracker icon ") + filePath + " at 48").c_str(), status == B_OK ? Describe(bitmap) : strerror(status));
}


static void CheckRasterFromMemory(const char *filePath)
{
	BFile file(filePath, B_READ_ONLY);
	off_t size = 0;
	file.GetSize(&size);
	std::vector<uint8> bytes(size);
	file.Read(bytes.data(), bytes.size());

	BMemoryIO input(bytes.data(), bytes.size());
	BBitmap *bitmap = BTranslationUtils::GetBitmap(&input);
	Report(bitmap != nullptr && bitmap->IsValid(),
		(std::string("decode from memory ") + strrchr(filePath, '/')).c_str(),
		bitmap != nullptr ? Describe(*bitmap) + ", " + std::to_string(bytes.size()) + " bytes" : "failed");
	delete bitmap;
}


//#pragma mark - LazyMenu

// Menu whose items are replaced on its own (menu window) thread. While closed
// it shows a "Loading…" item.
class LazyMenu: public BMenu {
private:
	int32 fOpenCount = 0;

	void ShowPlaceholder();

public:
	LazyMenu();

	int32 OpenCount() const {return fOpenCount;}

	void AttachedToWindow() final;
	void DetachedFromWindow() final;
	void MessageReceived(BMessage *message) final;
};


LazyMenu::LazyMenu():
	BMenu("Lazy")
{
	ShowPlaceholder();
}


void LazyMenu::ShowPlaceholder()
{
	RemoveItems(0, CountItems(), true);
	auto *placeholder = new BMenuItem(kLoadingLabel, nullptr);
	placeholder->SetEnabled(false);
	AddItem(placeholder);
}


void LazyMenu::AttachedToWindow()
{
	BMenu::AttachedToWindow();
	fOpenCount++;
	release_sem(sMenuOpenSem);
}


void LazyMenu::DetachedFromWindow()
{
	BMenu::DetachedFromWindow();
	release_sem(sMenuClosedSem);
}


// Latest item list requested by the "JS thread"; at most one sync message is
// pending, so the menu thread always applies the newest state at once.
static std::atomic<int32> sDesiredCount {0};
static std::atomic<bool> sSyncPending {false};


static void RequestSync(LazyMenu *menu, int32 count)
{
	sDesiredCount = count;
	if (!sSyncPending.exchange(true)) {
		BMessenger(menu).SendMessage(kMsgSyncItems);
	}
}


void LazyMenu::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgSyncItems: {
			// Runs on the menu window's thread, which holds its lock. Clear
			// the flag before reading, so a newer request posts again.
			sSyncPending = false;
			const int32 count = sDesiredCount;
			RemoveItems(0, CountItems(), true);
			for (int32 i = 0; i < count; i++) {
				AddItem(new BMenuItem(("Item " + std::to_string(i + 1)).c_str(), new BMessage('item')));
			}
			break;
		}
		default:
			BMenu::MessageReceived(message);
			break;
	}
}


//#pragma mark - SpikeMenuBar

class SpikeMenuBar: public BMenuBar {
public:
	SpikeMenuBar():
		BMenuBar("MenuBar")
	{
	}

	// Opens the first menu like Command+Escape and Down would: sticky,
	// without a mouse button. Window locked by the caller.
	void OpenFirstMenu() {StartMenuBar(0, true, true);}
};


//#pragma mark - SpikeColumnListView

class SpikeColumnListView: public BColumnListView {
private:
	int32 fExpandCount = 0;

public:
	SpikeColumnListView();

	int32 ExpandCount() const {return fExpandCount;}

	void ExpandOrCollapse(BRow *row, bool expand) final;
};


SpikeColumnListView::SpikeColumnListView():
	BColumnListView("Columns", 0, B_FANCY_BORDER, true)
{
}


void SpikeColumnListView::ExpandOrCollapse(BRow *row, bool expand)
{
	BColumnListView::ExpandOrCollapse(row, expand);
	if (expand) {
		fExpandCount++;
	}
}


//#pragma mark - Application

class SpikeApplication: public BApplication {
public:
	SpikeApplication():
		BApplication("application/x-vnd.HaikuReact-IconsMenusSpike")
	{
	}

	void ReadyToRun() final {release_sem(sReadySem);}
};


static status_t AppThread(void *data)
{
	(void)data;
	SpikeApplication app;
	app.Run();
	// Ends the wait of the manual mode.
	delete_sem(sMenuOpenSem);
	return B_OK;
}


struct SpikeWindow {
	BWindow *window = nullptr;
	SpikeMenuBar *menuBar = nullptr;
	LazyMenu *lazyMenu = nullptr;
	BStringView *status = nullptr;
	SpikeColumnListView *columns = nullptr;
	BRow *parentRow = nullptr;
	BMessenger windowMessenger;
};


static SpikeWindow CreateWindow()
{
	SpikeWindow result;
	result.window = new BWindow(BRect(100, 100, 499, 399), "Icons & menus spike", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE);
	result.menuBar = new SpikeMenuBar();
	result.lazyMenu = new LazyMenu();
	result.menuBar->AddItem(result.lazyMenu);
	result.status = new BStringView("Status", "Ready");
	result.columns = new SpikeColumnListView();

	result.columns->AddColumn(new BStringColumn("Name", 160, 50, 500, B_TRUNCATE_END), 0);
	result.columns->AddColumn(new BSizeColumn("Size", 80, 50, 200), 1);
	result.parentRow = new BRow();
	result.parentRow->SetField(new BStringField("Folder"), 0);
	result.parentRow->SetField(new BSizeField(0), 1);
	result.columns->AddRow(result.parentRow);
	for (int32 i = 0; i < 3; i++) {
		auto *child = new BRow();
		child->SetField(new BStringField(("File " + std::to_string(i)).c_str()), 0);
		child->SetField(new BSizeField(1024 * (i + 1)), 1);
		result.columns->AddRow(child, result.parentRow);
	}

	BLayoutBuilder::Group<>(result.window, B_VERTICAL, 0)
		.Add(result.menuBar)
		.AddGroup(B_VERTICAL)
			.SetInsets(B_USE_WINDOW_SPACING)
			.Add(result.status)
			.Add(result.columns)
		.End();
	result.window->Run();
	result.windowMessenger = BMessenger(result.window);
	return result;
}


//#pragma mark - Checks needing a window

static void CheckColumnListView(SpikeWindow &spike)
{
	spike.window->Lock();
	spike.columns->ExpandOrCollapse(spike.parentRow, true);
	const int32 topLevel = spike.columns->CountRows();
	const int32 children = spike.columns->CountRows(spike.parentRow);
	const BRect frame = spike.columns->Frame();
	const int32 expandCount = spike.columns->ExpandCount();
	spike.window->Unlock();

	Report(topLevel == 1 && children == 3, "BColumnListView nested rows",
		std::to_string(topLevel) + " top-level, " + std::to_string(children) + " children");
	Report(expandCount == 1, "ExpandOrCollapse() hook", std::to_string(expandCount) + " expand call(s)");
	Report(frame.Width() > 50 && frame.Height() > 50, "BColumnListView in layout",
		std::to_string(frame.IntegerWidth() + 1) + "x" + std::to_string(frame.IntegerHeight() + 1));
}


static int32 MenuItemCount(LazyMenu *menu, std::string *firstLabel, float *windowHeight)
{
	if (menu->LockLooperWithTimeout(1000000) != B_OK) {
		return -1;
	}
	const int32 count = menu->CountItems();
	if (firstLabel != nullptr && count > 0) {
		*firstLabel = menu->ItemAt(0)->Label();
	}
	if (windowHeight != nullptr) {
		*windowHeight = menu->Window() != nullptr ? menu->Window()->Frame().Height() : -1;
	}
	menu->UnlockLooper();
	return count;
}


static void CheckLazyMenu(SpikeWindow &spike)
{
	// Open the menu like Command+Escape would: sticky, without mouse button.
	spike.window->Lock();
	spike.menuBar->OpenFirstMenu();
	spike.window->Unlock();
	const bool opened = AcquireWithTimeout(sMenuOpenSem, 3000000);
	Report(opened, "menu opens, AttachedToWindow() reports it", opened ? "" : "timeout");
	if (!opened) {
		return;
	}

	std::string firstLabel;
	float placeholderHeight = 0;
	const int32 placeholderCount = MenuItemCount(spike.lazyMenu, &firstLabel, &placeholderHeight);
	Report(placeholderCount == 1 && firstLabel == kLoadingLabel, "open menu shows the placeholder",
		std::to_string(placeholderCount) + " item(s), \"" + firstLabel + "\"");

	// "React commits" while the menu is open: lock the main window, change a
	// view and the menu bar, and send the new item list to the menu thread.
	bigtime_t maxWait = 0;
	int32 lastCount = 0;
	for (int32 i = 0; i < 200; i++) {
		const bigtime_t start = system_time();
		spike.window->Lock();
		maxWait = std::max(maxWait, system_time() - start);
		spike.status->SetText(("Commit " + std::to_string(i)).c_str());
		spike.lazyMenu->Superitem()->SetLabel(i % 2 == 0 ? "Lazy" : "Lazy ");
		lastCount = 5 + i % 6;
		RequestSync(spike.lazyMenu, lastCount);
		spike.window->Unlock();
		snooze(2000);
	}
	snooze(100000);
	float itemsHeight = 0;
	const int32 syncedCount = MenuItemCount(spike.lazyMenu, &firstLabel, &itemsHeight);
	Report(syncedCount == lastCount && firstLabel == "Item 1", "items sent to the open menu's thread",
		std::to_string(syncedCount) + " items, first \"" + firstLabel + "\"");
	Report(itemsHeight > placeholderHeight, "open menu window grows with its items",
		std::to_string(int(placeholderHeight)) + " -> " + std::to_string(int(itemsHeight)));
	Report(maxWait < 200000, "main window lock never blocked long", std::to_string(maxWait) + " us max wait");

	// Diagnostic: the direct alternative, main window then menu window lock.
	int32 directOk = 0;
	int32 directTimeouts = 0;
	for (int32 i = 0; i < 50; i++) {
		spike.window->Lock();
		if (spike.lazyMenu->LockLooperWithTimeout(200000) == B_OK) {
			directOk++;
			spike.lazyMenu->UnlockLooper();
		} else {
			directTimeouts++;
		}
		spike.window->Unlock();
	}
	printf("INFO  direct main->menu window locking: %d ok, %d timeouts\n", int(directOk), int(directTimeouts));

	printf("INFO  menu open with items; holding 3 s for a screenshot\n");
	fflush(stdout);
	snooze(3000000);
}


static void SendEscape(BHandler *target)
{
	BMessage escape(B_KEY_DOWN);
	escape.AddString("bytes", "\x1b");
	escape.AddInt32("raw_char", B_ESCAPE);
	escape.AddInt32("modifiers", 0);
	BMessenger(target).SendMessage(&escape);
}


// Escape in a keyboard-opened menu returns to the menu bar; a second Escape
// leaves the menu bar.
static bool CloseMenus(SpikeWindow &spike)
{
	SendEscape(spike.lazyMenu);
	const bool closed = AcquireWithTimeout(sMenuClosedSem, 3000000);
	SendEscape(spike.menuBar);
	snooze(200000);
	return closed;
}


static void CheckCloseMenu(SpikeWindow &spike)
{
	const bool closed = CloseMenus(spike);
	Report(closed, "Escape closes the menu, DetachedFromWindow()", closed ? "" : "timeout");
}


static void CheckRepeatedOpenClose(SpikeWindow &spike)
{
	const int32 windowsBefore = be_app->CountWindows();
	int32 cycles = 0;
	for (; cycles < 20; cycles++) {
		spike.window->Lock();
		spike.menuBar->OpenFirstMenu();
		spike.window->Unlock();
		if (!AcquireWithTimeout(sMenuOpenSem, 3000000)) {
			break;
		}
		RequestSync(spike.lazyMenu, 3 + cycles % 4);
		if (!CloseMenus(spike)) {
			break;
		}
	}
	snooze(200000);
	Report(cycles == 20 && be_app->CountWindows() == windowsBefore, "20 open/sync/close cycles",
		std::to_string(cycles) + " cycles, windows " + std::to_string(windowsBefore) + " -> "
			+ std::to_string(be_app->CountWindows()));
}


static void RunManual(SpikeWindow &spike)
{
	printf("Manual mode: open the \"Lazy\" menu with the mouse (click, or press and hold).\n"
		"It shows \"Loading…\" and fills after 1 s. Close the window to quit.\n");
	fflush(stdout);
	while (true) {
		if (acquire_sem(sMenuOpenSem) != B_OK) {
			break;
		}
		snooze(1000000);
		// The window may have been closed during the delay. Lock through the
		// messenger: it fails once the window is gone, and while it is held
		// the window and the menus it owns stay alive.
		if (!spike.windowMessenger.LockTarget()) {
			break;
		}
		RequestSync(spike.lazyMenu, 8);
		spike.window->Unlock();
	}
}


int main(int argc, char **argv)
{
	const bool manual = argc > 1 && strcmp(argv[1], "--manual") == 0;
	sReadySem = create_sem(0, "ready");
	sMenuOpenSem = create_sem(0, "menu open");
	sMenuClosedSem = create_sem(0, "menu closed");

	const thread_id appThread = spawn_thread(AppThread, "spike app", B_NORMAL_PRIORITY, nullptr);
	resume_thread(appThread);
	acquire_sem(sReadySem);

	CheckSystemIcons();
	CheckMimeIcon("application/x-vnd.Be-directory");
	CheckMimeIcon("text/plain");
	CheckMimeIcon("text/x-made-up-subtype");
	CheckTrackerIcon("/boot/home");
	CheckTrackerIcon("/boot/system/apps/StyledEdit");
	CheckRasterFromMemory("/boot/system/data/cmake/Templates/Windows/StoreLogo.png");
	CheckRasterFromMemory("/boot/system/data/gutenprint/samples/profile.jpg");

	SpikeWindow spike = CreateWindow();
	spike.window->Lock();
	spike.window->Show();
	spike.window->Unlock();
	snooze(300000);

	if (manual) {
		RunManual(spike);
	} else {
		CheckColumnListView(spike);
		CheckLazyMenu(spike);
		CheckCloseMenu(spike);
		CheckRepeatedOpenClose(spike);
		spike.window->Lock();
		spike.window->Quit();
		be_app->PostMessage(B_QUIT_REQUESTED);
	}

	status_t exitValue;
	wait_for_thread(appThread, &exitValue);
	printf("%d failure(s)\n", int(sFailures));
	return sFailures;
}
