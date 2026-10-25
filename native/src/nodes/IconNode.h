#pragma once

#include <string>

#include "../IconSource.h"
#include "ViewNode.h"


namespace HaikuReact {


class ReactIconView;


// <Icon>: an icon from any IconSource, loaded synchronously at size (default
// 16, scaled with the system font). A source without an icon shows nothing.
class IconNode: public ViewNode {
private:
	ReactIconView *fIconView;
	IconSource fSource;
	float fSize = 16;
	std::string fError;

	void Reload();

protected:
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit IconNode(uint32 id);

	const char *TypeName() const final {return "Icon";}

	void Dump(Napi::Object &out) const final;
};


// Describes a bitmap for dumps: width, height and number of visible pixels.
Napi::Value DumpBitmap(Napi::Env env, const BBitmap *bitmap);


} // HaikuReact
