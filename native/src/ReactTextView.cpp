#include "ReactTextView.h"

#include <Messenger.h>
#include <OS.h>
#include <Window.h>

#include "EventBridge.h"
#include "StatusCheck.h"


namespace HaikuReact {


constexpr uint32 kMsgSetText = 'HRst';
constexpr const char *kFieldText = "text";


ReactTextView::ReactTextView(uint32 nodeId):
	BTextView("TextView"),
	fNodeId(nodeId)
{
}


void ReactTextView::NotifyChanged()
{
	// Changes applied from JS run on the JS thread; only those made on the
	// window thread come from the user.
	const BWindow *window = Window();
	if (window != nullptr && find_thread(nullptr) == window->Thread()) {
		EventBridge::Post(Event{fNodeId, "change", std::string(Text())});
	}
}


void ReactTextView::InsertText(const char *text, int32 length, int32 offset, const text_run_array *runs)
{
	BTextView::InsertText(text, length, offset, runs);
	NotifyChanged();
}


void ReactTextView::DeleteText(int32 fromOffset, int32 toOffset)
{
	BTextView::DeleteText(fromOffset, toOffset);
	NotifyChanged();
}


void ReactTextView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgSetText:
			SetText(message->GetString(kFieldText, ""));
			break;
		default:
			BTextView::MessageReceived(message);
			break;
	}
}


void ReactTextView::PostSetText(const std::string &text)
{
	BMessage message(kMsgSetText);
	message.AddString(kFieldText, text.c_str());
	CheckThrow(BMessenger(this).SendMessage(&message));
}


} // HaikuReact
