#include "KeyModifiers.h"

#include <stdexcept>

#include <InterfaceDefs.h>


namespace HaikuReact {


uint32 ParseModifiers(const std::vector<std::string> &names)
{
	uint32 modifiers = 0;
	for (const std::string &name : names) {
		if (name == "shift") {
			modifiers |= B_SHIFT_KEY;
		} else if (name == "option") {
			modifiers |= B_OPTION_KEY;
		} else if (name == "control") {
			modifiers |= B_CONTROL_KEY;
		} else if (name == "command") {
			modifiers |= B_COMMAND_KEY;
		} else {
			throw std::invalid_argument(
				"modifiers: expected \"shift\", \"option\", \"control\" or \"command\", not \"" + name + "\""
			);
		}
	}
	return modifiers;
}


} // HaikuReact
