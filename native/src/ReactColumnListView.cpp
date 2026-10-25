#include "ReactColumnListView.h"

#include <set>

#include <MessageFilter.h>
#include <Messenger.h>
#include <OS.h>
#include <Window.h>

#include "EventBridge.h"
#include "ReactColumns.h"
#include "StatusCheck.h"


namespace HaikuReact {


constexpr uint32 kMsgCheckSort = 'HRcs';
constexpr uint32 kMsgSelect = 'HRsl';
constexpr uint32 kMsgExpand = 'HRex';
constexpr uint32 kMsgTitleClick = 'HRtc';
constexpr const char *kFieldId = "id";
constexpr const char *kFieldRow = "row";
constexpr const char *kFieldExpand = "expand";
constexpr const char *kFieldColumn = "column";


// BColumnListView has no hook for sorting by the user, which happens when a
// column title is released; the list compares the sort after each release.
class TitleReleaseFilter: public BMessageFilter {
private:
	BMessenger fList;

public:
	explicit TitleReleaseFilter(BHandler *list):
		BMessageFilter(B_MOUSE_UP),
		fList(list)
	{
	}

	filter_result Filter(BMessage *message, BHandler **target) final
	{
		(void)message;
		(void)target;
		fList.SendMessage(kMsgCheckSort);
		return B_DISPATCH_MESSAGE;
	}
};


static bool OnWindowThread(const BView &view)
{
	const BWindow *window = view.Window();
	return window != nullptr && find_thread(nullptr) == window->Thread();
}


ReactRow::ReactRow(uint32 nodeId, const std::string &itemId, bool placeholder):
	fNodeId(nodeId),
	fItemId(itemId),
	fPlaceholder(placeholder)
{
}


ReactColumnListView::ReactColumnListView(uint32 nodeId):
	BColumnListView("ColumnListView", B_NAVIGABLE, B_FANCY_BORDER, true),
	fNodeId(nodeId)
{
}


void ReactColumnListView::SetColumns(const std::vector<ColumnSpec> &columns)
{
	while (CountColumns() > 0) {
		BColumn *column = ColumnAt(0);
		RemoveColumn(column);
		delete column;
	}
	fColumns = columns;
	for (size_t i = 0; i < columns.size(); i++) {
		AddColumn(CreateColumn(columns[i]), static_cast<int32>(i));
	}
}


BColumn *ReactColumnListView::ColumnById(const std::string &id) const
{
	for (int32 i = 0; i < CountColumns(); i++) {
		BColumn *column = ColumnAt(i);
		const int32 index = column->LogicalFieldNum();
		if (index >= 0 && index < static_cast<int32>(fColumns.size()) && fColumns[index].id == id) {
			return column;
		}
	}
	return nullptr;
}


void ReactColumnListView::ForEachRow(const std::function<void(ReactRow &row, int32 level)> &f, bool all)
{
	const auto visit = [&](auto &self, BRow *parent, int32 level) -> void {
		for (int32 i = 0; i < CountRows(parent); i++) {
			BRow *row = RowAt(i, parent);
			f(*static_cast<ReactRow*>(row), level);
			if (all || row->IsExpanded()) {
				self(self, row, level + 1);
			}
		}
	};
	visit(visit, nullptr, 0);
}


bool ReactColumnListView::IsRowShown(BRow *row) const
{
	BRow *parent = nullptr;
	bool visible = true;
	FindParent(row, &parent, &visible);
	return visible;
}


std::vector<std::string> ReactColumnListView::Selection()
{
	std::vector<std::string> selection;
	ForEachRow([&](ReactRow &row, int32) {
		if (row.IsSelected() && !row.IsPlaceholder()) {
			selection.push_back(row.ItemId());
		}
	});
	return selection;
}


void ReactColumnListView::SetSelection(const std::vector<std::string> &ids)
{
	fWantedSelection = ids;
	RestoreSelection();
}


void ReactColumnListView::RestoreSelection()
{
	if (fWantedSelection == Selection()) {
		return;
	}
	const std::set<std::string> wanted(fWantedSelection.begin(), fWantedSelection.end());
	DeselectAll();
	ForEachRow([&](ReactRow &row, int32) {
		if (!row.IsPlaceholder() && wanted.count(row.ItemId()) > 0) {
			AddToSelection(&row);
		}
	});
}


std::vector<SortKey> ReactColumnListView::Sort()
{
	BMessage state;
	SaveState(&state);
	std::vector<SortKey> sort;
	int32 index = 0;
	for (int32 i = 0; state.FindInt32("sortID", i, &index) == B_OK; i++) {
		if (index >= 0 && index < static_cast<int32>(fColumns.size())) {
			sort.push_back(SortKey{fColumns[index].id, state.GetBool("sortascending", i, true)});
		}
	}
	return sort;
}


void ReactColumnListView::SetSort(const std::vector<SortKey> &sort)
{
	if (sort != Sort()) {
		ClearSortColumns();
		bool add = false;
		for (const SortKey &key : sort) {
			if (BColumn *column = ColumnById(key.column)) {
				SetSortColumn(column, add, key.ascending);
				add = true;
			}
		}
	}
	fLastSort = Sort();
}


void ReactColumnListView::CheckSort()
{
	std::vector<SortKey> sort = Sort();
	if (sort == fLastSort) {
		return;
	}
	fLastSort = sort;
	std::vector<std::string> value;
	for (const SortKey &key : sort) {
		value.push_back(key.column + (key.ascending ? ":ascending" : ":descending"));
	}
	EventBridge::Post(Event{fNodeId, "sortChange", value});
}


void ReactColumnListView::AttachedToWindow()
{
	BColumnListView::AttachedToWindow();
	if (!fFilterInstalled) {
		if (BView *title = FindView("title_view")) {
			title->AddFilter(new TitleReleaseFilter(this));
			fFilterInstalled = true;
		}
	}
}


void ReactColumnListView::SelectionChanged()
{
	BColumnListView::SelectionChanged();
	if (OnWindowThread(*this)) {
		fWantedSelection = Selection();
		EventBridge::Post(Event{fNodeId, "selectionChange", fWantedSelection});
	}
}


void ReactColumnListView::ExpandOrCollapse(BRow *row, bool expand)
{
	const bool changed = row->IsExpanded() != expand;
	BColumnListView::ExpandOrCollapse(row, expand);

	const auto *reactRow = static_cast<const ReactRow*>(row);
	if (changed && reactRow->NodeId() != 0 && OnWindowThread(*this)) {
		EventBridge::Post(Event{reactRow->NodeId(), expand ? "expand" : "collapse", {}});
	}
}


status_t ReactColumnListView::Invoke(BMessage *message)
{
	(void)message;
	const auto *row = static_cast<const ReactRow*>(CurrentSelection());
	if (row != nullptr && !row->IsPlaceholder()) {
		EventBridge::Post(Event{fNodeId, "invoke", row->ItemId()});
	}
	return B_OK;
}


void ReactColumnListView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgCheckSort:
			CheckSort();
			break;
		case kMsgSelect: {
			std::vector<std::string> ids;
			const char *id = nullptr;
			for (int32 i = 0; message->FindString(kFieldId, i, &id) == B_OK; i++) {
				ids.push_back(id);
			}
			SetSelection(ids);
			SelectionChanged();
			break;
		}
		case kMsgExpand: {
			// The row may be gone; only its address was sent.
			void *target = nullptr;
			bool found = false;
			if (message->FindPointer(kFieldRow, &target) == B_OK) {
				ForEachRow([&](ReactRow &row, int32) {
					found |= &row == target;
				}, true);
			}
			if (found) {
				ExpandOrCollapse(static_cast<BRow*>(target), message->GetBool(kFieldExpand, true));
			}
			break;
		}
		case kMsgTitleClick: {
			const std::string id = message->GetString(kFieldColumn, "");
			BColumn *column = ColumnById(id);
			if (column != nullptr) {
				const std::vector<SortKey> sort = Sort();
				const bool reverse = !sort.empty() && sort[0].column == id;
				SetSortColumn(column, false, reverse ? !sort[0].ascending : true);
				CheckSort();
			}
			break;
		}
		default:
			BColumnListView::MessageReceived(message);
			break;
	}
}


void ReactColumnListView::PostSelect(const std::vector<std::string> &ids)
{
	BMessage message(kMsgSelect);
	for (const std::string &id : ids) {
		message.AddString(kFieldId, id.c_str());
	}
	CheckThrow(BMessenger(this).SendMessage(&message));
}


void ReactColumnListView::PostExpand(ReactRow *row, bool expand)
{
	BMessage message(kMsgExpand);
	message.AddPointer(kFieldRow, row);
	message.AddBool(kFieldExpand, expand);
	CheckThrow(BMessenger(this).SendMessage(&message));
}


void ReactColumnListView::PostTitleClick(const std::string &column)
{
	BMessage message(kMsgTitleClick);
	message.AddString(kFieldColumn, column.c_str());
	CheckThrow(BMessenger(this).SendMessage(&message));
}


} // HaikuReact
