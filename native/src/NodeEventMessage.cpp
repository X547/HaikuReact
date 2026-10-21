#include "NodeEventMessage.h"

#include <Message.h>
#include <TextControl.h>
#include <Window.h>


namespace HaikuReact {


constexpr const char *kFieldNode = "hr:node";
constexpr const char *kFieldEvent = "hr:event";
constexpr const char *kFieldKind = "hr:kind";
constexpr const char *kFieldControlValue = "be:value";
constexpr const char *kFieldIndex = "index";
constexpr const char *kFieldSource = "source";


// Compares pointers only: the source may have been deleted after posting.
static bool ContainsView(const BView &parent, const void *view)
{
	for (int32 i = 0; BView *child = parent.ChildAt(i); i++) {
		if (child == view || ContainsView(*child, view)) {
			return true;
		}
	}
	return false;
}


static bool ContainsView(const BWindow &window, const void *view)
{
	for (int32 i = 0; BView *child = window.ChildAt(i); i++) {
		if (child == view || ContainsView(*child, view)) {
			return true;
		}
	}
	return false;
}


static EventValue ControlText(const BMessage &message, const BWindow &window)
{
	void *source = nullptr;
	if (message.FindPointer(kFieldSource, &source) != B_OK || !ContainsView(window, source)) {
		return {};
	}
	const auto *view = static_cast<const BView*>(source);
	const auto *textControl = dynamic_cast<const BTextControl*>(view);
	if (textControl == nullptr) {
		return {};
	}
	return std::string(textControl->Text());
}


BMessage *MakeNodeMessage(uint32 nodeId, const char *eventName, EventValueKind kind)
{
	auto *message = new BMessage(kMsgNodeEvent);
	message->AddUInt32(kFieldNode, nodeId);
	message->AddString(kFieldEvent, eventName);
	message->AddInt32(kFieldKind, static_cast<int32>(kind));
	return message;
}


Event EventFromNodeMessage(const BMessage &message, const BWindow &window)
{
	Event event;
	event.nodeId = message.GetUInt32(kFieldNode, 0);
	event.name = message.GetString(kFieldEvent, "");

	const auto kind = static_cast<EventValueKind>(message.GetInt32(kFieldKind, 0));
	switch (kind) {
		case EventValueKind::None:
			break;
		case EventValueKind::ControlBool:
			event.value = message.GetInt32(kFieldControlValue, 0) != B_CONTROL_OFF;
			break;
		case EventValueKind::ControlInt:
			event.value = message.GetInt32(kFieldControlValue, 0);
			break;
		case EventValueKind::ControlText:
			event.value = ControlText(message, window);
			break;
		case EventValueKind::Index:
			event.value = message.GetInt32(kFieldEventValue, message.GetInt32(kFieldIndex, -1));
			break;
		default:
			break;
	}
	return event;
}


} // HaikuReact
