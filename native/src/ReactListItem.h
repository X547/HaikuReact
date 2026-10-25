#pragma once

#include <string>

#include <ListItem.h>

#include "BitmapCache.h"


namespace HaikuReact {


// Item of a <ListView>: optional icon and a label. Placeholder items
// ("Loading…") are drawn dimmed and belong to no element.
class ReactListItem: public BListItem {
private:
	// Node of the <ListItem>; 0 for placeholders and items of the items prop.
	const uint32 fNodeId;
	const bool fPlaceholder;
	std::string fItemId;
	std::string fLabel;
	SharedBitmapRef fIcon;
	int32 fIconSize;
	float fBaselineOffset = 0;

public:
	ReactListItem(uint32 nodeId, bool placeholder = false);

	uint32 NodeId() const {return fNodeId;}
	bool IsPlaceholder() const {return fPlaceholder;}
	const std::string &ItemId() const {return fItemId;}
	const std::string &Label() const {return fLabel;}
	const SharedBitmapRef &Icon() const {return fIcon;}

	void SetItemId(const std::string &id) {fItemId = id;}
	// Changes to label and icon need a list relayout; see ReactListView::ItemChanged.
	void SetLabel(const std::string &label) {fLabel = label;}
	void SetIcon(SharedBitmapRef icon) {fIcon = std::move(icon);}
	int32 IconSize() const {return fIconSize;}

	void DrawItem(BView *owner, BRect frame, bool complete) final;
	void Update(BView *owner, const BFont *font) final;
};


} // HaikuReact
