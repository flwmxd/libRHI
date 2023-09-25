//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdarg.h>
#include <string>
struct lua_State;

namespace maple
{

	class Console
	{
	public:
		static auto  init() -> void;
		static auto& getLogger()
		{
			return logger;
		}

	private:
		static std::shared_ptr<spdlog::logger> logger;
	};
};        // namespace maple

#define LOGV(...) maple::Console::getLogger()->trace(__VA_ARGS__)
#define LOGI(...) maple::Console::getLogger()->info(__VA_ARGS__)
#define LOGW(...) maple::Console::getLogger()->warn(__VA_ARGS__)
#define LOGE(...) maple::Console::getLogger()->error(__VA_ARGS__)
#define LOGC(...) maple::Console::getLogger()->critical(__VA_ARGS__)

#define MAPLE_ASSERT(condition, ...)                                                         \
	{                                                                                        \
		if (!(condition))                                                                    \
		{                                                                                    \
			LOGE("Assertion Failed : {0} . {1} : {2}", __VA_ARGS__, __FUNCTION__, __LINE__); \
			__debugbreak();																	 \
		}                                                                                    \
	}

#ifdef MAPLE_PROFILE
#	define TRACY_CALLSTACK 1 //win
#	include <public/tracy/Tracy.hpp>
#	define PROFILE_SCOPE(name) ZoneScopedN(name)
#	define PROFILE_FUNCTION() ZoneScoped
#	define PROFILE_FRAMEMARKER() FrameMark
#	define PROFILE_LOCK(type, var, name) TracyLockableN(type, var, name)
#	define PROFILE_LOCKMARKER(var) LockMark(var)
#	define PROFILE_SETTHREADNAME(name) tracy::SetThreadName(name)

#else
#	define PROFILE_SCOPE(name)
#	define PROFILE_FUNCTION()
#	define PROFILE_FRAMEMARKER()
#	define PROFILE_LOCK(type, var, name) type var
#	define PROFILE_LOCKMARKER(var)
#	define PROFILE_SETTHREADNAME(name)
#endif