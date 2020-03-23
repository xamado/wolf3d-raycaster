#include "stdafx.h"

#include <algorithm>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "Map.h"
#include "Player.h"
#include "Math.h"
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/FileInputStream.hpp>

Map::Map(const Player* player)
{
	_pixels = new uint8_t[kProjectionWidth * kProjectionHeight * 4];

	_texture.create(kProjectionWidth, kProjectionHeight);
	_sprite.setTexture(_texture);

	_player = player;

	_width = 10;
	_height = 10;

	_nullTexture = new sf::Image();
	_nullTexture->loadFromFile("null.png");

	const uint32_t kNumWallTextures = 30;
	_wallTextures.resize(kNumWallTextures);
	for (uint32_t i = 0; i < kNumWallTextures; i++)
	{
		char n[11];
		snprintf(n, 11, "tile%02d.bmp", i + 1);

		sf::Image* texture = new sf::Image();
		texture->loadFromFile(n);

		if (texture->getPixelsPtr() != nullptr)
		{
			_wallTextures[i] = texture;
		}
		else
		{
			_wallTextures[i] = nullptr;
		}
	}

	const uint32_t kNumSprites = 128;
	_spriteTextures.resize(kNumSprites);
	for (uint32_t i = 0; i < kNumSprites; i++)
	{
		char n[14];
		snprintf(n, 14, "sprite%03d.bmp", i + 1);


		sf::Image* texture = new sf::Image();
		texture->loadFromFile(n);

		if (texture->getPixelsPtr() != nullptr)
		{
			_spriteTextures[i] = texture;
		}
		else
		{
			_spriteTextures[i] = nullptr;
		}
	}
}

Map::~Map()
{
	delete[] _pixels;

	delete _nullTexture;

	for (sf::Image* img : _wallTextures)
	{
		if (img != nullptr)
			delete img;
	}
}

bool Map::LoadFromFile(const std::string& filename)
{
	sf::Image mapImage;
	mapImage.loadFromFile("e1m1.png");

	_width = mapImage.getSize().x;
	_height = mapImage.getSize().y;
	_grid = new uint8_t[_width * _height];
	memset(_grid, 0, _width * _height * sizeof(uint8_t));

	for (uint32_t x = 0; x < _width; ++x)
	{
		for (uint32_t y = 0; y < _height; ++y)
		{
			sf::Color c = mapImage.getPixel(x, y);
			sf::Uint32 ci = c.toInteger();

			const auto iter = _mapLoaderTable.find(ci);
			if (iter != _mapLoaderTable.end())
			{
				_grid[y * _width + x] = _mapLoaderTable[ci];
				continue;
			}

			const auto iter2 = _mapLoaderSpriteTable.find(ci);
			if (iter2 != _mapLoaderSpriteTable.end())
			{
				//_sprites[y * _width + x] = _mapLoaderSpriteTable[ci];
				Sprite* s = new Sprite();
				s->texture = _mapLoaderSpriteTable[ci];
				s->position = sf::Vector2f(x * kWallHeight + 0.5f * kWallHeight, y * kWallHeight + 0.5f * kWallHeight);
				_sprites.push_back(s);
			}

			if (ci == TILE_PLAYER)
			{
				_playerStart = sf::Vector2i(x, y);
				continue;
			}
		}
	}

	return true;
}

void Map::Render(sf::RenderWindow& window)
{
	sf::RectangleShape ceil(sf::Vector2f((float)kProjectionWidth, (float)kProjectionHeight * 0.5f));
	ceil.setFillColor(sf::Color(56, 56, 56));
	ceil.setScale(window.getSize().x / (float)kProjectionWidth, window.getSize().y / (float)kProjectionHeight);
	ceil.setPosition(0, 0);

	sf::RectangleShape floor(sf::Vector2f((float)kProjectionWidth, (float)kProjectionHeight * 0.5f));
	floor.setFillColor(sf::Color(112, 112, 122));
	floor.setScale(window.getSize().x / (float)kProjectionWidth, window.getSize().y / (float)kProjectionHeight);
	floor.setPosition(0, window.getSize().y * 0.5f);

	window.draw(ceil);
	window.draw(floor);

	DrawWalls();

	DrawSprites();

	// Blit our buffer to the screen
	_texture.update(_pixels);
	_sprite.setScale(window.getSize().x / (float)kProjectionWidth, window.getSize().y / (float)kProjectionHeight);
	window.draw(_sprite);
}

