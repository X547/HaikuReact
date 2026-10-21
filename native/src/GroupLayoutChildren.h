#pragma once

#include <InterfaceDefs.h>

#include "PropSet.h"

class BGroupLayout;


namespace HaikuReact {


class Node;


// Shared by nodes whose children live in a BGroupLayout (Window, Group, Box,
// Tab). The layout holds exactly one item per child node, so child-list
// indices are layout item indices.
namespace GroupLayoutChildren {


struct Defaults {
	orientation direction = B_VERTICAL;
	float spacing = B_USE_DEFAULT_SPACING;
	Insets insets;
};


void Attach(BGroupLayout &layout, Node &child, int32 index);
void Detach(BGroupLayout &layout, Node &child);
void UpdateHints(BGroupLayout &layout, Node &child, int32 index);

// Initial values for a new layout; JS only sends the props that are set.
void ApplyLayoutDefaults(BGroupLayout &layout, const Defaults &defaults);

// Applies orientation, spacing and insets; returns true if the orientation
// changed, in which case the caller notifies its children.
bool ApplyProps(BGroupLayout &layout, const PropSet &props, const Defaults &defaults);


} // GroupLayoutChildren


} // HaikuReact
