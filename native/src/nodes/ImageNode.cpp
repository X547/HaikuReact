#include "ImageNode.h"

#include <stdexcept>
#include <string>

#include "../PropSet.h"
#include "../ReactImageView.h"
#include "IconNode.h"


namespace HaikuReact {


static ImageFit ParseFit(const std::string &name)
{
	if (name == "contain") {
		return ImageFit::Contain;
	}
	if (name == "cover") {
		return ImageFit::Cover;
	}
	if (name == "fill") {
		return ImageFit::Fill;
	}
	if (name == "none") {
		return ImageFit::None;
	}
	throw std::invalid_argument("prop \"fit\": expected \"contain\", \"cover\", \"fill\" or \"none\"");
}


ImageNode::ImageNode(uint32 id):
	ViewNode(id, new ReactImageView(id)),
	fImageView(static_cast<ReactImageView*>(View()))
{
}


void ImageNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto fit = props.Get<std::string>("fit", "contain")) {
		fImageView->SetFit(ParseFit(*fit));
	}
	if (auto source = props.Get<IconSource>("src", {}); source && !(*source == fSource)) {
		fSource = *source;
		if (!fSource.IsNone()) {
			fImageView->Load(fSource);
		}
	}
}


void ImageNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	Napi::Env env = out.Env();
	out.Set("bitmap", DumpBitmap(env, fImageView->Bitmap()));
	out.Set("loading", fImageView->IsLoading());

	const BRect rect = fImageView->DrawRect();
	Napi::Object rectOut = Napi::Object::New(env);
	rectOut.Set("x", rect.left);
	rectOut.Set("y", rect.top);
	rectOut.Set("width", rect.IsValid() ? rect.Width() + 1 : 0);
	rectOut.Set("height", rect.IsValid() ? rect.Height() + 1 : 0);
	out.Set("drawRect", rectOut);
}


} // HaikuReact
