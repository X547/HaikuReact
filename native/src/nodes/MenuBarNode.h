#pragma once

#include "ViewNode.h"

class BMenuBar;


namespace HaikuReact {


// <MenuBar>: BMenuBar; usually the first child of a <Window>. The first menu
// bar in a window handles Command+Escape keyboard navigation.
class MenuBarNode: public ViewNode {
private:
	BMenuBar *fMenuBar;

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;

public:
	explicit MenuBarNode(uint32 id);

	const char *TypeName() const final {return "MenuBar";}
};


} // HaikuReact
