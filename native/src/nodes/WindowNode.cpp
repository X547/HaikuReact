#include "WindowNode.h"

#include <stdexcept>
#include <string>

#include <GroupLayout.h>
#include <Messenger.h>
#include <OS.h>

#include "../CommitScope.h"
#include "../StatusCheck.h"
#include "../GroupLayoutChildren.h"
#include "../PropSet.h"
#include "../ReactWindow.h"


namespace HaikuReact {


// The root group has no spacing or insets, so a menu bar can span the window.
static const GroupLayoutChildren::Defaults kWindowDefaults = {
	.direction = B_VERTICAL,
	.spacing = 0,
	.insets = {}
};


constexpr uint32 kBaseWindowFlags = B_AUTO_UPDATE_SIZE_LIMITS | B_ASYNCHRONOUS_CONTROLS;


template<typename T>
struct WindowName {
	const char *name;
	T value;
};

static constexpr WindowName<window_look> kLooks[] = {
	{"titled",   B_TITLED_WINDOW_LOOK},
	{"document", B_DOCUMENT_WINDOW_LOOK},
	{"modal",    B_MODAL_WINDOW_LOOK},
	{"floating", B_FLOATING_WINDOW_LOOK},
	{"bordered", B_BORDERED_WINDOW_LOOK},
	{"none",     B_NO_BORDER_WINDOW_LOOK},
};

static constexpr WindowName<window_feel> kFeels[] = {
	{"normal",       B_NORMAL_WINDOW_FEEL},
	{"modal-app",    B_MODAL_APP_WINDOW_FEEL},
	{"modal-all",    B_MODAL_ALL_WINDOW_FEEL},
	{"floating-app", B_FLOATING_APP_WINDOW_FEEL},
	{"floating-all", B_FLOATING_ALL_WINDOW_FEEL},
};


template<typename T, size_t N>
static T ParseWindowName(const char *prop, const std::string &name, const WindowName<T> (&names)[N])
{
	for (const WindowName<T> &item : names) {
		if (name == item.name) {
			return item.value;
		}
	}
	throw std::invalid_argument(std::string("prop \"") + prop + "\": unknown value \"" + name + "\"");
}


template<typename T, size_t N>
static const char *WindowNameOf(T value, const WindowName<T> (&names)[N])
{
	for (const WindowName<T> &item : names) {
		if (value == item.value) {
			return item.name;
		}
	}
	return "";
}


static uint32 FlagsFor(const WindowBehavior &behavior)
{
	uint32 flags = kBaseWindowFlags;
	if (!behavior.resizable) {
		flags |= B_NOT_RESIZABLE;
	}
	if (!behavior.zoomable) {
		flags |= B_NOT_ZOOMABLE;
	}
	if (!behavior.closable) {
		flags |= B_NOT_CLOSABLE;
	}
	if (!behavior.minimizable) {
		flags |= B_NOT_MINIMIZABLE;
	}
	if (!behavior.movable) {
		flags |= B_NOT_MOVABLE;
	}
	if (behavior.closeOnEscape) {
		flags |= B_CLOSE_ON_ESCAPE;
	}
	return flags;
}


WindowNode::WindowNode(uint32 id):
	Node(id),
	fWindow(new ReactWindow(id)),
	fLayout(new BGroupLayout(B_VERTICAL, 0))
{
	// The window is constructed locked by this thread; Run() starts its
	// thread and releases the lock while the window stays hidden.
	try {
		fWindow->SetLayout(fLayout);
		CheckThrow(fWindow->Run());
	} catch (...) {
		fWindow->Quit();
		throw;
	}
}


WindowNode::~WindowNode()
{
	// Quit() from a foreign thread fully unlocks, lets the window thread
	// drain its queue and waits for it. Child nodes do not delete their
	// views afterwards because they still have a parent.
	CommitScope::Forget(fWindow);
	if (fWindow->Lock()) {
		fWindow->Quit();
	}
}


BLooper *WindowNode::Looper() const
{
	return fWindow;
}


void WindowNode::AttachChild(Node &child, int32 index)
{
	GroupLayoutChildren::Attach(*fLayout, child, index);
}


void WindowNode::DetachChild(Node &child)
{
	GroupLayoutChildren::Detach(*fLayout, child);
}


void WindowNode::ChildLayoutHintsChanged(Node &child, int32 index)
{
	GroupLayoutChildren::UpdateHints(*fLayout, child, index);
}


void WindowNode::ApplyOwnProps(const PropSet &props)
{
	if (auto title = props.Get<std::string>("title", "")) {
		fWindow->SetTitle(title->c_str());
	}
	if (GroupLayoutChildren::ApplyProps(*fLayout, props, kWindowDefaults)) {
		NotifyChildrenOfOrientation(fLayout->Orientation());
	}

	bool frameChanged = false;
	if (auto left = props.Get<std::optional<float>>("left", std::nullopt)) {
		fLeft = *left;
		frameChanged = true;
	}
	if (auto top = props.Get<std::optional<float>>("top", std::nullopt)) {
		fTop = *top;
		frameChanged = true;
	}
	if (auto width = props.Get<std::optional<float>>("width", std::nullopt)) {
		fWidth = *width;
		frameChanged = true;
	}
	if (auto height = props.Get<std::optional<float>>("height", std::nullopt)) {
		fHeight = *height;
		frameChanged = true;
	}
	if (frameChanged && fMounted) {
		ApplyFrame();
	}

	ApplyLookAndFeel(props);

	if (auto visible = props.Get<bool>("visible", true)) {
		fVisible = *visible;
		UpdateVisibility();
	}
}


void WindowNode::ApplyLookAndFeel(const PropSet &props)
{
	if (auto look = props.Get<std::string>("look", "titled")) {
		fWindow->SetLook(ParseWindowName("look", *look, kLooks));
	}
	if (auto feel = props.Get<std::string>("feel", "normal")) {
		fWindow->SetFeel(ParseWindowName("feel", *feel, kFeels));
	}

	const WindowBehavior defaults;
	bool flagsChanged = false;
	const auto update = [&props, &flagsChanged](const char *key, bool &value, bool defaultValue) {
		if (auto newValue = props.Get<bool>(key, defaultValue)) {
			value = *newValue;
			flagsChanged = true;
		}
	};
	update("resizable", fBehavior.resizable, defaults.resizable);
	update("zoomable", fBehavior.zoomable, defaults.zoomable);
	update("closable", fBehavior.closable, defaults.closable);
	update("minimizable", fBehavior.minimizable, defaults.minimizable);
	update("movable", fBehavior.movable, defaults.movable);
	update("closeOnEscape", fBehavior.closeOnEscape, defaults.closeOnEscape);
	if (flagsChanged) {
		fWindow->SetFlags(FlagsFor(fBehavior));
	}
}


void WindowNode::ApplyFrame()
{
	const BRect frame = fWindow->Frame();
	if (fWidth.has_value() || fHeight.has_value()) {
		fWindow->ResizeTo(
			fWidth.value_or(frame.Width() + 1) - 1,
			fHeight.value_or(frame.Height() + 1) - 1
		);
	}
	if (fLeft.has_value() || fTop.has_value()) {
		fWindow->MoveTo(fLeft.value_or(frame.left), fTop.value_or(frame.top));
	}
}


void WindowNode::UpdateVisibility()
{
	if (!fMounted || fVisible == !fWindow->IsHidden()) {
		return;
	}
	if (fVisible) {
		fWindow->Show();
	} else {
		fWindow->Hide();
	}
}


void WindowNode::Mount()
{
	if (fMounted) {
		return;
	}
	if (!fWidth.has_value() && !fHeight.has_value()) {
		fWindow->ResizeToPreferred();
	}
	ApplyFrame();
	if (!fLeft.has_value() && !fTop.has_value()) {
		fWindow->CenterOnScreen();
	}
	fMounted = true;
	UpdateVisibility();
}


void WindowNode::Invoke()
{
	CheckThrow(fWindow->PostMessage(B_QUIT_REQUESTED));
}


void WindowNode::SimulateKey(const std::string &bytes, uint32 modifiers)
{
	BMessage keyDown(B_KEY_DOWN);
	keyDown.AddInt64("when", system_time());
	keyDown.AddString("bytes", bytes.c_str());
	keyDown.AddInt32("modifiers", static_cast<int32>(modifiers));
	keyDown.AddInt32("raw_char", bytes.empty() ? 0 : bytes[0]);
	keyDown.AddInt32("key", 0);
	// Keyboard messages go to the preferred handler (the focus view); only
	// those are checked for shortcuts.
	CheckThrow(BMessenger(nullptr, fWindow).SendMessage(&keyDown));
}


void WindowNode::Dump(Napi::Object &out) const
{
	Node::Dump(out);

	Napi::Env env = out.Env();
	const BRect frame = fWindow->Frame();
	Napi::Object frameOut = Napi::Object::New(env);
	frameOut.Set("x", frame.left);
	frameOut.Set("y", frame.top);
	frameOut.Set("width", frame.Width() + 1);
	frameOut.Set("height", frame.Height() + 1);
	out.Set("frame", frameOut);
	out.Set("title", fWindow->Title());
	out.Set("hidden", fWindow->IsHidden());
	out.Set("active", fWindow->IsActive());
	out.Set("look", WindowNameOf(fWindow->Look(), kLooks));
	out.Set("feel", WindowNameOf(fWindow->Feel(), kFeels));

	const uint32 flags = fWindow->Flags();
	out.Set("resizable", (flags & B_NOT_RESIZABLE) == 0);
	out.Set("closable", (flags & B_NOT_CLOSABLE) == 0);
	out.Set("closeOnEscape", (flags & B_CLOSE_ON_ESCAPE) != 0);
}


} // HaikuReact
