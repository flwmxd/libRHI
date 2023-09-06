//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(__x86_64__) || defined(__i386__) || defined(_WIN32)
#ifndef __SSE__
#   define __SSE__
#endif
#   include <emmintrin.h>
#endif

#if defined(__ANDROID__)
#  include <cpu-features.h>
#endif

#if defined(__ARM_NEON__)
#  include <arm_neon.h>

#  if defined(__ANDROID__) && defined(__arm__)
    // NEON support must be checked at runtime on 32-bit Android
    class AndroidNeonChecker final
    {
    public:
        AndroidNeonChecker() noexcept:
            neonAvailable(android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
                          (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0)
        {
        }

        operator bool() const noexcept { return neonAvailable; }

    private:
        bool neonAvailable;
    };

    extern AndroidNeonChecker isSimdAvailable;
#  else
	constexpr auto isSimdAvailable = true;
#  endif
#elif defined(__SSE__)
    constexpr auto isSimdAvailable = true;
#else
    constexpr auto isSimdAvailable = false;
#endif
