#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <Menu.h>
#include <MenuBar.h>
#include <Messenger.h>


namespace HaikuReact {


// Menu backing <Menu>. Reports opening and closing (events open, close) and
// can show a disabled "Loading…" item.
//
// While the menu is shown, its window has its own thread and the JS thread
// does not touch its items: SetContent() stores the desired item list and
// posts one sync message, and the menu applies the newest list on its own
// thread (coalescing quick updates). Items are installed on the owner
// window (shortcuts, message target), so the sync also holds that window's
// lock; it is taken with a timeout and retried, never waited for.
class ReactMenu: public BMenu {
private:
	const uint32 fNodeId;
	BMessenger fOwner;

	// Guards everything below and all changes of the item list.
	std::mutex fLock;
	std::vector<BMenuItem*> fDesired;
	bool fPlaceholderShown = false;
	std::string fPlaceholderLabel;
	BMenuItem *fPlaceholder;
	// Items of deleted nodes that the open menu still shows.
	std::vector<BMenuItem*> fRemoved;
	bool fShown = false;
	bool fSyncPending = false;

	void ApplyLocked();
	BMessenger RequestSyncLocked();
	void SendSync(const BMessenger &target);
	void Sync();

public:
	explicit ReactMenu(uint32 nodeId);
	~ReactMenu() final;

	// JS thread, owner window locked. The items are the children's, in order.
	void SetContent(const std::vector<BMenuItem*> &items, bool placeholder, const std::string &placeholderLabel);
	// Takes an item of a deleted node: deleted now, or once the open menu
	// stopped showing it.
	void Adopt(BMenuItem *item);

	bool IsShown();
	// Labels of the items the menu holds, for dumps.
	std::vector<std::string> ItemLabels();

	void AttachedToWindow() final;
	void DetachedFromWindow() final;
	void MessageReceived(BMessage *message) final;
};


// Menu bar that tests can open from the keyboard side, like Command+Escape.
class ReactMenuBar: public BMenuBar {
public:
	ReactMenuBar();

	// Opens the menu at index, sticky (as from the keyboard). Window locked.
	void OpenMenu(int32 index) {StartMenuBar(index, true, true);}
};


} // HaikuReact
