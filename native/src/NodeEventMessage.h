#pragma once

#include <SupportDefs.h>

#include "EventBridge.h"

class BMessage;
class BWindow;


namespace HaikuReact {


// Message sent by invokers (controls, menu items) to their window. It names
// the node by id only, so it stays harmless if the node is gone on arrival.
constexpr uint32 kMsgNodeEvent = 'HRne';

// Field with an explicit int32 event value, e.g. the index of a menu item.
constexpr const char *kFieldEventValue = "hr:value";


// Where the window thread finds the value reported with the event.
enum class EventValueKind: int32 {
	None,
	ControlBool,  // "be:value" of the invoking BControl, as a boolean
	ControlInt,   // "be:value" of the invoking BControl
	ControlText,  // text of the invoking BTextControl
	Index,        // kFieldEventValue, or "index" added by list views and menu items
};


BMessage *MakeNodeMessage(uint32 nodeId, const char *eventName, EventValueKind kind = EventValueKind::None);

// Called on the window thread, with the window locked.
Event EventFromNodeMessage(const BMessage &message, const BWindow &window);


} // HaikuReact