void Map::DrawWalls()
{
	const float MAX_VALUE = 999999;

	// 
	const float viewAngle = _player->GetRotation();
	const sf::Vector2f pos = _player->GetPosition();
	float angle = viewAngle + kFOV / 2;
	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;

	// Determine the angle to rotate for each column
	const float castAngle = (float)kFOV / (float)kProjectionWidth;

	// Clear
	memset(_pixels, 0, sizeof(uint8_t) * kProjectionWidth * kProjectionHeight * 4);

	// Cast a ray for each pixel column
	uint32_t hTexOffset[kProjectionWidth];
	uint8_t wallType[kProjectionWidth];

	for (int col = 0; col < kProjectionWidth; ++col)
	{
		float Hx, Hy;
		float Vx, Vy;
		float xStep, yStep;

		float hDist = MAX_VALUE;
		float vDist = MAX_VALUE;

		uint8_t hWallType = 0;
		uint8_t vWallType = 0;

		const int32_t xStepSign = angle < 90 || angle > 270 ? 1 : -1;
		const int32_t yStepSign = angle > 0 && angle < 180 ? -1 : 1;

		const float radAngle = angle * Math::kDegToRad + 0.001f;
		const float tanAngle = std::tan(radAngle);
		const float absTanAngle = std::abs(tanAngle);

		//
		// Horizontal Intersection
		//

		if (angle > 0 && angle < 180)
		{
			// ray is facing up
			Hy = floor(pos.y / 64) * (64) - 1;
		}
		else
		{
			// ray is facing down
			Hy = floor(pos.y / 64) * (64) + 64;
		}

		Hx = pos.x + std::abs(pos.y - Hy) / absTanAngle * xStepSign;
		xStep = 64.0f / absTanAngle * xStepSign;
		yStep = 64.0f * yStepSign;

		if (angle == 0 || angle == 180)
		{
			hDist = MAX_VALUE;
		}
		else
		{
			while (true)
			{
				const uint32_t gridHx = static_cast<uint32_t>(Hx / 64);
				if (gridHx < 0 || gridHx >= _width)
					break;

				const uint32_t gridHy = static_cast<uint32_t>(Hy / 64);
				if (gridHy < 0 || gridHy >= _width)
					break;

				const uint32_t idx = gridHy * _width + gridHx;
				if (_grid[idx] != 0)
				{
					hDist = std::abs((Hx - pos.x) / std::cos(radAngle));
					hWallType = _grid[idx];
					break;
				}

				Hx += xStep;
				Hy += yStep;
			}
		}

		//
		// Vertical grid intersection
		//
		if (angle < 90 || angle > 270)
		{
			// ray is to the right	
			Vx = floor(pos.x / 64) * 64 + 64;
			Vy = pos.y - (Vx - pos.x) * tanAngle;
		}
		else
		{
			// ray is to the left
			Vx = floor(pos.x / 64) * 64;
			Vy = pos.y - (Vx - pos.x) * tanAngle;
			Vx--;
		}

		xStep = 64.0f * xStepSign;
		yStep = 64.0f * absTanAngle * yStepSign;

		if (angle == 90 || angle == 270)
		{
			vDist = MAX_VALUE;
		}
		else
		{
			while (true)
			{
				const uint32_t gridVx = static_cast<uint32_t>(Vx / 64);
				if (gridVx < 0 || gridVx >= _width)
					break;

				const uint32_t gridVy = static_cast<uint32_t>(Vy / 64);
				if (gridVy < 0 || gridVy >= _width)
					break;

				const uint32_t idx = gridVy * _width + gridVx;
				if (_grid[idx] != 0)
				{
					vDist = std::abs((pos.y - Vy) / std::sin(radAngle));
					vWallType = _grid[idx];
					break;
				}

				Vx += xStep;
				Vy += yStep;
			}
		}

		// Keep the shortest distance
		if (hDist < vDist)
		{
			_depth[col] = hDist;
			hTexOffset[col] = ((int)Hx) % 64;
			wallType[col] = hWallType - 1;
		}
		else
		{
			_depth[col] = vDist;
			hTexOffset[col] = ((int)Vy) % 64;
			wallType[col] = vWallType - 1;
		}

		// Correct for fisheye effect
		_depth[col] = _depth[col] * std::cos((angle - viewAngle) * Math::kDegToRad);

		// Continue with the next column/angle pair
		angle -= castAngle;
		if (angle < 0)
			angle += 360;
	}

	for (int col = 0; col < kProjectionWidth; ++col)
	{
		// Project the wall to the projection plane
		const uint32_t projHeight = static_cast<uint32_t>((64.0f / _depth[col]) * kDistanceToProjectionPlane);
		const int32_t startY = static_cast<uint32_t>(kHalfProjectionHeight - projHeight * 0.5f);
		const int32_t endY = static_cast<uint32_t>(kHalfProjectionHeight + projHeight * 0.5f);

		// Find the correct texture, or use the null one
		uint8_t texIndex = wallType[col];
		sf::Image* img = _nullTexture;
		if (texIndex >= 0 && texIndex < _wallTextures.size() && _wallTextures[texIndex] != nullptr)
			img = _wallTextures[texIndex];

		const sf::Uint8* texels = img->getPixelsPtr() + (63 - hTexOffset[col]) * 64 * 4;

		for (int32_t i = endY; i > startY; --i)
		{
			if (i < 0 || i >= kProjectionHeight)
				continue;

			uint32_t vTexOffset = static_cast<uint32_t>(((endY - i) / (float)projHeight) * 64);

			const sf::Uint8* texel = texels + vTexOffset * 4;
			uint8_t* pixel = _pixels + (i * kProjectionWidth * 4) + col * 4;
			*(pixel) = *texel;
			*(pixel + 1) = *(texel + 1);
			*(pixel + 2) = *(texel + 2);
			*(pixel + 3) = 255;
		}
	}
}

