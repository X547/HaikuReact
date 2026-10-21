#include "TextViewNode.h"

#include <stdexcept>
#include <string>

#include "../PropSet.h"
#include "../ReactTextView.h"
#include "../TextUpdate.h"


namespace HaikuReact {


TextViewNode::TextViewNode(uint32 id):
	ViewNode(id, new ReactTextView(id)),
	fTextView(static_cast<ReactTextView*>(View()))
{
}


void TextViewNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto text = props.Get<std::string>("text", "")) {
		SetTextKeepingSelection(*fTextView, *text);
	}
	if (auto editable = props.Get<bool>("editable", true)) {
		fTextView->MakeEditable(*editable);
	}
	if (auto selectable = props.Get<bool>("selectable", true)) {
		fTextView->MakeSelectable(*selectable);
	}
	if (auto wordWrap = props.Get<bool>("wordWrap", true)) {
		fTextView->SetWordWrap(*wordWrap);
	}
}


void TextViewNode::SimulateInput(const EventValue &value)
{
	const std::string *text = std::get_if<std::string>(&value);
	if (text == nullptr) {
		throw std::invalid_argument("<TextView> input must be a string");
	}
	fTextView->PostSetText(*text);
}


void TextViewNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("text", fTextView->Text());
}


} // HaikuReact
