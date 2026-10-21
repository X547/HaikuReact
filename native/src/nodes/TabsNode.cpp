#include "TabsNode.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "../PropSet.h"
#include "../ReactTabView.h"
#include "TabNode.h"


namespace HaikuReact {


static TabNode &AsTab(Node &child)
{
	auto *tab = dynamic_cast<TabNode*>(&child);
	if (tab == nullptr) {
		throw std::invalid_argument(std::string("<Tabs> can only contain <Tab>, not <") + child.TypeName() + ">");
	}
	return *tab;
}


TabsNode::TabsNode(uint32 id):
	ViewNode(id, new ReactTabView(id), kFillWidthMaxSize),
	fTabView(static_cast<ReactTabView*>(View()))
{
}


void TabsNode::SelectTab(const BTab *tab)
{
	for (int32 i = 0; i < fTabView->CountTabs(); i++) {
		if (fTabView->TabAt(i) == tab) {
			fTabView->Select(i);
			return;
		}
	}
}


void TabsNode::ApplySelection()
{
	if (!fSelected.has_value() || *fSelected < 0 || *fSelected >= fTabView->CountTabs()) {
		return;
	}
	fTabView->Select(*fSelected);
}


void TabsNode::AttachChild(Node &child, int32 index)
{
	TabNode &tab = AsTab(child);
	const BTab *selected = fTabView->TabAt(fTabView->Selection());

	// BTabView can only append: take off the tabs after the insertion point
	// and append them again.
	std::vector<BTab*> following;
	while (fTabView->CountTabs() > index) {
		following.push_back(fTabView->RemoveTab(index));
	}
	fTabView->AddTab(tab.Content(), tab.Tab());
	for (BTab *item : following) {
		fTabView->AddTab(item->View(), item);
	}
	SelectTab(selected);
}


void TabsNode::DetachChild(Node &child)
{
	TabNode &tab = AsTab(child);
	const BTab *selected = fTabView->TabAt(fTabView->Selection());

	for (int32 i = 0; i < fTabView->CountTabs(); i++) {
		if (fTabView->TabAt(i) == tab.Tab()) {
			fTabView->RemoveTab(i);
			break;
		}
	}
	if (selected != tab.Tab()) {
		SelectTab(selected);
	}
}


void TabsNode::ChildListChanged()
{
	ApplySelection();
}


void TabsNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto selected = props.Get<std::optional<int32>>("selected", std::nullopt)) {
		fSelected = *selected;
		ApplySelection();
	}
}


void TabsNode::Invoke()
{
	fTabView->PostSelectNext();
}


void TabsNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	out.Set("selected", fTabView->Selection());
}


} // HaikuReact
