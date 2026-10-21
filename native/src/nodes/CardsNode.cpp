#include "CardsNode.h"

#include <stdexcept>
#include <string>

#include <CardLayout.h>
#include <CardView.h>

#include "../PropSet.h"


namespace HaikuReact {


CardsNode::CardsNode(uint32 id):
	ViewNode(id, new BCardView("Cards"), kFillWidthMaxSize),
	fCardView(static_cast<BCardView*>(View()))
{
}


void CardsNode::ShowActiveCard()
{
	BCardLayout &layout = *fCardView->CardLayout();
	if (fActiveIndex < 0 || fActiveIndex >= layout.CountItems()) {
		layout.SetVisibleItem(static_cast<BLayoutItem*>(nullptr));
		return;
	}
	layout.SetVisibleItem(fActiveIndex);
}


void CardsNode::AttachChild(Node &child, int32 index)
{
	BCardLayout &layout = *fCardView->CardLayout();
	bool added = false;
	if (BView *view = child.View(); view != nullptr) {
		added = layout.AddView(index, view) != nullptr;
	} else if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		added = layout.AddItem(index, item);
	} else {
		throw std::invalid_argument(std::string("<") + child.TypeName() + "> cannot be placed in a layout");
	}
	if (!added) {
		throw std::runtime_error(std::string("failed to add <") + child.TypeName() + "> to <Cards>");
	}
}


void CardsNode::DetachChild(Node &child)
{
	BCardLayout &layout = *fCardView->CardLayout();
	if (BView *view = child.View(); view != nullptr) {
		layout.RemoveView(view);
		return;
	}
	if (BLayoutItem *item = child.LayoutItem(); item != nullptr) {
		layout.RemoveItem(item);
	}
}


void CardsNode::ChildListChanged()
{
	ShowActiveCard();
}


void CardsNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto value = props.Get<int32>("activeIndex", 0)) {
		fActiveIndex = *value;
		ShowActiveCard();
	}
}


void CardsNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("activeIndex", fCardView->CardLayout()->VisibleIndex());
}


} // HaikuReact
