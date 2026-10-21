#pragma once

#include <string>

#include <TextView.h>


namespace HaikuReact {


// Text view backing <TextView>. BTextView has no modification message; the
// edit hooks report changes made by the user.
class ReactTextView: public BTextView {
private:
	const uint32 fNodeId;

	void NotifyChanged();

protected:
	void InsertText(const char *text, int32 length, int32 offset, const text_run_array *runs) final;
	void DeleteText(int32 fromOffset, int32 toOffset) final;

public:
	explicit ReactTextView(uint32 nodeId);

	void MessageReceived(BMessage *message) final;

	// Replaces the text on the window thread, like a user edit; for tests.
	void PostSetText(const std::string &text);
};


} // HaikuReact
