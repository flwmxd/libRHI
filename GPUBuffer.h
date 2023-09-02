//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Definitions.h"

namespace maple
{
	/**
	 * refactor index/vertex/uniform/ssbo in the future.
	 */

	class  GPUBuffer
	{
	  public:
		virtual auto copy(CommandBuffer *cmd, GPUBuffer *to, const BufferCopy &copy) const -> void
		{
			__debugbreak();
		};

		virtual auto handle() const -> void *
		{
			return nullptr;
		};
	};
};        // namespace maple
