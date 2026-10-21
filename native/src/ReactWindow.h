#pragma once

#include <Window.h>


namespace HaikuReact {


// Window backing a <Window> node. Forwards node events, window events and
// close requests to JS; closing is decided by React, so QuitRequested()
// never agrees.
class ReactWindow: public BWindow {
private:
	const uint32 fNodeId;

public:
	explicit ReactWindow(uint32 nodeId);

	void MessageReceived(BMessage *message) final;
	bool QuitRequested() final;
	void WindowActivated(bool active) final;
	void FrameMoved(BPoint newPosition) final;
	void FrameResized(float newWidth, float newHeight) final;
};


} // HaikuReact
