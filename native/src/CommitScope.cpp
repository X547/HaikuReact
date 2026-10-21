#include "CommitScope.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <Looper.h>


namespace HaikuReact {


static bool sActive = false;
static std::vector<BLooper*> sHeld;


void CommitScope::Begin()
{
	sActive = true;
}


void CommitScope::End()
{
	sActive = false;
	for (BLooper *looper : sHeld) {
		looper->Unlock();
	}
	sHeld.clear();
}


bool CommitScope::IsActive()
{
	return sActive;
}


void CommitScope::Hold(BLooper *looper)
{
	if (std::find(sHeld.begin(), sHeld.end(), looper) != sHeld.end()) {
		return;
	}
	if (!looper->Lock()) {
		throw std::runtime_error("failed to lock window");
	}
	sHeld.push_back(looper);
}


void CommitScope::Forget(BLooper *looper)
{
	std::erase(sHeld, looper);
}


} // HaikuReact
