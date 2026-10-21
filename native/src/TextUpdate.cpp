#include "TextUpdate.h"

#include <algorithm>

#include <TextView.h>


namespace HaikuReact {


void SetTextKeepingSelection(BTextView &textView, const std::string &text)
{
	if (text == textView.Text()) {
		return;
	}
	int32 start = 0;
	int32 end = 0;
	textView.GetSelection(&start, &end);
	textView.SetText(text.c_str());

	const int32 length = textView.TextLength();
	textView.Select(std::min(start, length), std::min(end, length));
}


} // HaikuReact
