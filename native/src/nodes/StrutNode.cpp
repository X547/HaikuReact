#include "StrutNode.h"

#include <ControlLook.h>
#include <SpaceLayoutItem.h>

#include "../PropSet.h"


namespace HaikuReact {


StrutNode::StrutNode(uint32 id):
	SpaceNode(id, BSpaceLayoutItem::CreateGlue())
{
	UpdateItem();
}


void StrutNode::UpdateItem()
{
	BSize min(fSize, fSize);
	BSize max(fSize, fSize);
	BSize preferred(fSize, fSize);
	if (fOrientation == B_HORIZONTAL) {
		min.height = preferred.height = -1;
		max.height = B_SIZE_UNLIMITED;
	} else if (fOrientation == B_VERTICAL) {
		min.width = preferred.width = -1;
		max.width = B_SIZE_UNLIMITED;
	}

	BSpaceLayoutItem &item = Item();
	item.SetExplicitMinSize(min);
	item.SetExplicitMaxSize(max);
	item.SetExplicitPreferredSize(preferred);
	item.InvalidateLayout();
}


void StrutNode::ApplyOwnProps(const PropSet &props)
{
	if (auto size = props.Get<Spacing>("size", Spacing{0})) {
		// Layout sizes count like BRect::Width(): one less than the pixels.
		fSize = BControlLook::ComposeSpacing(size->value) - 1;
		UpdateItem();
	}
}


void StrutNode::LayoutOrientationChanged(orientation direction)
{
	if (fOrientation == direction) {
		return;
	}
	fOrientation = direction;
	UpdateItem();
}


} // HaikuReact
