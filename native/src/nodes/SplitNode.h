#pragma once

#include "ViewNode.h"

class BSplitView;


namespace HaikuReact {


// <Split>: BSplitView with user-draggable dividers between the children.
// Children use the weight and collapsible props.
class SplitNode: public ViewNode {
private:
	BSplitView *fSplitView;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildLayoutHintsChanged(Node &child, int32 index) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit SplitNode(uint32 id);

	const char *TypeName() const final {return "Split";}

	void Drag(BPoint from, BPoint to) final;
};


} // HaikuReact
