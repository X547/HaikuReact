#include "ReactListView.h"

#include <Messenger.h>
#include <OS.h>
#include <Window.h>

#include "EventBridge.h"
#include "StatusCheck.h"


namespace HaikuReact {


constexpr uint32 kMsgSelect = 'HRsl';
constexpr const char *kFieldIndex = "index";


ReactListView::ReactListView(uint32 nodeId):
	BListView("ListView"),
	fNodeId(nodeId)
{
}


std::vector<int32> ReactListView::Selection() const
{
	std::vector<int32> selection;
	for (int32 i = 0; true; i++) {
		const int32 index = CurrentSelection(i);
		if (index < 0) {
			break;
		}
		selection.push_back(index);
	}
	return selection;
}


void ReactListView::SetSelection(const std::vector<int32> &selection)
{
	if (selection == Selection()) {
		return;
	}
	DeselectAll();
	for (const int32 index : selection) {
		if (index >= 0 && index < CountItems()) {
			Select(index, true);
		}
	}
}


void ReactListView::SelectionChanged()
{
	BListView::SelectionChanged();

	// Selections made from JS run on the JS thread; only those made on the
	// window thread come from the user.
	const BWindow *window = Window();
	if (window != nullptr && find_thread(nullptr) == window->Thread()) {
		EventBridge::Post(Event{fNodeId, "selectionChange", Selection()});
	}
}


void ReactListView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgSelect: {
			std::vector<int32> selection;
			int32 index = 0;
			for (int32 i = 0; message->FindInt32(kFieldIndex, i, &index) == B_OK; i++) {
				selection.push_back(index);
			}
			SetSelection(selection);
			break;
		}
		default:
			BListView::MessageReceived(message);
			break;
	}
}


void ReactListView::PostSelect(const std::vector<int32> &selection)
{
	BMessage message(kMsgSelect);
	for (const int32 index : selection) {
		message.AddInt32(kFieldIndex, index);
	}
	CheckThrow(BMessenger(this).SendMessage(&message));
}


} // HaikuReact
