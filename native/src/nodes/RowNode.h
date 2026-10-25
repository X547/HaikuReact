#pragma once

#include <string>

#include "../ColumnSpec.h"
#include "Node.h"


namespace HaikuReact {


class ReactColumnListView;
class ReactRow;


// <Row>: a row of a <ColumnListView>, with <Row> children shown below it
// when it is expanded. While the row is in a list, the list owns its BRow;
// the BRow is created when the row is added and deleted when it is removed.
// Placeholder, hasChildren, loading and expanded work as for <ListItem>.
// Events: expand, collapse (by the user).
class RowNode: public Node {
private:
	ReactColumnListView *fList = nullptr;
	ReactRow *fRow = nullptr;
	ReactRow *fPlaceholder = nullptr;
	std::string fItemId;
	CellMap fCells;
	std::string fLoadingLabel;
	bool fHasChildren = false;
	bool fLoading = false;
	// While not in a list; in a list the row holds the state.
	bool fExpanded = false;

	bool NeedsPlaceholder() const;
	void AddPlaceholder();
	void UpdatePlaceholder();
	void SetExpanded(bool expanded);
	void UpdateFields();

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildListChanged() final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit RowNode(uint32 id);

	const char *TypeName() const final {return "Row";}

	bool IsExpanded() const;
	// Adds the rows of the subtree; index is among the parent's rows.
	void AddToList(ReactColumnListView &list, int32 index, ReactRow *parent);
	void RemoveFromList();

	// Expands (true) or collapses (false) like a click on the arrow.
	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


RowNode &AsRow(Node &parent, Node &child);


} // HaikuReact
