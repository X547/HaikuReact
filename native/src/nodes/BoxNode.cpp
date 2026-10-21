#include "BoxNode.h"

#include <string>

#include <Box.h>
#include <GroupView.h>

#include "../GroupLayoutChildren.h"
#include "../PropSet.h"


namespace HaikuReact {


static const GroupLayoutChildren::Defaults kBoxDefaults = {
	.insets = {
		B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING,
		B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING
	}
};


BoxNode::BoxNode(uint32 id):
	ViewNode(id, new BBox("Box"), kFillWidthMaxSize),
	fBox(static_cast<BBox*>(View())),
	fContent(new BGroupView(B_VERTICAL, B_USE_DEFAULT_SPACING))
{
	// BBox centers its child within its frame; let the content fill it.
	fContent->SetExplicitMaxSize(kFillWidthMaxSize);
	GroupLayoutChildren::ApplyLayoutDefaults(*fContent->GroupLayout(), kBoxDefaults);
	fBox->AddChild(fContent);
}


void BoxNode::AttachChild(Node &child, int32 index)
{
	GroupLayoutChildren::Attach(*fContent->GroupLayout(), child, index);
}


void BoxNode::DetachChild(Node &child)
{
	GroupLayoutChildren::Detach(*fContent->GroupLayout(), child);
}


void BoxNode::ChildLayoutHintsChanged(Node &child, int32 index)
{
	GroupLayoutChildren::UpdateHints(*fContent->GroupLayout(), child, index);
}


void BoxNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto label = props.Get<std::optional<std::string>>("label", std::nullopt)) {
		fBox->SetLabel(label->has_value() ? (*label)->c_str() : nullptr);
	}
	if (auto border = props.Get<border_style>("border", B_FANCY_BORDER)) {
		fBox->SetBorder(*border);
	}

	BGroupLayout &layout = *fContent->GroupLayout();
	if (GroupLayoutChildren::ApplyProps(layout, props, kBoxDefaults)) {
		NotifyChildrenOfOrientation(layout.Orientation());
	}
}


void BoxNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("label", fBox->Label() != nullptr ? fBox->Label() : "");
}


} // HaikuReact
