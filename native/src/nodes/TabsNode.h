#pragma once

#include <optional>

#include "ViewNode.h"

class BTab;


namespace HaikuReact {


class ReactTabView;


// <Tabs>: BTabView whose children are <Tab> nodes. The selected prop makes
// the selection controlled; without it the user's choice is kept.
// Events: change (index).
class TabsNode: public ViewNode {
private:
	ReactTabView *fTabView;
	std::optional<int32> fSelected;

	void SelectTab(const BTab *tab);
	void ApplySelection();

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildListChanged() final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit TabsNode(uint32 id);

	const char *TypeName() const final {return "Tabs";}

	void Invoke() final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
