#pragma once

#include <vector>

#include "../ColumnSpec.h"
#include "ViewNode.h"


namespace HaikuReact {


class ReactColumnListView;


// <ColumnListView>: BColumnListView (private Haiku API) with <Row> children.
// It scrolls itself. While a sort is active the rows show in sort order, not
// in child order. Events: selectionChange (ids), invoke (id), sortChange
// (["column:ascending", ...]).
class ColumnListViewNode: public ViewNode {
private:
	ReactColumnListView *fListView;
	std::vector<SortKey> fSort;

	void RemoveRows();
	void AddRows();

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildListChanged() final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit ColumnListViewNode(uint32 id);

	const char *TypeName() const final {return "ColumnListView";}

	void Invoke() final;
	// An array of ids selects rows; a column id clicks that column's title.
	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
