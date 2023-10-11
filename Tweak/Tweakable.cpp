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

#include "Tweakable.h"

namespace maple
{
	static std::vector<std::vector<TweakValue*>>* values = nullptr;
	
	auto add(TweakValue* tweak) -> void
	{
		if (values == nullptr) 
		{
			values = new std::vector<std::vector<TweakValue*>>();
		}

		if (values->size() == 0)
		{
			values->resize((uint32_t)TweakType::LENGTH);
		}
		(*values)[(uint32_t)tweak->getType()].push_back(tweak);
	};

	auto getValues() ->std::vector<std::vector<TweakValue*>>&
	{
		return *values;
	}
}
