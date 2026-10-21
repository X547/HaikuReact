#pragma once

#include <optional>

#include "SpaceNode.h"


namespace HaikuReact {


// <Strut>: fixed space along the orientation of the parent group or split;
// a square in other containers.
class StrutNode final: public SpaceNode {
private:
	float fSize = -1;
	std::optional<orientation> fOrientation;

	void UpdateItem();

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit StrutNode(uint32 id);

	const char *TypeName() const final {return "Strut";}

	void LayoutOrientationChanged(orientation direction) final;
};


} // HaikuReact
