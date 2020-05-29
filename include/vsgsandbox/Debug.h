#pragma once

#include <iostream>

// debugging output

#if defined(VSGSANDBOX_DEBUG)
#define VSGSB_DEBUG std::clog
#else
#define VSGSB_DEBUG false && std::clog
#endif
