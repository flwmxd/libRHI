//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#include "Tweakable.h"
#include "StringUtils.h"

namespace maple
{
	static std::unordered_map<std::string, std::vector<TweakValue*>> * values = nullptr;

	auto add(TweakValue* tweak) -> void
	{
		if (values == nullptr) 
		{
			values = new std::unordered_map<std::string, std::vector<TweakValue*>>();
		}

		std::vector<std::string> outs;
		StringUtils::split(tweak->varName, ":", outs);

		if (!outs.empty())
		{
			(*values)[outs[0]].emplace_back(tweak);
		}
		else 
		{
			(*values)["Untitled"].emplace_back(tweak);
		}
	};

	auto getValues() ->std::unordered_map<std::string, std::vector<TweakValue*>>&
	{
		return *values;
	}
}
