#include "TabNode.h"

#include <string>

#include <GroupView.h>
#include <TabView.h>

#include "../GroupLayoutChildren.h"
#include "../PropSet.h"
#include "ViewNode.h"


namespace HaikuReact {


static const GroupLayoutChildren::Defaults kTabDefaults = {
	.insets = {
		B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING,
		B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING
	}
};


TabNode::TabNode(uint32 id):
	Node(id),
	fContent(new BGroupView("Tab", B_VERTICAL, B_USE_DEFAULT_SPACING)),
	fTab(new BTab(fContent))
{
	fContent->SetExplicitMaxSize(kFillWidthMaxSize);
	GroupLayoutChildren::ApplyLayoutDefaults(*fContent->GroupLayout(), kTabDefaults);
}


TabNode::~TabNode()
{
	// Inside a <Tabs> the BTabView owns the tab.
	if (Parent() == nullptr) {
		delete fTab;
	}
}


BView *TabNode::Content() const
{
	return fContent;
}


void TabNode::AttachChild(Node &child, int32 index)
{
	GroupLayoutChildren::Attach(*fContent->GroupLayout(), child, index);
}


void TabNode::DetachChild(Node &child)
{
	GroupLayoutChildren::Detach(*fContent->GroupLayout(), child);
}


void TabNode::ChildLayoutHintsChanged(Node &child, int32 index)
{
	GroupLayoutChildren::UpdateHints(*fContent->GroupLayout(), child, index);
}


void TabNode::ApplyOwnProps(const PropSet &props)
{
	if (auto label = props.Get<std::string>("label", "")) {
		fTab->SetLabel(label->c_str());
	}

	BGroupLayout &layout = *fContent->GroupLayout();
	if (GroupLayoutChildren::ApplyProps(layout, props, kTabDefaults)) {
		NotifyChildrenOfOrientation(layout.Orientation());
	}
}


void TabNode::Dump(Napi::Object &out) const
{
	Node::Dump(out);
	out.Set("label", fTab->Label() != nullptr ? fTab->Label() : "");
	out.Set("selected", fTab->IsSelected());
}


} // HaikuReact
