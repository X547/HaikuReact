#include "MenuFieldNode.h"

#include <stdexcept>

#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

#include "../NodeEventMessage.h"
#include "../PropSet.h"
#include "../StatusCheck.h"


namespace HaikuReact {


MenuFieldNode::MenuFieldNode(uint32 id):
	ViewNode(id, new BMenuField("MenuField", "", new BPopUpMenu("", true, true))),
	fMenuField(static_cast<BMenuField*>(View())),
	fMenu(static_cast<BPopUpMenu*>(fMenuField->Menu()))
{
}


void MenuFieldNode::SetItems(const std::vector<std::string> &items)
{
	if (items == fItems) {
		return;
	}
	fMenu->RemoveItems(0, fMenu->CountItems(), true);
	for (size_t i = 0; i < items.size(); i++) {
		BMessage *message = MakeNodeMessage(Id(), "change", EventValueKind::Index);
		message->AddInt32(kFieldEventValue, static_cast<int32>(i));
		fMenu->AddItem(new BMenuItem(items[i].c_str(), message));
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
	if (auto items = props.Get<std::vector<std::string>>("items", {})) {
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
}


} // HaikuReact