void Map::DrawSprites()
{
	sf::Vector2f pPos = _player->GetPosition();
	float plAngle = _player->GetRotation();

	for (Sprite* spr : _sprites)
	{
		// Translate position to viewer space
		const sf::Vector2f rPos = spr->position - pPos;
		
		// Distance to sprite
		const float dist = std::sqrt(rPos.x * rPos.x + rPos.y * rPos.y);

		// Sprite angle relative to viewing angle
		const float spriteAngle = (std::atan2(rPos.y, rPos.x) * -180 / Math::kPI) - plAngle;

		// Scale the sprite based on the distance
		const float size = kDistanceToProjectionPlane / (std::cos(spriteAngle * Math::kDegToRad) * dist);

		// X-position on screen
		const float x = kProjectionWidth - (kHalfProjectionWidth + std::tan(spriteAngle * Math::kDegToRad) * kDistanceToProjectionPlane);

		int32_t startX = static_cast<int32_t>(x - kWallHeight * size * 0.5f);
		int32_t startY = static_cast<int32_t>(kHalfProjectionHeight - kWallHeight * 0.5f * size);
		int32_t endX = static_cast<int32_t>(startX + kWallHeight * size);
		int32_t endY = static_cast<int32_t>(startY + kWallHeight * size);
		uint32_t skipX = 0;
		uint32_t skipY = 0;
		
		if (startX < 0)
		{
			skipX = -startX;
			startX = 0;
		}

		if (startY < 0)
		{
			skipY = -startY;
			startY = 0;
		}
				
		if (endX > kProjectionWidth) 
			endX = kProjectionWidth;
				
		if (endY > kProjectionHeight) 
			endY = kProjectionHeight;
		
		// Grab the sprite
		sf::Image* img = _spriteTextures[spr->texture-1];
		if (img == nullptr)
			continue;

		// Draw
		const uint8_t* imgPixels = static_cast<const uint8_t*>(img->getPixelsPtr());
		for (int i = startX; i < endX; ++i)
		{
			// if column is occluded, skip the whole loop
			if (dist > _depth[i])
				continue;

			const uint8_t* texelCol = imgPixels + static_cast<uint32_t>((skipX + i - startX) * (1 / size)) * kWallHeight * 4;
			for (int j = startY; j < endY; ++j)
			{
				const uint8_t* texel = texelCol + static_cast<uint32_t>((skipY + j - startY) * (1/size)) * 4;
				const uint8_t r = *texel++;
				const uint8_t g = *texel++;
				const uint8_t b = *texel++;

				// skip magenta
				if (r == 152 && g == 0 && b == 136)
					continue;

				uint8_t* pixel = _pixels + ((kProjectionHeight - j - 1) * kProjectionWidth + i) * 4;
				*(pixel) = r;
				*(pixel + 1) = g;
				*(pixel + 2) = b;
				*(pixel + 3) = 255;
			}
		}
	}
}

sf::Vector2f Map::GetStartPosition() const
{
	return sf::Vector2f(_playerStart.x * 64.0f + 32.0f, _playerStart.y * 64.0f + 32.0f);
}
