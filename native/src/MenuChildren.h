#pragma once

#include <SupportDefs.h>

class BMenu;


namespace HaikuReact {


class Node;


// Shared by nodes whose children are menu entries (MenuBar, Menu). The menu
// holds exactly one item per child node, so child-list indices are item
// indices.
namespace MenuChildren {


void Attach(BMenu &menu, Node &child, int32 index);
void Detach(BMenu &menu, Node &child);


} // MenuChildren


} // HaikuReact
