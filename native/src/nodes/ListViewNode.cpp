#include "ListViewNode.h"

#include <stdexcept>
#include <string>

#include "../BitmapCache.h"
#include "../ReactListItem.h"
#include "../ReactListView.h"
#include "ListItemNode.h"


namespace HaikuReact {


ListViewNode::ListViewNode(uint32 id):
	ViewNode(id, new ReactListView(id)),
	fListView(static_cast<ReactListView*>(View()))
{
}


ListViewNode::~ListViewNode()
{
	// The view may already be deleted by its parent view; ~BListView does
	// not touch the items, so they are only deleted by their owners.
	if (Parent() == nullptr) {
		fListView->MakeEmpty();
	}
}


void ListViewNode::SetItems(const std::vector<LabeledItem> &labels)
{
	if (labels == fLabels) {
		return;
	}
	if (!Children().empty()) {
		throw std::invalid_argument("<ListView> takes either the items prop or <ListItem> children");
	}
	fListView->MakeEmpty();
	fItems.clear();
	for (size_t i = 0; i < labels.size(); i++) {
		auto item = std::make_unique<ReactListItem>(0);
		item->SetItemId(std::to_string(i));
		item->SetLabel(labels[i].label);
		item->SetIcon(BitmapCache::Get(labels[i].icon, item->IconSize()));
		fListView->AddItem(item.get());
		fItems.push_back(std::move(item));
	}
	fLabels = labels;
	fListView->SetIndexIds(true);
	fListView->RestoreSelection();
}


void ListViewNode::AttachChild(Node &child, int32 index)
{
	ListItemNode &item = ListChildren::AsItem(*this, child);
	if (!fItems.empty()) {
		throw std::invalid_argument("<ListView> takes either the items prop or <ListItem> children");
	}
	fListView->SetIndexIds(false);
	const int32 fullIndex = ListChildren::InsertionIndex(*fListView, Children(), index, fListView->FullListCountItems());
	item.AddToList(*fListView, fullIndex, 0);
}


void ListViewNode::DetachChild(Node &child)
{
	static_cast<ListItemNode&>(child).RemoveFromList();
}


void ListViewNode::ChildListChanged()
{
	fListView->RestoreSelection();
}


void ListViewNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto multiple = props.Get<bool>("multiple", false)) {
		fListView->SetListType(*multiple ? B_MULTIPLE_SELECTION_LIST : B_SINGLE_SELECTION_LIST);
	}
	if (auto items = props.Get<std::vector<LabeledItem>>("items", {})) {
		SetItems(*items);
	}
	if (auto selection = props.Get<IdList>("selection", {})) {
		fListView->SetSelection(selection->ids);
	}
}


void ListViewNode::Invoke()
{
	fListView->Invoke();
}


void ListViewNode::SimulateInput(const EventValue &value)
{
	if (const auto *ids = std::get_if<std::vector<std::string>>(&value)) {
		fListView->PostSelect(*ids);
		return;
	}
	const auto *indices = std::get_if<std::vector<int32>>(&value);
	if (indices == nullptr) {
		throw std::invalid_argument("<ListView> input must be an array of ids or indices");
	}
	std::vector<std::string> ids;
	for (const int32 index : *indices) {
		ids.push_back(std::to_string(index));
	}
	fListView->PostSelect(ids);
}


void ListViewNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);

	Napi::Env env = out.Env();
	Napi::Array items = Napi::Array::New(env, fLabels.size());
	for (uint32 i = 0; i < fLabels.size(); i++) {
		items.Set(i, fLabels[i].label);
	}
	out.Set("items", items);

	const std::vector<std::string> selection = fListView->Selection();
	Napi::Array selectionOut = Napi::Array::New(env, selection.size());
	for (uint32 i = 0; i < selection.size(); i++) {
		if (fItems.empty()) {
			selectionOut.Set(i, selection[i]);
		} else {
			selectionOut.Set(i, std::stoi(selection[i]));
		}
	}
	out.Set("selection", selectionOut);

	// What the list shows, top to bottom.
	Napi::Array rows = Napi::Array::New(env, fListView->CountItems());
	for (int32 i = 0; i < fListView->CountItems(); i++) {
		const auto *item = static_cast<const ReactListItem*>(fListView->ItemAt(i));
		Napi::Object row = Napi::Object::New(env);
		row.Set("id", item->IsPlaceholder() ? env.Null() : Napi::Value(Napi::String::New(env, item->ItemId())));
		row.Set("label", item->Label());
		row.Set("level", item->OutlineLevel());
		row.Set("placeholder", item->IsPlaceholder());
		rows.Set(i, row);
	}
	out.Set("rows", rows);
}


} // HaikuReact
