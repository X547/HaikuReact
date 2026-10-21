#include "GroupLayoutChildren.h"

#include <stdexcept>
#include <string>

#include <GroupLayout.h>

#include "nodes/Node.h"


namespace HaikuReact {


namespace GroupLayoutChildren {


void Attach(BGroupLayout &layout, Node &child, int32 index)
{
	const float weight = child.Hints().weight;
	child.LayoutOrientationChanged(layout.Orientation());

	if (BView *view = child.View(); view != nullptr) {
		if (layout.AddView(index, view, weight) == nullptr) {
			throw std::runtime_error(std::string("failed to add <") + child.TypeName() + "> to layout");
		}
		return;
	}
	if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		if (!layout.AddItem(index, item, weight)) {
			throw std::runtime_error(std::string("failed to add <") + child.TypeName() + "> to layout");
		}
		return;
	}
	throw std::invalid_argument(std::string("<") + child.TypeName() + "> cannot be placed in a layout");
}


void Detach(BGroupLayout &layout, Node &child)
{
	// RemoveView() deletes the view's layout item and removes the view from
	// its parent; the view itself stays alive and owned by the node.
	if (BView *view = child.View(); view != nullptr) {
		layout.RemoveView(view);
		return;
	}
	if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		layout.RemoveItem(item);
	}
}


void UpdateHints(BGroupLayout &layout, Node &child, int32 index)
{
	layout.SetItemWeight(index, child.Hints().weight);
}


void ApplyLayoutDefaults(BGroupLayout &layout, const Defaults &defaults)
{
	layout.SetOrientation(defaults.direction);
	layout.SetSpacing(defaults.spacing);
	layout.SetInsets(defaults.insets.left, defaults.insets.top, defaults.insets.right, defaults.insets.bottom);
}


bool ApplyProps(BGroupLayout &layout, const PropSet &props, const Defaults &defaults)
{
	bool orientationChanged = false;
	if (auto value = props.Get<orientation>("orientation", defaults.direction)) {
		orientationChanged = *value != layout.Orientation();
		layout.SetOrientation(*value);
	}
	if (auto value = props.Get<Spacing>("spacing", Spacing{defaults.spacing})) {
		layout.SetSpacing(value->value);
	}
	if (auto value = props.Get<Insets>("insets", defaults.insets)) {
		layout.SetInsets(value->left, value->top, value->right, value->bottom);
	}
	return orientationChanged;
}


} // GroupLayoutChildren


} // HaikuReact
