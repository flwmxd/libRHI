//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#include "SimdChecker.h"

#if defined(__ANDROID__) && defined(__ARM_NEON__) && defined(__arm__)
    // NEON support must be checked at runtime on 32-bit Android
    AndroidNeonChecker isSimdAvailable;
#endif