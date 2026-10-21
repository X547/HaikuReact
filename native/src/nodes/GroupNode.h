#pragma once

#include "ViewNode.h"

class BGroupView;


namespace HaikuReact {


// <Group>: BGroupView. A view-backed container is required because a nested
// BLayout without its own view deletes its items when it is re-targeted,
// which happens when React attaches a prebuilt subtree.
class GroupNode: public ViewNode {
private:
	BGroupView *fGroupView;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildLayoutHintsChanged(Node &child, int32 index) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit GroupNode(uint32 id);

	const char *TypeName() const final {return "Group";}
};


} // HaikuReact
