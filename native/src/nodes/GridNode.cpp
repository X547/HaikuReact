#include "GridNode.h"

#include <algorithm>
#include <stdexcept>
#include <string>

#include <GridView.h>

#include "../PropSet.h"


namespace HaikuReact {


// Sets the weights of the given columns or rows, resetting those no longer
// listed to the default weight of 1.
template<typename Setter>
static void ApplyWeights(std::vector<float> &current, const std::vector<float> &weights, Setter setWeight)
{
	const size_t count = std::max(current.size(), weights.size());
	for (size_t i = 0; i < count; i++) {
		setWeight(static_cast<int32>(i), i < weights.size() ? weights[i] : 1.0f);
	}
	current = weights;
}


GridNode::GridNode(uint32 id):
	ViewNode(id, new BGridView(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING), kFillWidthMaxSize),
	fGridView(static_cast<BGridView*>(View()))
{
}


void GridNode::AttachChild(Node &child, int32 index)
{
	(void)index;
	BGridLayout &layout = *fGridView->GridLayout();
	const LayoutHints &hints = child.Hints();

	bool added = false;
	if (BView *view = child.View(); view != nullptr) {
		added = layout.AddView(view, hints.column, hints.row, hints.columnSpan, hints.rowSpan) != nullptr;
	} else if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		added = layout.AddItem(item, hints.column, hints.row, hints.columnSpan, hints.rowSpan);
	} else {
		throw std::invalid_argument(std::string("<") + child.TypeName() + "> cannot be placed in a layout");
	}
	if (!added) {
		throw std::invalid_argument(
			std::string("<Grid>: cannot place <") + child.TypeName() + "> at column "
				+ std::to_string(hints.column) + ", row " + std::to_string(hints.row)
				+ "; the cells are already occupied"
		);
	}
}


void GridNode::DetachChild(Node &child)
{
	BGridLayout &layout = *fGridView->GridLayout();
	if (BView *view = child.View(); view != nullptr) {
		layout.RemoveView(view);
		return;
	}
	if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		layout.RemoveItem(item);
	}
}


void GridNode::ChildLayoutHintsChanged(Node &child, int32 index)
{
	// BGridLayout cannot move an item to another cell; place it again.
	DetachChild(child);
	AttachChild(child, index);
}


void GridNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	BGridLayout &layout = *fGridView->GridLayout();
	if (auto value = props.Get<Spacing>("horizontalSpacing", Spacing{B_USE_DEFAULT_SPACING})) {
		layout.SetHorizontalSpacing(value->value);
	}
	if (auto value = props.Get<Spacing>("verticalSpacing", Spacing{B_USE_DEFAULT_SPACING})) {
		layout.SetVerticalSpacing(value->value);
	}
	if (auto value = props.Get<Insets>("insets", Insets{})) {
		layout.SetInsets(value->left, value->top, value->right, value->bottom);
	}
	if (auto weights = props.Get<std::vector<float>>("columnWeights", {})) {
		ApplyWeights(fColumnWeights, *weights, [&layout](int32 column, float weight) {
			layout.SetColumnWeight(column, weight);
		});
	}
	if (auto weights = props.Get<std::vector<float>>("rowWeights", {})) {
		ApplyWeights(fRowWeights, *weights, [&layout](int32 row, float weight) {
			layout.SetRowWeight(row, weight);
		});
	}
}


} // HaikuReact
