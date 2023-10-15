//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace maple
{
	enum class TweakType
	{
		TweakInt32,
		TweakBool,
		TweakFloat,
		TweakDouble,
		LENGTH
	};

	class TweakValue
	{
	public:
		inline auto getType() const {
			return varType;
		}
		TweakType varType;
		const char* varName;
	};

	extern auto add(TweakValue* tweak) -> void;
	extern auto getValues() ->std::unordered_map<std::string, std::vector<TweakValue*>>&;

	

#define TWEEKCLASS(name, type)																\
	class name : public TweakValue                                                          \
	{                                                                                       \
	public:                                                                                 \
		type var, lowerBound, upperBound, step;												\
																							\
		name(const char* strName, const type& var = type{} ,const type& nLower = type{}, const type& nUpper= type{}, const type& step = type{})						\
		: 	lowerBound(nLower), upperBound(nUpper), step(step), var(var)	\
																							\
		{																					\
			varName = strName;																\
			add(this);																		\
			varType = TweakType::name;														\
		}																					\
		inline operator auto() const { return var; }										\
	}
	
	TWEEKCLASS(TweakInt32	, int32_t);
	TWEEKCLASS(TweakBool	, bool);
	TWEEKCLASS(TweakFloat	, float);
	TWEEKCLASS(TweakDouble	, double);
}

