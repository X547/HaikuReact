#pragma once

#include <vector>

#include "ViewNode.h"

class BGridView;


namespace HaikuReact {


// <Grid>: BGridView. Children are placed by their column/row/columnSpan/
// rowSpan props; the child order does not matter.
class GridNode: public ViewNode {
private:
	BGridView *fGridView;
	std::vector<float> fColumnWeights;
	std::vector<float> fRowWeights;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildLayoutHintsChanged(Node &child, int32 index) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit GridNode(uint32 id);

	const char *TypeName() const final {return "Grid";}
};


} // HaikuReact
