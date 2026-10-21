#pragma once

#include "ViewNode.h"

class BBox;
class BGroupView;


namespace HaikuReact {


// <Box>: BBox with an optional label. BBox lays out a single child view, so
// the children go into an inner BGroupView that takes the group props.
class BoxNode: public ViewNode {
private:
	BBox *fBox;
	BGroupView *fContent;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildLayoutHintsChanged(Node &child, int32 index) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit BoxNode(uint32 id);

	const char *TypeName() const final {return "Box";}

	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
