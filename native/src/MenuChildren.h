#pragma once

#include <memory>
#include <vector>

#include <SupportDefs.h>

class BMenu;


namespace HaikuReact {


class Node;
class PropSet;


// Shared by nodes whose children are menu entries (MenuBar, Menu). A menu
// holds one item per child node, in child order (a Menu also its "Loading…"
// item).
namespace MenuChildren {


// Checks that the child is a menu entry and sets its icon size (pixels).
void Validate(Node &child, int32 iconSize);

// iconSize (pixels) applies to the child's icon.
void Attach(BMenu &menu, Node &child, int32 index, int32 iconSize);
void Detach(BMenu &menu, Node &child);

// Applies the iconSize prop (nominal size); returns whether it changed and
// updates the children's icons.
bool ApplyIconSize(const PropSet &props, float &nominalSize, const std::vector<std::unique_ptr<Node>> &children);


} // MenuChildren


} // HaikuReact
