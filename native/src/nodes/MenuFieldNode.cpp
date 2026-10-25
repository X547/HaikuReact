#include "MenuFieldNode.h"

#include <stdexcept>

#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

#include "../NodeEventMessage.h"
#include "../PropSet.h"
#include "../ReactMenuItem.h"
#include "../StatusCheck.h"
#include "MenuEntryNode.h"


namespace HaikuReact {


MenuFieldNode::MenuFieldNode(uint32 id):
	ViewNode(id, new BMenuField("MenuField", "", new BPopUpMenu("", true, true))),
	fMenuField(static_cast<BMenuField*>(View())),
	fMenu(static_cast<BPopUpMenu*>(fMenuField->Menu()))
{
}


void MenuFieldNode::SetItems(const std::vector<LabeledItem> &items)
{
	if (items == fItems) {
		return;
	}
	fMenu->RemoveItems(0, fMenu->CountItems(), true);
	for (size_t i = 0; i < items.size(); i++) {
		BMessage *message = MakeNodeMessage(Id(), "change", EventValueKind::Index);
		message->AddInt32(kFieldEventValue, static_cast<int32>(i));
		auto *item = new ReactMenuItem(items[i].label.c_str(), message);
		item->SetIcon(items[i].icon);
		fMenu->AddItem(item);
	}
	fItems = items;
	MarkValue();
}


void MenuFieldNode::MarkValue()
{
	BMenuItem *item = fMenu->ItemAt(fValue);
	if (item != nullptr) {
		item->SetMarked(true);
		return;
	}
	if (BMenuItem *marked = fMenu->FindMarked(); marked != nullptr) {
		marked->SetMarked(false);
	}
}


void MenuFieldNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto label = props.Get<std::string>("label", "")) {
		fMenuField->SetLabel(label->c_str());
	}
	if (auto enabled = props.Get<bool>("enabled", true)) {
		fMenuField->SetEnabled(*enabled);
	}
	if (auto items = props.Get<std::vector<LabeledItem>>("items", {})) {
		SetItems(*items);
	}
	if (auto value = props.Get<int32>("value", -1)) {
		fValue = *value;
		MarkValue();
	}
}


void MenuFieldNode::SimulateInput(const EventValue &value)
{
	const double *index = std::get_if<double>(&value);
	BMenuItem *item = index != nullptr ? fMenu->ItemAt(static_cast<int32>(*index)) : nullptr;
	if (item == nullptr) {
		throw std::invalid_argument("<MenuField> input must be an item index");
	}
	// What choosing the item does: mark it (radio mode) and send its message
	// to its target, the window.
	item->SetMarked(true);
	CheckThrow(item->Messenger().SendMessage(item->Message()));
}


void MenuFieldNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("label", fMenuField->Label() != nullptr ? fMenuField->Label() : "");
	out.Set("value", fMenu->IndexOf(fMenu->FindMarked()));
	out.Set("enabled", fMenuField->IsEnabled());

	Napi::Env env = out.Env();
	Napi::Array items = Napi::Array::New(env, fMenu->CountItems());
	for (int32 i = 0; i < fMenu->CountItems(); i++) {
		Napi::Object itemOut = Napi::Object::New(env);
		itemOut.Set("label", fMenu->ItemAt(i)->Label());
		DumpMenuItem(itemOut, *static_cast<ReactMenuItem*>(fMenu->ItemAt(i)));
		items.Set(i, itemOut);
	}
	out.Set("items", items);
}


} // HaikuReact
