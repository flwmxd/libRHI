//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "vulkan/vulkan.h"

#include <string>
namespace maple
{
	namespace debug_utils
	{
		auto cmdBeginLabel(const std::string & caption)  -> void;
		auto cmdEndLabel()  -> void;
	}
}