#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <unordered_map>
#include "Math.h"

class Player;

#define TILE_PLAYER 0x04AC04FF

const uint32_t kFOV = 60;
const uint32_t kWallHeight = 64;

const int32_t kProjectionWidth = 320;
const int32_t kProjectionHeight = 200;
const int32_t kHalfProjectionWidth = kProjectionWidth / 2;
const int32_t kHalfProjectionHeight = kProjectionHeight / 2;

const int32_t kDistanceToProjectionPlane = (uint32_t)(kProjectionWidth * 0.5f / std::tan(kFOV / 2 * Math::kDegToRad));

const float kProjectionScale = (float)kWallHeight / (float)kDistanceToProjectionPlane;

class Sprite
{
public:
	uint8_t texture;
	sf::Vector2f position;
};

class Map
{
public:
	Map(const Player* player);
	~Map();
	bool LoadFromFile(const std::string& filename);
	void Init();

	
	void Render(sf::RenderWindow& window);

	void DrawWalls();
	void DrawSprites();

	sf::Vector2f GetStartPosition() const;

private:
	uint32_t _width;
	uint32_t _height;

	uint8_t* _grid;
	
	std::vector<Sprite*> _sprites;

	uint8_t* _pixels;
	float _depth[kProjectionWidth];
	
	sf::Texture _texture;
	sf::Sprite _sprite;

	const Player* _player;
	
	sf::Image* _nullTexture;
	std::vector<sf::Image*> _wallTextures;
	std::vector<sf::Image*> _spriteTextures;
	
	sf::Vector2i _playerStart;

	std::unordered_map<uint32_t, uint8_t> _mapLoaderTable = {
		{ 0x545454FF, 1 }, // grey brick 1
		{ 0x464646FF, 2 }, // grey brick 2
		{ 0xD71919FF, 3 }, // grey brick + flag
		{ 0x7F00BAFF, 4 }, // grey brick + hitler
		{ 0x00989EFF, 6 }, // grey brick + eagle
		{ 0x010159FF, 5 }, // blue brick cell
		{ 0x000035FF, 7 }, // blue brick cell skeleton
		{ 0x0404ACFF, 8 }, // blue brick 1
		{ 0x000099FF, 9 }, // blue brick 2
		{ 0x01CED5FF, 10 }, // wood + eagle
		{ 0xAF04FFFF, 11 }, // wood + hitler
		{ 0xAC5404FF, 12 }, // wood
		{ 0xDEFF00FF, 21 }, // elevator
	};

	std::unordered_map<uint32_t, uint8_t> _mapLoaderSpriteTable = {
		{ 0x28003BFF, 26 }, // floor lamp
		{ 0x553C0FFF, 34 }, // brown plant
		{ 0xFFFFFFFF, 37 }, // light
		{ 0x025200FF, 124 }, // dead guard
	};
};
