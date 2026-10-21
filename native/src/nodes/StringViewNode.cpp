#include "StringViewNode.h"

#include <string>

#include <StringView.h>

#include "../PropSet.h"


namespace HaikuReact {


StringViewNode::StringViewNode(uint32 id):
	ViewNode(id, new BStringView("StringView", "")),
	fStringView(static_cast<BStringView*>(View()))
{
}


void StringViewNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto text = props.Get<std::string>("text", "")) {
		fStringView->SetText(text->c_str());
	}
}


void StringViewNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("text", fStringView->Text() != nullptr ? fStringView->Text() : "");
}


} // HaikuReact
