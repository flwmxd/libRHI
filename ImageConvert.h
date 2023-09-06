//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <cstdint>

namespace maple
{
	namespace ImageConverter
	{
		//RGBA4444 -> RGBA8888
		auto convert4444To8888(const uint8_t * in, uint8_t* out, int32_t pixels) -> void;
		//R5G6B6-> RGBA8888
		auto convert565To8888(const uint8_t* in, uint8_t* out, int32_t pixels) -> void;
		//RGB888 -> RGBA8888
		auto convert888To8888(const uint8_t* in, uint8_t* out, int32_t pixels) -> void;
	};
}