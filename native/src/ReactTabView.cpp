#include "ReactTabView.h"

#include <Messenger.h>
#include <OS.h>
#include <Window.h>

#include "EventBridge.h"
#include "StatusCheck.h"


namespace HaikuReact {


constexpr uint32 kMsgSelectNext = 'HRsn';


ReactTabView::ReactTabView(uint32 nodeId):
	BTabView("Tabs", B_WIDTH_FROM_LABEL),
	fNodeId(nodeId)
{
}


void ReactTabView::Select(int32 index)
{
	const int32 previous = Selection();
	BTabView::Select(index);

	// Selections made from JS run on the JS thread; only those made on the
	// window thread come from the user.
	const BWindow *window = Window();
	if (Selection() != previous && window != nullptr && find_thread(nullptr) == window->Thread()) {
		EventBridge::Post(Event{fNodeId, "change", Selection()});
	}
}


void ReactTabView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgSelectNext:
			if (CountTabs() > 0) {
				Select((Selection() + 1) % CountTabs());
			}
			break;
		default:
			BTabView::MessageReceived(message);
			break;
	}
}


void ReactTabView::PostSelectNext()
{
	CheckThrow(BMessenger(this).SendMessage(kMsgSelectNext));
}


} // HaikuReact
