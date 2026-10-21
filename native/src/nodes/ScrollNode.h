#pragma once

#include "ViewNode.h"

class BScrollView;


namespace HaikuReact {


class ReactScrollViewport;


// <Scroll>: BScrollView around a single child. Text and list views are the
// scroll target themselves; other content is scrolled by a viewport. The
// horizontal and vertical props choose the scroll bars and are read at
// creation only.
class ScrollNode: public ViewNode {
private:
	BScrollView *fScrollView;
	ReactScrollViewport *fViewport;
	bool fOwnsViewport = false;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	ScrollNode(uint32 id, const PropSet &props);
	~ScrollNode() final;

	const char *TypeName() const final {return "Scroll";}

	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
