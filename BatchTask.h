//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>

namespace maple
{
	class CommandBuffer;
	class BatchTask
	{
	  public:
		using Ptr = std::shared_ptr<BatchTask>;
		virtual auto execute(const CommandBuffer* cmd) -> void = 0;
		static auto  create()->Ptr;
	};
};        // namespace maple