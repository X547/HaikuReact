#include "LooperLock.h"

#include <stdexcept>

#include <Looper.h>

#include "CommitScope.h"


namespace HaikuReact {


LooperLock::LooperLock(BLooper *looper)
{
	if (looper == nullptr) {
		return;
	}
	if (CommitScope::IsActive()) {
		CommitScope::Hold(looper);
		return;
	}
	if (!looper->Lock()) {
		throw std::runtime_error("failed to lock window");
	}
	fLooper = looper;
}


LooperLock::~LooperLock()
{
	if (fLooper != nullptr) {
		fLooper->Unlock();
	}
}


} // HaikuReact
