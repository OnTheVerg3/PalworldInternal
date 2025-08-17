#pragma once

#ifdef PLATFORM_STEAM
#include "SDKIncludes/SDK/SDK.hpp"
#elif defined(PLATFORM_XBOX)
#include "SDKIncludesXBox/SDK/SDK.hpp"
#else
#error "Define PLATFORM_STEAM or PLATFORM_XBOX"
#endif