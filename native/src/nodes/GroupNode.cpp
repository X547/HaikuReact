#include "GroupNode.h"

#include <GroupView.h>

#include "../GroupLayoutChildren.h"
#include "../PropSet.h"


namespace HaikuReact {


GroupNode::GroupNode(uint32 id):
	ViewNode(id, new BGroupView(B_VERTICAL, B_USE_DEFAULT_SPACING), kFillWidthMaxSize),
	fGroupView(static_cast<BGroupView*>(View()))
{
}


void GroupNode::AttachChild(Node &child, int32 index)
{
	GroupLayoutChildren::Attach(*fGroupView->GroupLayout(), child, index);
}


void GroupNode::DetachChild(Node &child)
{
	GroupLayoutChildren::Detach(*fGroupView->GroupLayout(), child);
}


void GroupNode::ChildLayoutHintsChanged(Node &child, int32 index)
{
	GroupLayoutChildren::UpdateHints(*fGroupView->GroupLayout(), child, index);
}


void GroupNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	BGroupLayout &layout = *fGroupView->GroupLayout();
	if (GroupLayoutChildren::ApplyProps(layout, props, {})) {
		NotifyChildrenOfOrientation(layout.Orientation());
	}
}


} // HaikuReact
