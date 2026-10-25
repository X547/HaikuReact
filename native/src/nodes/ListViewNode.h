#pragma once

#include <memory>
#include <vector>

#include "../PropSet.h"
#include "ViewNode.h"


namespace HaikuReact {


class ReactListItem;
class ReactListView;


// <ListView>: BOutlineListView of <ListItem> children, or of the entries of
// the items prop (whose ids are their indices); place it in a <Scroll> to
// scroll. BListView does not own its items, the nodes do.
// Events: selectionChange (ids, or indices with items), invoke (id or
// index; double-click or Enter).
class ListViewNode: public ViewNode {
private:
	ReactListView *fListView;
	std::vector<LabeledItem> fLabels;
	std::vector<std::unique_ptr<ReactListItem>> fItems;

	void SetItems(const std::vector<LabeledItem> &labels);

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildListChanged() final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit ListViewNode(uint32 id);
	~ListViewNode() final;

	const char *TypeName() const final {return "ListView";}
	bool ScrollsItself() const final {return true;}

	void Invoke() final;
	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
