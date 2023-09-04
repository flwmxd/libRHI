//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <vector>
#include "Definitions.h"

namespace maple
{
	namespace ShaderCompiler 
	{
		auto init() -> void;
		auto finalize() -> void;
		auto complie(const ShaderType & shader_type, 
			const char* pshader,
			std::vector<uint32_t>& spirv, const char * userDefine) -> bool;
	};
}        // namespace maple
