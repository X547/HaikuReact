#pragma once

#include <string>
#include <vector>

#include <OutlineListView.h>

#include "EventBridge.h"


namespace HaikuReact {


class ReactListItem;


// List view backing <ListView>, flat or hierarchical. Items are identified by
// their ids; with the items prop the ids are indices and events report
// numbers. Reports selection changes, invocations and expanding or
// collapsing by the user.
class ReactListView: public BOutlineListView {
private:
	const uint32 fNodeId;
	bool fIndexIds = false;
	// Last selection set from JS or by the user; reapplied when items with
	// these ids come back (moved, re-added, the items prop rebuilt).
	std::vector<std::string> fWantedSelection;

	EventValue IdsValue(const std::vector<std::string> &ids) const;

protected:
	void ExpandOrCollapse(BListItem *item, bool expand) final;

public:
	explicit ReactListView(uint32 nodeId);

	void SetIndexIds(bool indexIds) {fIndexIds = indexIds;}

	// Ids of the selected items, in list order.
	std::vector<std::string> Selection() const;
	// Selects the visible items with these ids.
	void SetSelection(const std::vector<std::string> &ids);
	// Selects the visible items of the last selection again.
	void RestoreSelection();
	// Remeasures the items after a label or icon changed.
	void ItemsChanged();

	void SelectionChanged() final;
	void MessageReceived(BMessage *message) final;
	// Double-click or Enter: reports the selected item.
	status_t Invoke(BMessage *message = nullptr) final;

	// Act on the window thread, like a user would; for tests.
	void PostSelect(const std::vector<std::string> &ids);
	void PostExpand(ReactListItem *item, bool expand);
};


} // HaikuReact
