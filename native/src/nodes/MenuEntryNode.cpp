#include "MenuEntryNode.h"

#include <string>

#include <MenuItem.h>

#include "../PropSet.h"
#include "../ReactMenu.h"
#include "../ReactMenuItem.h"


namespace HaikuReact {


MenuEntryNode::MenuEntryNode(uint32 id, BMenuItem *item):
	Node(id),
	fItem(item)
{
}


MenuEntryNode::~MenuEntryNode()
{
	if (Parent() != nullptr) {
		return;
	}
	// Removed from a menu that is open: the menu still shows the item until
	// its own thread applies the removal.
	if (auto *menu = dynamic_cast<ReactMenu*>(fItem->Menu())) {
		menu->Adopt(fItem);
		return;
	}
	delete fItem;
}


void MenuEntryNode::ApplyOwnProps(const PropSet &props)
{
	if (auto label = props.Get<std::string>("label", "")) {
		fItem->SetLabel(label->c_str());
	}
	if (auto enabled = props.Get<bool>("enabled", true)) {
		fItem->SetEnabled(*enabled);
	}
	if (auto *item = dynamic_cast<ReactMenuItem*>(fItem)) {
		if (auto icon = props.Get<IconSource>("icon", {})) {
			item->SetIcon(*icon);
		}
	}
}


void MenuEntryNode::Dump(Napi::Object &out) const
{
	Node::Dump(out);
	out.Set("label", fItem->Label() != nullptr ? fItem->Label() : "");
	out.Set("enabled", fItem->IsEnabled());
	if (auto *item = dynamic_cast<ReactMenuItem*>(fItem)) {
		DumpMenuItem(out, *item);
	}
}


void DumpMenuItem(Napi::Object &out, ReactMenuItem &item)
{
	Napi::Env env = out.Env();
	out.Set("hasIcon", item.HasIcon());
	out.Set("iconSize", item.IconSize());
	if (item.Menu() == nullptr) {
		return;
	}
	float width = 0;
	float height = 0;
	item.ContentSize(&width, &height);
	Napi::Object contentOut = Napi::Object::New(env);
	contentOut.Set("width", width);
	contentOut.Set("height", height);
	out.Set("contentSize", contentOut);

	const BRect frame = item.Frame();
	Napi::Object frameOut = Napi::Object::New(env);
	frameOut.Set("x", frame.left);
	frameOut.Set("y", frame.top);
	frameOut.Set("width", frame.Width() + 1);
	frameOut.Set("height", frame.Height() + 1);
	out.Set("itemFrame", frameOut);
}


} // HaikuReact
