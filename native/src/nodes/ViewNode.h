#pragma once

#include <Size.h>

#include "Node.h"


namespace HaikuReact {


// Default maximum of container views: fill the available width. Haiku caps a
// column at the smallest maximum of its children, and labels and buttons
// are capped at their preferred size, so containers that keep their natural
// maximum make splitters immovable and windows impossible to widen.
inline const BSize kFillWidthMaxSize(B_SIZE_UNLIMITED, B_SIZE_UNSET);


// Node backed by a BView. A view attached to a parent view belongs to that
// parent (BView deletes its children), so the node deletes its view only while
// it is the root of a detached subtree.
class ViewNode: public Node {
private:
	BView *fView;
	const BSize fDefaultMaxSize;
	bool fHidden = false;
	bool fHasMaxWidth = false;
	bool fHasMaxHeight = false;

	void SetHidden(bool hidden);
	void ApplyExplicitLayout(const PropSet &props);
	void FitMaxSizeToMinSize();

protected:
	void ApplyOwnProps(const PropSet &props) override;

public:
	// defaultMaxSize applies to dimensions without an explicit maximum prop.
	ViewNode(uint32 id, BView *view, BSize defaultMaxSize = BSize(B_SIZE_UNSET, B_SIZE_UNSET));
	~ViewNode() override;

	BView *View() const final {return fView;}

	void Dump(Napi::Object &out) const override;
};


} // HaikuReact
