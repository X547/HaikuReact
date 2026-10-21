#pragma once

#include "Node.h"

class BGroupView;
class BTab;


namespace HaikuReact {


// <Tab>: one page of a <Tabs>. The BTab owns the content BGroupView that
// holds the children; the node owns the BTab while it is not in a <Tabs>.
class TabNode: public Node {
private:
	BGroupView *fContent;
	BTab *fTab;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildLayoutHintsChanged(Node &child, int32 index) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit TabNode(uint32 id);
	~TabNode() final;

	const char *TypeName() const final {return "Tab";}

	BTab *Tab() const {return fTab;}
	BView *Content() const;

	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
