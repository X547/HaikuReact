#include "ReactMenu.h"

#include <algorithm>

#include <Window.h>
#include <MenuItem.h>

#include "EventBridge.h"


namespace HaikuReact {


constexpr uint32 kMsgSyncItems = 'HRsy';
// How long the menu thread waits for the owner window before trying again.
constexpr bigtime_t kOwnerLockTimeout = 20000;


ReactMenu::ReactMenu(uint32 nodeId):
	BMenu(""),
	fNodeId(nodeId),
	fPlaceholder(new BMenuItem("", nullptr))
{
	fPlaceholder->SetEnabled(false);
}


ReactMenu::~ReactMenu()
{
	// ~BMenu deletes the items it holds: the children's (whose nodes no
	// longer own them) and the placeholder, which belongs to this menu.
	std::lock_guard guard(fLock);
	RemoveItem(fPlaceholder);
	delete fPlaceholder;
	for (BMenuItem *item : fRemoved) {
		RemoveItem(item);
		delete item;
	}
	// Children added while the menu was shown, before a sync.
	for (BMenuItem *item : fDesired) {
		if (IndexOf(item) < 0) {
			delete item;
		}
	}
}


void ReactMenu::ApplyLocked()
{
	std::vector<BMenuItem*> desired;
	if (fPlaceholderShown) {
		desired.push_back(fPlaceholder);
	}
	desired.insert(desired.end(), fDesired.begin(), fDesired.end());

	if (fPlaceholderLabel != fPlaceholder->Label()) {
		fPlaceholder->SetLabel(fPlaceholderLabel.c_str());
	}
	for (int32 i = CountItems() - 1; i >= 0; i--) {
		BMenuItem *item = ItemAt(i);
		if (std::find(desired.begin(), desired.end(), item) == desired.end()) {
			RemoveItem(item);
		}
	}
	for (size_t i = 0; i < desired.size(); i++) {
		const int32 index = static_cast<int32>(i);
		if (ItemAt(index) == desired[i]) {
			continue;
		}
		if (IndexOf(desired[i]) >= 0) {
			RemoveItem(desired[i]);
		}
		AddItem(desired[i], index);
	}
	for (BMenuItem *item : fRemoved) {
		delete item;
	}
	fRemoved.clear();
}


// Marks a sync as pending; returns where to send it, or an invalid
// messenger if one is pending already. The message is sent after fLock is
// released: sending can block while the menu window's port is full.
BMessenger ReactMenu::RequestSyncLocked()
{
	if (fSyncPending || Looper() == nullptr) {
		return BMessenger();
	}
	fSyncPending = true;
	return BMessenger(this);
}


void ReactMenu::SendSync(const BMessenger &target)
{
	if (!target.IsValid() || target.SendMessage(kMsgSyncItems) == B_OK) {
		return;
	}
	// The menu closed in between; it syncs when it opens again.
	std::lock_guard guard(fLock);
	fSyncPending = false;
}


void ReactMenu::SetContent(const std::vector<BMenuItem*> &items, bool placeholder,
	const std::string &placeholderLabel)
{
	BMessenger target;
	{
		std::lock_guard guard(fLock);
		fDesired = items;
		fPlaceholderShown = placeholder;
		fPlaceholderLabel = placeholderLabel;
		if (fShown) {
			target = RequestSyncLocked();
		} else {
			ApplyLocked();
		}
	}
	SendSync(target);
}


void ReactMenu::Adopt(BMenuItem *item)
{
	BMessenger target;
	{
		std::lock_guard guard(fLock);
		if (item->Menu() != this) {
			delete item;
			return;
		}
		fRemoved.push_back(item);
		if (fShown) {
			target = RequestSyncLocked();
		} else {
			ApplyLocked();
		}
	}
	SendSync(target);
}


bool ReactMenu::IsShown()
{
	std::lock_guard guard(fLock);
	return fShown;
}


std::vector<std::string> ReactMenu::ItemLabels()
{
	std::lock_guard guard(fLock);
	std::vector<std::string> labels;
	for (int32 i = 0; i < CountItems(); i++) {
		labels.push_back(ItemAt(i)->Label() != nullptr ? ItemAt(i)->Label() : "");
	}
	return labels;
}


// Menu window thread, with the menu window locked.
void ReactMenu::Sync()
{
	if (fOwner.LockTargetWithTimeout(kOwnerLockTimeout) != B_OK) {
		if (fOwner.IsValid()) {
			// The owner window is busy, possibly waiting for this window.
			Looper()->PostMessage(kMsgSyncItems, this);
		} else {
			std::lock_guard guard(fLock);
			fSyncPending = false;
		}
		return;
	}
	BLooper *owner = nullptr;
	fOwner.Target(&owner);
	{
		// Cleared before reading the state, so that a newer change posts
		// another sync.
		std::lock_guard guard(fLock);
		fSyncPending = false;
		if (fShown) {
			ApplyLocked();
		}
	}
	owner->Unlock();
}


void ReactMenu::AttachedToWindow()
{
	BMenu::AttachedToWindow();
	// Items are installed on the window of the menu bar (or pop-up menu)
	// at the root of the menu hierarchy.
	BMenu *root = this;
	while (root->Supermenu() != nullptr) {
		root = root->Supermenu();
	}
	BMessenger target;
	{
		std::lock_guard guard(fLock);
		fShown = true;
		fOwner = BMessenger(root->Window());
		fSyncPending = false;
		// Applies changes made while the menu was closed or being opened.
		target = RequestSyncLocked();
	}
	SendSync(target);
	EventBridge::Post(Event{fNodeId, "open", {}});
}


void ReactMenu::DetachedFromWindow()
{
	{
		std::lock_guard guard(fLock);
		fShown = false;
		// A sync still queued is dropped with the menu window.
		fSyncPending = false;
	}
	BMenu::DetachedFromWindow();
	EventBridge::Post(Event{fNodeId, "close", {}});
}


void ReactMenu::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgSyncItems:
			Sync();
			break;
		default:
			BMenu::MessageReceived(message);
			break;
	}
}


ReactMenuBar::ReactMenuBar():
	BMenuBar("MenuBar")
{
}


} // HaikuReact
