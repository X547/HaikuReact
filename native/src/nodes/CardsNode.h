#pragma once

#include "ViewNode.h"

class BCardView;


namespace HaikuReact {


// <Cards>: BCardView; shows only the child at activeIndex.
class CardsNode: public ViewNode {
private:
	BCardView *fCardView;
	int32 fActiveIndex = 0;

	void ShowActiveCard();

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildListChanged() final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit CardsNode(uint32 id);

	const char *TypeName() const final {return "Cards";}

	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
