#include "RowNode.h"

#include <stdexcept>

#include "../PropSet.h"
#include "../ReactColumnListView.h"
#include "../ReactColumns.h"


namespace HaikuReact {


static constexpr const char *kDefaultLoadingLabel = "Loading…";


RowNode &AsRow(Node &parent, Node &child)
{
	auto *row = dynamic_cast<RowNode*>(&child);
	if (row == nullptr) {
		throw std::invalid_argument(std::string("<") + parent.TypeName() + "> can only contain <Row>, not <"
			+ child.TypeName() + ">");
	}
	return *row;
}


// The placeholder's label goes into the first text column.
static ReactRow *MakeRow(const ReactColumnListView &list, uint32 nodeId, const std::string &itemId,
	const CellMap &cells, const std::string *placeholderLabel)
{
	auto *row = new ReactRow(nodeId, itemId, placeholderLabel != nullptr);
	const std::vector<ColumnSpec> &columns = list.Columns();
	bool labelPlaced = false;
	for (size_t i = 0; i < columns.size(); i++) {
		const Cell *cell = nullptr;
		Cell label;
		if (placeholderLabel != nullptr) {
			if (!labelPlaced && columns[i].type == ColumnType::Text) {
				label.text = *placeholderLabel;
				cell = &label;
				labelPlaced = true;
			}
		} else if (const auto it = cells.find(columns[i].id); it != cells.end()) {
			cell = &it->second;
		}
		row->SetField(CreateField(columns[i].type, cell), static_cast<int32>(i));
	}
	return row;
}


RowNode::RowNode(uint32 id):
	Node(id),
	fLoadingLabel(kDefaultLoadingLabel)
{
}


bool RowNode::NeedsPlaceholder() const
{
	return fLoading || (fHasChildren && Children().empty());
}


bool RowNode::IsExpanded() const
{
	return fList != nullptr ? fRow->IsExpanded() : fExpanded;
}


void RowNode::AddPlaceholder()
{
	fPlaceholder = MakeRow(*fList, 0, "", {}, &fLoadingLabel);
	fList->AddRow(fPlaceholder, 0, fRow);
}


void RowNode::UpdatePlaceholder()
{
	if (fList == nullptr) {
		return;
	}
	const bool needed = NeedsPlaceholder();
	if (needed && fPlaceholder != nullptr) {
		// The label may have changed.
		fList->RemoveRow(fPlaceholder);
		delete fPlaceholder;
		AddPlaceholder();
	} else if (needed) {
		AddPlaceholder();
	} else if (fPlaceholder != nullptr) {
		fList->RemoveRow(fPlaceholder);
		delete fPlaceholder;
		fPlaceholder = nullptr;
	}
}


void RowNode::SetExpanded(bool expanded)
{
	if (fList == nullptr) {
		fExpanded = expanded;
	} else if (fRow->IsExpanded() != expanded) {
		fList->ExpandOrCollapse(fRow, expanded);
	}
}


void RowNode::UpdateFields()
{
	if (fList == nullptr) {
		return;
	}
	const std::vector<ColumnSpec> &columns = fList->Columns();
	for (size_t i = 0; i < columns.size(); i++) {
		const auto it = fCells.find(columns[i].id);
		fRow->SetField(CreateField(columns[i].type, it != fCells.end() ? &it->second : nullptr), static_cast<int32>(i));
	}
	// Moves the row if the list is sorted.
	fList->UpdateRow(fRow);
}


void RowNode::AddToList(ReactColumnListView &list, int32 index, ReactRow *parent)
{
	fList = &list;
	fRow = MakeRow(list, Id(), fItemId, fCells, nullptr);
	list.AddRow(fRow, index, parent);
	if (fExpanded) {
		list.ExpandOrCollapse(fRow, true);
	}
	if (NeedsPlaceholder()) {
		AddPlaceholder();
	}
	for (const std::unique_ptr<Node> &child : Children()) {
		static_cast<RowNode&>(*child).AddToList(list, -1, fRow);
	}
}


void RowNode::RemoveFromList()
{
	if (fList == nullptr) {
		return;
	}
	// Children first: BColumnListView::AddRow() drops the child list of a
	// row added again, and the rows of a removed parent would be lost.
	for (auto it = Children().rbegin(); it != Children().rend(); ++it) {
		static_cast<RowNode&>(**it).RemoveFromList();
	}
	if (fPlaceholder != nullptr) {
		fList->RemoveRow(fPlaceholder);
		delete fPlaceholder;
		fPlaceholder = nullptr;
	}
	fExpanded = fRow->IsExpanded();
	fList->RemoveRow(fRow);
	delete fRow;
	fRow = nullptr;
	fList = nullptr;
}


void RowNode::AttachChild(Node &child, int32 index)
{
	RowNode &row = AsRow(*this, child);
	if (fList != nullptr) {
		row.AddToList(*fList, index + (fPlaceholder != nullptr ? 1 : 0), fRow);
	}
}


void RowNode::DetachChild(Node &child)
{
	static_cast<RowNode&>(child).RemoveFromList();
}


void RowNode::ChildListChanged()
{
	if (fList != nullptr && (fPlaceholder != nullptr) != NeedsPlaceholder()) {
		UpdatePlaceholder();
	}
	if (fList != nullptr) {
		fList->RestoreSelection();
	}
}


void RowNode::ApplyOwnProps(const PropSet &props)
{
	if (auto id = props.Get<std::string>("id", "")) {
		fItemId = *id;
		if (fRow != nullptr) {
			fRow->SetItemId(fItemId);
			fList->RestoreSelection();
		}
	}
	if (auto cells = props.Get<CellMap>("cells", {}); cells && *cells != fCells) {
		fCells = *cells;
		UpdateFields();
	}

	bool placeholderChanged = false;
	if (auto hasChildren = props.Get<bool>("hasChildren", false)) {
		fHasChildren = *hasChildren;
		placeholderChanged = true;
	}
	if (auto loading = props.Get<bool>("loading", false)) {
		fLoading = *loading;
		placeholderChanged = true;
	}
	if (auto loadingLabel = props.Get<std::string>("loadingLabel", kDefaultLoadingLabel)) {
		placeholderChanged |= *loadingLabel != fLoadingLabel;
		fLoadingLabel = *loadingLabel;
	}
	if (placeholderChanged && fList != nullptr) {
		UpdatePlaceholder();
	}

	if (auto expanded = props.Get<bool>("expanded", false)) {
		SetExpanded(*expanded);
	}
}


void RowNode::SimulateInput(const EventValue &value)
{
	const bool *expand = std::get_if<bool>(&value);
	if (expand == nullptr || fList == nullptr) {
		throw std::invalid_argument("<Row> input must be a boolean (expand), and the row must be in a list");
	}
	fList->PostExpand(fRow, *expand);
}


void RowNode::Dump(Napi::Object &out) const
{
	Node::Dump(out);
	Napi::Env env = out.Env();
	out.Set("itemId", fItemId);
	out.Set("expanded", IsExpanded());
	out.Set("placeholder", fPlaceholder != nullptr ? Napi::Value(Napi::String::New(env, fLoadingLabel)) : env.Null());
	out.Set("visible", fList != nullptr && fList->IsRowShown(fRow));
}


} // HaikuReact
