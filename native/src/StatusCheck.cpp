#include "StatusCheck.h"

#include <system_error>


namespace HaikuReact {


void CheckThrow(status_t res)
{
	if (res < B_OK) {
		throw std::system_error(res, std::generic_category());
	}
}


} // HaikuReact
