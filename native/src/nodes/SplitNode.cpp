#include "SplitNode.h"

#include <stdexcept>
#include <string>

#include <Layout.h>
#include <SplitView.h>

#include "../PropSet.h"


namespace HaikuReact {


SplitNode::SplitNode(uint32 id):
	ViewNode(id, new BSplitView(B_HORIZONTAL, B_USE_DEFAULT_SPACING), kFillWidthMaxSize),
	fSplitView(static_cast<BSplitView*>(View()))
{
}


void SplitNode::AttachChild(Node &child, int32 index)
{
	const LayoutHints &hints = child.Hints();
	child.LayoutOrientationChanged(fSplitView->Orientation());

	bool added = false;
	if (BView *view = child.View(); view != nullptr) {
		added = fSplitView->AddChild(index, view, hints.weight);
	} else if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		added = fSplitView->AddChild(index, item, hints.weight);
	} else {
		throw std::invalid_argument(std::string("<") + child.TypeName() + "> cannot be placed in a layout");
	}
	if (!added) {
		throw std::runtime_error(std::string("failed to add <") + child.TypeName() + "> to <Split>");
	}
	fSplitView->SetCollapsible(index, hints.collapsible);
}


void SplitNode::DetachChild(Node &child)
{
	BLayout &layout = *fSplitView->GetLayout();
	if (BView *view = child.View(); view != nullptr) {
		layout.RemoveView(view);
		return;
	}
	if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		layout.RemoveItem(item);
	}
}


void SplitNode::ChildLayoutHintsChanged(Node &child, int32 index)
{
	const LayoutHints &hints = child.Hints();
	fSplitView->SetItemWeight(index, hints.weight, true);
	fSplitView->SetCollapsible(index, hints.collapsible);
}


void SplitNode::Drag(BPoint from, BPoint to)
{
	// The same handlers a user's drag of a splitter goes through.
	fSplitView->MouseDown(from);
	fSplitView->MouseMoved(to, B_INSIDE_VIEW, nullptr);
	fSplitView->MouseUp(to);
}


void SplitNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto value = props.Get<orientation>("orientation", B_HORIZONTAL)) {
		const bool changed = *value != fSplitView->Orientation();
		fSplitView->SetOrientation(*value);
		if (changed) {
			NotifyChildrenOfOrientation(*value);
		}
	}
	if (auto value = props.Get<Spacing>("spacing", Spacing{B_USE_DEFAULT_SPACING})) {
		fSplitView->SetSpacing(value->value);
	}
	if (auto value = props.Get<Insets>("insets", Insets{})) {
		fSplitView->SetInsets(value->left, value->top, value->right, value->bottom);
	}
}


} // HaikuReact
