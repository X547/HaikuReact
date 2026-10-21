#pragma once

#include <string>
#include <vector>

#include <SupportDefs.h>


namespace HaikuReact {


// "shift", "option", "control" and "command" to B_*_KEY flags.
uint32 ParseModifiers(const std::vector<std::string> &names);


} // HaikuReact
