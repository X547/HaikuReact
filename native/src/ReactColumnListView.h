#pragma once

#include <functional>
#include <string>
#include <vector>

#include <interface/ColumnListView.h>

#include "ColumnSpec.h"


namespace HaikuReact {


// Row of a <ColumnListView>. Placeholder rows ("Loading…") belong to no
// element. The list owns its rows and deletes them when it is deleted.
class ReactRow: public BRow {
private:
	const uint32 fNodeId;
	std::string fItemId;
	const bool fPlaceholder;

public:
	ReactRow(uint32 nodeId, const std::string &itemId, bool placeholder);

	uint32 NodeId() const {return fNodeId;}
	const std::string &ItemId() const {return fItemId;}
	bool IsPlaceholder() const {return fPlaceholder;}
	void SetItemId(const std::string &id) {fItemId = id;}
};


// BColumnListView backing <ColumnListView>. Rows are identified by id.
// Reports selection changes, invocations, expanding or collapsing and
// sorting by the user (clicks on the column titles).
class ReactColumnListView: public BColumnListView {
private:
	const uint32 fNodeId;
	std::vector<ColumnSpec> fColumns;
	// Last selection set from JS or by the user (see ReactListView).
	std::vector<std::string> fWantedSelection;
	std::vector<SortKey> fLastSort;
	bool fFilterInstalled = false;

	void CheckSort();
	BColumn *ColumnById(const std::string &id) const;

public:
	explicit ReactColumnListView(uint32 nodeId);

	const std::vector<ColumnSpec> &Columns() const {return fColumns;}
	// Replaces the columns; the list must have no rows.
	void SetColumns(const std::vector<ColumnSpec> &columns);

	// Calls f for each row in display order; only rows under expanded parents
	// unless all is set. level is 0 for top-level rows.
	void ForEachRow(const std::function<void(ReactRow &row, int32 level)> &f, bool all = false);
	bool IsRowShown(BRow *row) const;

	std::vector<std::string> Selection();
	void SetSelection(const std::vector<std::string> &ids);
	void RestoreSelection();

	std::vector<SortKey> Sort();
	void SetSort(const std::vector<SortKey> &sort);

	void AttachedToWindow() final;
	void MessageReceived(BMessage *message) final;
	void SelectionChanged() final;
	// Reports changes made on the window thread (by the user).
	void ExpandOrCollapse(BRow *row, bool expand) final;
	// Double-click or Enter: reports the selected row.
	status_t Invoke(BMessage *message = nullptr) final;

	// Act on the window thread, like a user would; for tests.
	void PostSelect(const std::vector<std::string> &ids);
	void PostExpand(ReactRow *row, bool expand);
	// Like a click on the column's title: sorts by it, or reverses the order.
	void PostTitleClick(const std::string &column);
};


} // HaikuReact
