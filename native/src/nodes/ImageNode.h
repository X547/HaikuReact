#pragma once

#include "../IconSource.h"
#include "ViewNode.h"


namespace HaikuReact {


class ReactImageView;


// <Image>: a raster image from {path} or {data}, decoded off the JS thread.
// Its preferred size is the image size, scaled down to explicit maximums.
// Events: load ([width, height]), error (message).
class ImageNode: public ViewNode {
private:
	ReactImageView *fImageView;
	IconSource fSource;

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit ImageNode(uint32 id);

	const char *TypeName() const final {return "Image";}

	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
