#pragma once

#include "Node.h"

class BSpaceLayoutItem;


namespace HaikuReact {


// Layout item without a view. A layout deletes its items, so the node owns
// the item only while it has no parent.
class SpaceNode: public Node {
private:
	BSpaceLayoutItem *fItem;

protected:
	void ApplyOwnProps(const PropSet &props) override;

	BSpaceLayoutItem &Item() const {return *fItem;}

public:
	SpaceNode(uint32 id, BSpaceLayoutItem *item);
	~SpaceNode() override;

	BLayoutItem *LayoutItem() const final;
};


// <Glue>: takes up the remaining space.
class GlueNode final: public SpaceNode {
public:
	explicit GlueNode(uint32 id);

	const char *TypeName() const final {return "Glue";}
};


} // HaikuReact
