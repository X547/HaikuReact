#include "ColumnListViewNode.h"

#include <stdexcept>

#include "../PropSet.h"
#include "../ReactColumnListView.h"
#include "../ReactColumns.h"
#include "RowNode.h"


namespace HaikuReact {


ColumnListViewNode::ColumnListViewNode(uint32 id):
	ViewNode(id, new ReactColumnListView(id), BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED)),
	fListView(static_cast<ReactColumnListView*>(View()))
{
}


void ColumnListViewNode::RemoveRows()
{
	for (auto it = Children().rbegin(); it != Children().rend(); ++it) {
		static_cast<RowNode&>(**it).RemoveFromList();
	}
}


void ColumnListViewNode::AddRows()
{
	for (const std::unique_ptr<Node> &child : Children()) {
		static_cast<RowNode&>(*child).AddToList(*fListView, -1, nullptr);
	}
	fListView->RestoreSelection();
}


void ColumnListViewNode::AttachChild(Node &child, int32 index)
{
	AsRow(*this, child).AddToList(*fListView, index, nullptr);
}


void ColumnListViewNode::DetachChild(Node &child)
{
	static_cast<RowNode&>(child).RemoveFromList();
}


void ColumnListViewNode::ChildListChanged()
{
	fListView->RestoreSelection();
}


void ColumnListViewNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto columns = props.Get<std::vector<ColumnSpec>>("columns", {}); columns && *columns != fListView->Columns()) {
		// Rows are built for the columns, so they are rebuilt with them.
		RemoveRows();
		fListView->SetColumns(*columns);
		fListView->SetSort(fSort);
		AddRows();
	}
	if (auto multiple = props.Get<bool>("multiple", false)) {
		fListView->SetSelectionMode(*multiple ? B_MULTIPLE_SELECTION_LIST : B_SINGLE_SELECTION_LIST);
		fListView->RestoreSelection();
	}
	if (auto sort = props.Get<std::vector<SortKey>>("sort", {})) {
		const bool cleared = sort->empty() && !fListView->Sort().empty();
		fSort = *sort;
		fListView->SetSort(fSort);
		if (cleared) {
			// Without a sort the rows show in child order again.
			RemoveRows();
			AddRows();
		}
	}
	if (auto selection = props.Get<IdList>("selection", {})) {
		fListView->SetSelection(selection->ids);
	}
}


void ColumnListViewNode::Invoke()
{
	fListView->Invoke();
}


void ColumnListViewNode::SimulateInput(const EventValue &value)
{
	if (const auto *column = std::get_if<std::string>(&value)) {
		fListView->PostTitleClick(*column);
	} else if (const auto *ids = std::get_if<std::vector<std::string>>(&value)) {
		fListView->PostSelect(*ids);
	} else if (const auto *indices = std::get_if<std::vector<int32>>(&value); indices != nullptr && indices->empty()) {
		fListView->PostSelect({});
	} else {
		throw std::invalid_argument("<ColumnListView> input must be an array of row ids or a column id");
	}
}


void ColumnListViewNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	Napi::Env env = out.Env();

	const std::vector<ColumnSpec> &columns = fListView->Columns();
	Napi::Array columnsOut = Napi::Array::New(env, columns.size());
	for (uint32 i = 0; i < columns.size(); i++) {
		Napi::Object column = Napi::Object::New(env);
		column.Set("id", columns[i].id);
		column.Set("title", columns[i].title);
		columnsOut.Set(i, column);
	}
	out.Set("columns", columnsOut);

	const std::vector<SortKey> sort = fListView->Sort();
	Napi::Array sortOut = Napi::Array::New(env, sort.size());
	for (uint32 i = 0; i < sort.size(); i++) {
		Napi::Object key = Napi::Object::New(env);
		key.Set("column", sort[i].column);
		key.Set("direction", sort[i].ascending ? "ascending" : "descending");
		sortOut.Set(i, key);
	}
	out.Set("sort", sortOut);

	const std::vector<std::string> selection = fListView->Selection();
	Napi::Array selectionOut = Napi::Array::New(env, selection.size());
	for (uint32 i = 0; i < selection.size(); i++) {
		selectionOut.Set(i, selection[i]);
	}
	out.Set("selection", selectionOut);

	// What the list shows, top to bottom.
	Napi::Array rows = Napi::Array::New(env);
	uint32 count = 0;
	fListView->ForEachRow([&](ReactRow &row, int32 level) {
		Napi::Object rowOut = Napi::Object::New(env);
		rowOut.Set("id", row.IsPlaceholder() ? env.Null() : Napi::Value(Napi::String::New(env, row.ItemId())));
		rowOut.Set("level", level);
		rowOut.Set("placeholder", row.IsPlaceholder());
		Napi::Object cells = Napi::Object::New(env);
		for (size_t i = 0; i < columns.size(); i++) {
			cells.Set(columns[i].id, FieldToJs(env, columns[i].type, row.GetField(static_cast<int32>(i))));
		}
		rowOut.Set("cells", cells);
		rows.Set(count++, rowOut);
	});
	out.Set("rows", rows);
}


} // HaikuReact
