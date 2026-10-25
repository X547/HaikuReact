#pragma once

#include <memory>
#include <string>

#include <View.h>

#include "IconSource.h"


namespace HaikuReact {


enum class ImageFit {
	Contain,  // whole image, aspect ratio kept, centered
	Cover,    // fills the view, aspect ratio kept, cropped
	Fill,     // stretched to the view
	None,     // natural size, centered
};


struct ImageJob;


// View backing <Image>. Decodes its source on a background thread and swaps
// the result in on the window thread. Events: load ([width, height]), error
// (message).
class ReactImageView: public BView {
private:
	const uint32 fNodeId;
	std::shared_ptr<ImageJob> fJob;
	std::unique_ptr<BBitmap> fBitmap;
	ImageFit fFit = ImageFit::Contain;

	void ApplyResult();
	BSize NaturalSize();

public:
	explicit ReactImageView(uint32 nodeId);
	~ReactImageView() final;

	const BBitmap *Bitmap() const {return fBitmap.get();}
	// Starts decoding; the current image stays until the new one is ready.
	void Load(const IconSource &source);
	void SetFit(ImageFit fit);
	// Where the image is drawn in view coordinates; empty without an image.
	BRect DrawRect() const;
	bool IsLoading() const;

	void AttachedToWindow() final;
	void DetachedFromWindow() final;
	void MessageReceived(BMessage *message) final;
	void Draw(BRect updateRect) final;

	BSize MinSize() final;
	BSize MaxSize() final;
	BSize PreferredSize() final;
};


} // HaikuReact
