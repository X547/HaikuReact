#include "ReactWindow.h"

#include "EventBridge.h"
#include "NodeEventMessage.h"


namespace HaikuReact {


ReactWindow::ReactWindow(uint32 nodeId):
	BWindow(
		BRect(0, 0, 99, 99),
		"",
		B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_ASYNCHRONOUS_CONTROLS
	),
	fNodeId(nodeId)
{
}


void ReactWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgNodeEvent:
			EventBridge::Post(EventFromNodeMessage(*message, *this));
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool ReactWindow::QuitRequested()
{
	EventBridge::Post(Event{fNodeId, "closeRequest", {}});
	return false;
}


void ReactWindow::WindowActivated(bool active)
{
	BWindow::WindowActivated(active);
	EventBridge::Post(Event{fNodeId, "activate", active});
}


void ReactWindow::FrameMoved(BPoint newPosition)
{
	BWindow::FrameMoved(newPosition);
	EventBridge::Post(Event{
		fNodeId,
		"move",
		std::vector<int32>{static_cast<int32>(newPosition.x), static_cast<int32>(newPosition.y)}
	});
}


void ReactWindow::FrameResized(float newWidth, float newHeight)
{
	BWindow::FrameResized(newWidth, newHeight);
	// Reported in pixels, like the width and height props.
	EventBridge::Post(Event{
		fNodeId,
		"resize",
		std::vector<int32>{static_cast<int32>(newWidth) + 1, static_cast<int32>(newHeight) + 1}
	});
}


} // HaikuReact
