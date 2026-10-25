#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../IconSource.h"
#include "Node.h"


namespace HaikuReact {


class ReactListItem;
class ReactListView;


// <ListItem>: an item of a <ListView>, with <ListItem> children shown below
// it when it is expanded. The node owns its item and, while one is needed,
// a "Loading…" placeholder child: when loading is set, or when hasChildren
// is set and there are no children yet.
// Events: expand, collapse (by the user).
class ListItemNode: public Node {
private:
	std::unique_ptr<ReactListItem> fItem;
	std::unique_ptr<ReactListItem> fPlaceholder;
	// The list the subtree is shown in, if any.
	ReactListView *fList = nullptr;
	std::string fLabel;
	IconSource fIconSource;
	std::string fLoadingLabel;
	bool fHasChildren = false;
	bool fLoading = false;
	// While not in a list; in a list the item holds the state.
	bool fExpanded = false;

	bool NeedsPlaceholder() const;
	void UpdatePlaceholder();
	void SetExpanded(bool expanded);
	// Redraws the item, e.g. after its arrow appeared or went away.
	void InvalidateLatch();
	void ApplyLabelAndIcon();
	// Replaces the items once they left a list: BListItem keeps list state
	// that cannot be reset.
	void RenewItems();

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildListChanged() final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit ListItemNode(uint32 id);
	~ListItemNode() final;

	const char *TypeName() const final {return "ListItem";}

	ReactListItem *Item() const {return fItem.get();}
	bool IsExpanded() const;

	// Adds the items of the subtree to the list at a full-list index;
	// returns the index after them.
	int32 AddToList(ReactListView &list, int32 fullIndex, uint32 level);
	void RemoveFromList();
	// Full-list index after the last item of the subtree.
	int32 EndIndex() const;

	// Expands (true) or collapses (false) like a click on the arrow.
	void SimulateInput(const EventValue &value) final;
	void Dump(Napi::Object &out) const final;
};


namespace ListChildren {


// Full-list index for a child inserted at a child-list index; endIndex is
// the index after the parent's last descendant.
int32 InsertionIndex(const ReactListView &list, const std::vector<std::unique_ptr<Node>> &children, int32 index,
	int32 endIndex);

ListItemNode &AsItem(Node &parent, Node &child);


} // ListChildren


} // HaikuReact
