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
		TweakInt8,
		TweakUInt8,
		TweakInt16,
		TweakUInt16,
		TweakInt32,
		TweakUInt32,
		TweakInt64,
		TweakUInt64,
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

		static auto add(TweakValue* tweak)
		{
			if (values.size() == 0)
			{
				values.resize((uint32_t)TweakType::LENGTH);
			}
			values[(uint32_t)tweak->getType()].emplace_back(tweak);
		}

		TweakType varType;
		static std::vector<std::vector<TweakValue*>> values;
	};


#define TWEEKCLASS(name, type)																\
	class name : public TweakValue                                                          \
	{                                                                                       \
	public:                                                                                 \
		const char* varName;				                                                \
		type var, lowerBound, upperBound, step;												\
																							\
		name(const char* strName, const type& nLower = type{}, const type& nUpper= type{}, const type& step = type{})						\
		: varName(strName),	lowerBound(nLower), upperBound(nUpper), step(step)			\
																							\
		{																					\
			TweakValue::add(this);															\
			varType = TweakType::name;														\
		}																					\
	}
	
	TWEEKCLASS(TweakInt8	, int8_t);
	TWEEKCLASS(TweakUInt8	, uint8_t);
	TWEEKCLASS(TweakInt16	, int16_t);
	TWEEKCLASS(TweakUInt16	, uint16_t);
	TWEEKCLASS(TweakInt32	, int32_t);
	TWEEKCLASS(TweakUInt32	, uint32_t);
	TWEEKCLASS(TweakInt64	, int64_t);
	TWEEKCLASS(TweakUInt64	, uint64_t);
	TWEEKCLASS(TweakBool	, bool);
	TWEEKCLASS(TweakFloat	, float);
	TWEEKCLASS(TweakDouble	, double);
}
