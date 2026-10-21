#include "MenuEntryNode.h"

#include <string>

#include <MenuItem.h>

#include "../PropSet.h"


namespace HaikuReact {


MenuEntryNode::MenuEntryNode(uint32 id, BMenuItem *item):
	Node(id),
	fItem(item)
{
}


MenuEntryNode::~MenuEntryNode()
{
	if (Parent() == nullptr) {
		delete fItem;
	}
}


void MenuEntryNode::ApplyOwnProps(const PropSet &props)
{
	if (auto label = props.Get<std::string>("label", "")) {
		fItem->SetLabel(label->c_str());
	}
	if (auto enabled = props.Get<bool>("enabled", true)) {
		fItem->SetEnabled(*enabled);
	}
}


void MenuEntryNode::Dump(Napi::Object &out) const
{
	Node::Dump(out);
	out.Set("label", fItem->Label() != nullptr ? fItem->Label() : "");
	out.Set("enabled", fItem->IsEnabled());
}


} // HaikuReact
