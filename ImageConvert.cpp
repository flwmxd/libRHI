//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#include "ImageConvert.h"
#include "SimdChecker.h"

namespace maple
{
	namespace ImageConverter
	{
		namespace
		{//https://github.com/barbudreadmon/libretro-ppsspp/blob/master/Common/ColorConv.cpp
			constexpr uint8_t table4[0x10] = {
				0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
				0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

			constexpr uint8_t table5[0x20] = {
				0x00, 0x08, 0x10, 0x19, 0x21, 0x29, 0x31, 0x3A, 0x42, 0x4A, 0x52,
				0x5A, 0x63, 0x6B, 0x73, 0x7B, 0x84, 0x8C, 0x94, 0x9C, 0xA5, 0xAD,
				0xB5, 0xBD, 0xC5, 0xCE, 0xD6, 0xDE, 0xE6, 0xEF, 0xF7, 0xFF };
			constexpr uint8_t table6[0x40] = {
				0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2D, 0x31, 0x35, 0x39, 0x3D,
				0x41, 0x45, 0x49, 0x4D, 0x51, 0x55, 0x59, 0x5D, 0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D,
				0x82, 0x86, 0x8A, 0x8E, 0x92, 0x96, 0x9A, 0x9E, 0xA2, 0xA6, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA, 0xBE,
				0xC2, 0xC6, 0xCA, 0xCE, 0xD2, 0xD7, 0xDB, 0xDF, 0xE3, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFB, 0xFF };

			struct color4444
			{
				uint8_t b : 4;
				uint8_t g : 4;
				uint8_t r : 4;
				uint8_t a : 4;
			};

			struct color8888
			{
				uint8_t b;
				uint8_t g;
				uint8_t r;
				uint8_t a;
			};

			struct color888
			{
				uint8_t b;
				uint8_t g;
				uint8_t r;
			};

			struct color565
			{
				uint16_t b : 5;
				uint16_t g : 6;
				uint16_t r : 5;
			};

			inline auto convert4444To8888WithSoftware(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
			{
				auto pixels4444 = reinterpret_cast<const color4444*>(in);
				auto pixelsout = reinterpret_cast<color8888*>(out);

				for (auto i = 0; i < pixels; ++i)
				{
					const auto p = pixels4444[i];
					pixelsout[i] = { table4[p.r], table4[p.g], table4[p.b], table4[p.a] };
				}
			}

			inline auto convert4444To8888WithHardware(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
			{
#	ifdef __SSE__
				const __m128i mask4 = _mm_set1_epi16(0x000f);
				const __m128i mask8 = _mm_set1_epi16(0x00ff);
				const __m128i factor = _mm_set1_epi16(0x0011);

				const __m128i* srcp = (const __m128i*) in;
				__m128i* dstp = (__m128i*) out;
				uint32_t       sseChunks = pixels / 8;

				for (int32_t i = 0; i < sseChunks; ++i)
				{
					const __m128i c = _mm_load_si128(&srcp[i]);
					// Let's just grab R000 R000, without swizzling yet.
					__m128i b = _mm_mullo_epi16(_mm_and_si128(c, mask4), factor);        // 16 * 8
					// And then 00G0 00G0.
					__m128i g = _mm_mullo_epi16(_mm_and_si128(_mm_srli_epi16(c, 4), mask4), factor);        //ÓÒÒÆËÄÎ»
					g = _mm_slli_epi16(g, 8);                                                       //
					// Now B000 B000.
					__m128i r = _mm_mullo_epi16(_mm_and_si128(_mm_srli_epi16(c, 8), mask4), factor);
					//b = _mm_slli_epi16(b, 8);
					// And lastly 00A0 00A0.  No mask needed, we have a wall.
					__m128i a = _mm_mullo_epi16(_mm_srli_epi16(c, 12), factor);
					a = _mm_slli_epi16(a, 8);
					// We swizzle after combining - R0G0 R0G0 and B0A0 B0A0 -> RRGG RRGG and BBAA BBAA.
					__m128i rg = _mm_or_si128(r, g);
					__m128i ba = _mm_or_si128(b, a);
					rg = _mm_or_si128(rg, _mm_slli_epi16(rg, 4));
					ba = _mm_or_si128(ba, _mm_slli_epi16(ba, 4));
					// And then we can store.
					_mm_store_si128(&dstp[i * 2 + 0], _mm_unpacklo_epi16(rg, ba));
					_mm_store_si128(&dstp[i * 2 + 1], _mm_unpackhi_epi16(rg, ba));
				}

				auto pixels4444 = reinterpret_cast<const color4444*>(in);
				auto pixelsout = reinterpret_cast<color8888*>(out);
				//remain..
				for (int32_t i = sseChunks * 8; i < pixels; ++i)
				{
					auto p = pixels4444[i];
					pixelsout[i] = { table4[p.r], table4[p.g], table4[p.b], table4[p.a] };
				}
#endif
			}

			inline auto convert565To8888WithSoftware(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
			{
				auto pixels565 = reinterpret_cast<const color565*>(in);
				auto pixelsout = reinterpret_cast<color8888*>(out);

				for (auto i = 0; i < pixels; ++i)
				{
					auto p = pixels565[i];
					pixelsout[i] = { table5[p.r], table6[p.g], table5[p.b], 255 };
				}
			}

			inline auto convert565To8888Hardware(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
			{
#	ifdef __SSE__
				const __m128i mask5 = _mm_set1_epi16(0x001f);
				const __m128i mask6 = _mm_set1_epi16(0x003f);
				const __m128i mask8 = _mm_set1_epi16(0x00ff);

				const __m128i* srcp = (const __m128i*) in;
				__m128i* dstp = (__m128i*) out;
				auto           sseChunks = pixels / 8;

				for (int32_t i = 0; i < sseChunks; ++i)
				{
					const __m128i c = _mm_load_si128(&srcp[i]);

					// Swizzle, resulting in RR00 RR00.
					__m128i r = _mm_and_si128(c, mask5);
					r = _mm_or_si128(_mm_slli_epi16(r, 3), _mm_srli_epi16(r, 2));
					r = _mm_and_si128(r, mask8);

					// This one becomes 00GG 00GG.
					__m128i g = _mm_and_si128(_mm_srli_epi16(c, 5), mask6);
					g = _mm_or_si128(_mm_slli_epi16(g, 2), _mm_srli_epi16(g, 4));
					g = _mm_slli_epi16(g, 8);

					// Almost done, we aim for BB00 BB00 again here.
					__m128i b = _mm_and_si128(_mm_srli_epi16(c, 11), mask5);
					b = _mm_or_si128(_mm_slli_epi16(b, 3), _mm_srli_epi16(b, 2));
					b = _mm_and_si128(b, mask8);

					// Always set to 00FF 00FF.
					__m128i a = _mm_slli_epi16(mask8, 8);

					// Now combine them, RRGG RRGG and BBAA BBAA, and then interleave.
					const __m128i bg = _mm_or_si128(b, g);
					const __m128i ra = _mm_or_si128(r, a);
					_mm_store_si128(&dstp[i * 2 + 0], _mm_unpacklo_epi16(bg, ra));
					_mm_store_si128(&dstp[i * 2 + 1], _mm_unpackhi_epi16(bg, ra));
				}
				uint32_t i = sseChunks * 8;

				auto pixels565 = reinterpret_cast<const color565*>(in);
				auto pixelsout = reinterpret_cast<color8888*>(out);

				for (int32_t x = i; x < pixels; x++)
				{
					auto p = pixels565[i];
					pixelsout[i] = { table5[p.r], table6[p.g], table5[p.b], 255 };
				}
#endif
			}

			inline auto convert888To8888WithSoftware(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
			{
				auto pixels888 = reinterpret_cast<const color888*>(in);
				auto pixelsout = reinterpret_cast<color8888*>(out);

				for (auto i = 0; i < pixels; ++i)
				{
					auto p = pixels888[i];
					pixelsout[i] = { p.r, p.g, p.b, 255 };
				}
			}
		}


		auto convert4444To8888(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
		{
			if (isSimdAvailable)
			{
				convert4444To8888WithHardware(in, out, pixels);
			}
			else
			{
				convert4444To8888WithSoftware(in, out, pixels);
			}
		}

		auto convert565To8888(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
		{
			if (isSimdAvailable)
			{
				convert565To8888Hardware(in, out, pixels);
			}
			else
			{
				convert565To8888WithSoftware(in, out, pixels);
			}
		}

		auto convert888To8888(const uint8_t* in, uint8_t* out, int32_t pixels) -> void
		{//sse is not available...
			convert888To8888WithSoftware(in, out, pixels);
		}
	};
}