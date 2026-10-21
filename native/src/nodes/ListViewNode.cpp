#include "ListViewNode.h"

#include <stdexcept>

#include <StringItem.h>

#include "../NodeEventMessage.h"
#include "../PropSet.h"
#include "../ReactListView.h"


namespace HaikuReact {


ListViewNode::ListViewNode(uint32 id):
	ViewNode(id, new ReactListView(id)),
	fListView(static_cast<ReactListView*>(View()))
{
	fListView->SetInvocationMessage(MakeNodeMessage(id, "invoke", EventValueKind::Index));
}


ListViewNode::~ListViewNode()
{
	// The view may already be deleted by its parent view; ~BListView does
	// not touch the items, so they are only deleted here.
	if (Parent() == nullptr) {
		fListView->MakeEmpty();
	}
}


void ListViewNode::SetItems(const std::vector<std::string> &labels)
{
	if (labels == fLabels) {
		return;
	}
	fListView->MakeEmpty();
	fItems.clear();
	for (const std::string &label : labels) {
		fItems.push_back(std::make_unique<BStringItem>(label.c_str()));
		fListView->AddItem(fItems.back().get());
	}
	fLabels = labels;
	fListView->SetSelection(fSelection);
}


void ListViewNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto multiple = props.Get<bool>("multiple", false)) {
		fListView->SetListType(*multiple ? B_MULTIPLE_SELECTION_LIST : B_SINGLE_SELECTION_LIST);
	}
	if (auto items = props.Get<std::vector<std::string>>("items", {})) {
		SetItems(*items);
	}
	if (auto selection = props.Get<std::vector<int32>>("selection", {})) {
		fSelection = *selection;
		fListView->SetSelection(fSelection);
	}
}


void ListViewNode::SimulateInput(const EventValue &value)
{
	const auto *selection = std::get_if<std::vector<int32>>(&value);
	if (selection == nullptr) {
		throw std::invalid_argument("<ListView> input must be an array of indices");
	}
	fListView->PostSelect(*selection);
}


void ListViewNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);

	Napi::Env env = out.Env();
	Napi::Array items = Napi::Array::New(env, fLabels.size());
	for (uint32 i = 0; i < fLabels.size(); i++) {
		items.Set(i, fLabels[i]);
	}
	out.Set("items", items);

	const std::vector<int32> selection = fListView->Selection();
	Napi::Array selectionOut = Napi::Array::New(env, selection.size());
	for (uint32 i = 0; i < selection.size(); i++) {
		selectionOut.Set(i, selection[i]);
	}
	out.Set("selection", selectionOut);
}


} // HaikuReact
