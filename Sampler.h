//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Definitions.h"

namespace maple
{
	class Sampler
	{
	public:
		using Ptr = std::shared_ptr<Sampler>;

		Sampler(TextureFilter filter, TextureWrap wrapU, TextureWrap wrapV,  float maxAnisotropy)
		    : filter(filter), wrapV(wrapV), wrapU(wrapU), wrapW(wrapW), maxAnisotropy(maxAnisotropy)
		{
		}

		static auto create(TextureFilter filter, TextureWrap wrapU, TextureWrap wrapV, float maxAnisotropy) -> Ptr;

		inline auto getFilter() const { return filter; }
		inline auto getWrapU() const { return wrapU; }
		inline auto getWrapV() const { return wrapV; }
		inline auto getWrapW() const { return wrapV; }

	private:
		TextureFilter filter;
		TextureWrap wrapU;
		TextureWrap wrapV;
		TextureWrap wrapW;
		float maxAnisotropy;
	};
} // namespace maple