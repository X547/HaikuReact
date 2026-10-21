#pragma once

#include <string>

class BTextView;


namespace HaikuReact {


// Replaces the text unless it is unchanged, keeping the selection (clamped
// to the new length), so re-applying a controlled value while the user
// types does not move the caret.
void SetTextKeepingSelection(BTextView &textView, const std::string &text);


} // HaikuReact
