#include "ReactImageView.h"

#include <math.h>

#include <algorithm>
#include <mutex>
#include <thread>

#include <Bitmap.h>
#include <LayoutUtils.h>
#include <Messenger.h>

#include "EventBridge.h"
#include "ReactIconView.h"


namespace HaikuReact {


constexpr uint32 kMsgImageDecoded = 'HRid';


// One decoding run, shared by the view and its thread. The thread may outlive
// the view; whoever lets go last frees the result.
struct ImageJob {
	std::mutex lock;
	// The view while it is attached to a window.
	BMessenger target;
	bool done = false;
	std::unique_ptr<BBitmap> bitmap;
	std::string error;
};


static void Decode(std::shared_ptr<ImageJob> job, IconSource source)
{
	std::string error;
	std::unique_ptr<BBitmap> bitmap = LoadImage(source, error);

	BMessenger target;
	{
		std::lock_guard guard(job->lock);
		job->bitmap = std::move(bitmap);
		job->error = std::move(error);
		job->done = true;
		target = job->target;
	}
	// Fails harmlessly once the view is gone; AttachedToWindow() picks up
	// results that arrive while it is detached.
	target.SendMessage(kMsgImageDecoded);
}


ReactImageView::ReactImageView(uint32 nodeId):
	BView("Image", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fNodeId(nodeId)
{
}


ReactImageView::~ReactImageView() = default;


void ReactImageView::Load(const IconSource &source)
{
	auto job = std::make_shared<ImageJob>();
	if (Window() != nullptr) {
		job->target = BMessenger(this);
	}
	fJob = job;
	std::thread(Decode, job, source).detach();
}


bool ReactImageView::IsLoading() const
{
	return fJob != nullptr;
}


void ReactImageView::ApplyResult()
{
	if (fJob == nullptr) {
		return;
	}
	std::unique_ptr<BBitmap> bitmap;
	std::string error;
	{
		std::lock_guard guard(fJob->lock);
		if (!fJob->done) {
			return;
		}
		bitmap = std::move(fJob->bitmap);
		error = std::move(fJob->error);
	}
	fJob.reset();

	if (bitmap == nullptr) {
		EventBridge::Post(Event{fNodeId, "error", error});
		return;
	}
	fBitmap = std::move(bitmap);
	InvalidateLayout();
	Invalidate();
	const BRect bounds = fBitmap->Bounds();
	EventBridge::Post(Event{fNodeId, "load", std::vector<int32>{bounds.IntegerWidth() + 1, bounds.IntegerHeight() + 1}});
}


void ReactImageView::SetFit(ImageFit fit)
{
	if (fit == fFit) {
		return;
	}
	fFit = fit;
	InvalidateLayout();
	Invalidate();
}


BRect ReactImageView::DrawRect() const
{
	if (fBitmap == nullptr) {
		return BRect();
	}
	const BRect bounds = Bounds();
	const float viewWidth = bounds.Width() + 1;
	const float viewHeight = bounds.Height() + 1;
	const float imageWidth = fBitmap->Bounds().Width() + 1;
	const float imageHeight = fBitmap->Bounds().Height() + 1;

	float width = imageWidth;
	float height = imageHeight;
	switch (fFit) {
		case ImageFit::Fill:
			width = viewWidth;
			height = viewHeight;
			break;
		case ImageFit::Contain:
		case ImageFit::Cover: {
			const float scaleX = viewWidth / imageWidth;
			const float scaleY = viewHeight / imageHeight;
			const float scale = fFit == ImageFit::Contain ? std::min(scaleX, scaleY) : std::max(scaleX, scaleY);
			width = roundf(imageWidth * scale);
			height = roundf(imageHeight * scale);
			break;
		}
		case ImageFit::None:
			break;
	}
	const float left = bounds.left + floorf((viewWidth - width) / 2);
	const float top = bounds.top + floorf((viewHeight - height) / 2);
	return BRect(left, top, left + width - 1, top + height - 1);
}


void ReactImageView::AttachedToWindow()
{
	BView::AttachedToWindow();
	AdoptParentColors();
	if (fJob != nullptr) {
		{
			std::lock_guard guard(fJob->lock);
			fJob->target = BMessenger(this);
		}
		ApplyResult();
	}
}


void ReactImageView::DetachedFromWindow()
{
	if (fJob != nullptr) {
		std::lock_guard guard(fJob->lock);
		fJob->target = BMessenger();
	}
	BView::DetachedFromWindow();
}


void ReactImageView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgImageDecoded:
			ApplyResult();
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}


void ReactImageView::Draw(BRect updateRect)
{
	(void)updateRect;
	if (fBitmap != nullptr) {
		DrawIconBitmap(*this, *fBitmap, DrawRect());
	}
}


BSize ReactImageView::NaturalSize()
{
	if (fBitmap == nullptr) {
		return BSize(0, 0);
	}
	// The image size, scaled down to fit explicit maximums.
	float width = fBitmap->Bounds().Width() + 1;
	float height = fBitmap->Bounds().Height() + 1;
	const BSize maxSize = ExplicitMaxSize();
	float scale = 1;
	if (maxSize.width >= 0 && maxSize.width < B_SIZE_UNLIMITED) {
		scale = std::min(scale, (maxSize.width + 1) / width);
	}
	if (maxSize.height >= 0 && maxSize.height < B_SIZE_UNLIMITED) {
		scale = std::min(scale, (maxSize.height + 1) / height);
	}
	return BSize(roundf(width * scale) - 1, roundf(height * scale) - 1);
}


// Images do not shrink below their natural size unless minWidth/minHeight
// allow it: windows are sized before the image is decoded, and the layout
// would otherwise squeeze it to nothing.
BSize ReactImageView::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), NaturalSize());
}


BSize ReactImageView::MaxSize()
{
	BSize size(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED);
	if (fFit == ImageFit::None && fBitmap != nullptr) {
		size = fBitmap->Bounds().Size();
	}
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), size);
}


BSize ReactImageView::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), NaturalSize());
}


} // HaikuReact
