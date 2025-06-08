#pragma once

#include "stb_image.h"

#include <iostream>

namespace Engine
{
	class ImageLoader
	{
	public:

		static unsigned char* LoadImg(int& width, int& height, int& channels)
		{
			unsigned char* pixelData = stbi_load("./brick.png", &width, &height, &channels, 4);
			if (!pixelData)
			{
				throw std::runtime_error("Failed to load image!");
			}

			return pixelData;
		}

	private:

	};
}