#include "ReactListView.h"

#include <stdlib.h>

#include <set>

#include <Messenger.h>
#include <OS.h>
#include <Window.h>

#include "ReactListItem.h"
#include "StatusCheck.h"


namespace HaikuReact {


constexpr uint32 kMsgSelect = 'HRsl';
constexpr uint32 kMsgExpand = 'HRex';
constexpr const char *kFieldId = "id";
constexpr const char *kFieldItem = "item";
constexpr const char *kFieldExpand = "expand";


static bool OnWindowThread(const BView &view)
{
	const BWindow *window = view.Window();
	return window != nullptr && find_thread(nullptr) == window->Thread();
}


ReactListView::ReactListView(uint32 nodeId):
	BOutlineListView("ListView"),
	fNodeId(nodeId)
{
}


EventValue ReactListView::IdsValue(const std::vector<std::string> &ids) const
{
	if (!fIndexIds) {
		return ids;
	}
	std::vector<int32> indices;
	for (const std::string &id : ids) {
		indices.push_back(atoi(id.c_str()));
	}
	return indices;
}


std::vector<std::string> ReactListView::Selection() const
{
	std::vector<std::string> selection;
	for (int32 i = 0; true; i++) {
		const int32 index = CurrentSelection(i);
		if (index < 0) {
			break;
		}
		const auto *item = static_cast<const ReactListItem*>(ItemAt(index));
		if (!item->IsPlaceholder()) {
			selection.push_back(item->ItemId());
		}
	}
	return selection;
}


void ReactListView::SetSelection(const std::vector<std::string> &ids)
{
	fWantedSelection = ids;
	RestoreSelection();
}


void ReactListView::RestoreSelection()
{
	if (fWantedSelection == Selection()) {
		return;
	}
	const std::set<std::string> wanted(fWantedSelection.begin(), fWantedSelection.end());
	DeselectAll();
	for (int32 i = 0; i < CountItems(); i++) {
		const auto *item = static_cast<const ReactListItem*>(ItemAt(i));
		if (!item->IsPlaceholder() && wanted.count(item->ItemId()) > 0) {
			Select(i, true);
		}
	}
}


void ReactListView::ItemsChanged()
{
	// Recomputes the heights and positions of the visible items.
	FrameResized(Bounds().Width(), Bounds().Height());
	Invalidate();
}


void ReactListView::SelectionChanged()
{
	BOutlineListView::SelectionChanged();

	// Selections made from JS run on the JS thread; only those made on the
	// window thread come from the user.
	if (OnWindowThread(*this)) {
		fWantedSelection = Selection();
		EventBridge::Post(Event{fNodeId, "selectionChange", IdsValue(fWantedSelection)});
	}
}


void ReactListView::ExpandOrCollapse(BListItem *item, bool expand)
{
	const bool changed = item->IsExpanded() != expand;
	BOutlineListView::ExpandOrCollapse(item, expand);

	const auto *reactItem = static_cast<const ReactListItem*>(item);
	if (changed && reactItem->NodeId() != 0 && OnWindowThread(*this)) {
		EventBridge::Post(Event{reactItem->NodeId(), expand ? "expand" : "collapse", {}});
	}
}


status_t ReactListView::Invoke(BMessage *message)
{
	(void)message;
	const int32 index = CurrentSelection();
	if (index < 0) {
		return B_OK;
	}
	const auto *item = static_cast<const ReactListItem*>(ItemAt(index));
	if (item->IsPlaceholder()) {
		return B_OK;
	}
	EventValue id = item->ItemId();
	if (fIndexIds) {
		id = static_cast<int32>(atoi(item->ItemId().c_str()));
	}
	EventBridge::Post(Event{fNodeId, "invoke", id});
	return B_OK;
}


void ReactListView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgSelect: {
			std::vector<std::string> ids;
			const char *id = nullptr;
			for (int32 i = 0; message->FindString(kFieldId, i, &id) == B_OK; i++) {
				ids.push_back(id);
			}
			SetSelection(ids);
			break;
		}
		case kMsgExpand: {
			// The item may be gone; only its address was sent.
			void *item = nullptr;
			if (message->FindPointer(kFieldItem, &item) == B_OK
				&& FullListHasItem(static_cast<BListItem*>(item))) {
				ExpandOrCollapse(static_cast<BListItem*>(item), message->GetBool(kFieldExpand, true));
			}
			break;
		}
		default:
			BOutlineListView::MessageReceived(message);
			break;
	}
}


void ReactListView::PostSelect(const std::vector<std::string> &ids)
{
	BMessage message(kMsgSelect);
	for (const std::string &id : ids) {
		message.AddString(kFieldId, id.c_str());
	}
	CheckThrow(BMessenger(this).SendMessage(&message));
}


void ReactListView::PostExpand(ReactListItem *item, bool expand)
{
	BMessage message(kMsgExpand);
	message.AddPointer(kFieldItem, item);
	message.AddBool(kFieldExpand, expand);
	CheckThrow(BMessenger(this).SendMessage(&message));
}


} // HaikuReact
