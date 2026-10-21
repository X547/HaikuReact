#include "TextControlNode.h"

#include <limits>
#include <stdexcept>
#include <string>

#include <TextControl.h>

#include "../NodeEventMessage.h"
#include "../PropSet.h"
#include "../TextUpdate.h"


namespace HaikuReact {


TextControlNode::TextControlNode(uint32 id):
	ControlNode(id, new BTextControl("TextControl", "", "", MakeNodeMessage(id, "submit", EventValueKind::ControlText))),
	fTextControl(static_cast<BTextControl*>(&Control()))
{
	fTextControl->SetModificationMessage(MakeNodeMessage(id, "change", EventValueKind::ControlText));
}


void TextControlNode::ApplyOwnProps(const PropSet &props)
{
	ControlNode::ApplyOwnProps(props);

	if (auto value = props.Get<std::string>("value", "")) {
		// BTextControl reports every text change, including this one; detach
		// the modification message so that only user edits reach onChange.
		fTextControl->SetModificationMessage(nullptr);
		SetTextKeepingSelection(*fTextControl->TextView(), *value);
		fTextControl->SetModificationMessage(MakeNodeMessage(Id(), "change", EventValueKind::ControlText));
	}
	if (auto password = props.Get<bool>("password", false)) {
		fTextControl->TextView()->HideTyping(*password);
	}
	if (auto maxLength = props.Get<int32>("maxLength", std::numeric_limits<int32>::max())) {
		fTextControl->TextView()->SetMaxBytes(*maxLength);
	}
}


void TextControlNode::SimulateInput(const EventValue &value)
{
	const std::string *text = std::get_if<std::string>(&value);
	if (text == nullptr) {
		throw std::invalid_argument("<TextControl> input must be a string");
	}
	// Like typing: the text view reports the change through the modification
	// message.
	fTextControl->SetText(text->c_str());
}


void TextControlNode::Dump(Napi::Object &out) const
{
	ControlNode::Dump(out);
	out.Set("text", fTextControl->Text());
}


} // HaikuReact
