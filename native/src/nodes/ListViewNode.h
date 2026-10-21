#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ViewNode.h"

class BStringItem;


namespace HaikuReact {


class ReactListView;


// <ListView>: BListView of text items; place it in a <Scroll> to scroll.
// BListView does not own its items, the node does.
// Events: selectionChange (indices), invoke (index; double-click or Enter).
class ListViewNode: public ViewNode {
private:
	ReactListView *fListView;
	std::vector<std::string> fLabels;
	std::vector<std::unique_ptr<BStringItem>> fItems;
	std::vector<int32> fSelection;

	void SetItems(const std::vector<std::string> &labels);

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit ListViewNode(uint32 id);
	~ListViewNode() final;

	const char *TypeName() const final {return "ListView";}
	bool ScrollsItself() const final {return true;}

	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
