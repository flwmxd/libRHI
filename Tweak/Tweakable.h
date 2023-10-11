//////////////////////////////////////////////////////////////////////////////
// This file is part of the PharaohStroy MMORPG client                      //
// Copyright ?2020-2022 Prime Zeng                                          //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

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
	};

	extern auto add(TweakValue* tweak) -> void;
	extern auto getValues() ->std::vector<std::vector<TweakValue*>>&;

	

#define TWEEKCLASS(name, type)																\
	class name : public TweakValue                                                          \
	{                                                                                       \
	public:                                                                                 \
		const char* varName;				                                                \
		type var, lowerBound, upperBound, step;												\
																							\
		name(const char* strName, const type& var = type{} ,const type& nLower = type{}, const type& nUpper= type{}, const type& step = type{})						\
		: varName(strName),	lowerBound(nLower), upperBound(nUpper), step(step), var(var)	\
																							\
		{																					\
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

