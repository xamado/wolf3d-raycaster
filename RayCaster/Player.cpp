#include "stdafx.h"
#include "Player.h"

#include <SFML/Window/Keyboard.hpp>
#include "Math.h"

const float Player::kRotateSpeed = 1.0f;
const float Player::kMoveSpeed = 2.5f;

Player::Player() 
{
	_angle = 80;
	_position = sf::Vector2f(256, 256);
}


Player::~Player()
{
}

void Player::TickUpdate()
{
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		_angle += kRotateSpeed;
	}
	else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		_angle -= kRotateSpeed;
	}

	sf::Vector2f dir;
	dir.x = std::cos(_angle * Math::kDegToRad);
	dir.y = -std::sin(_angle * Math::kDegToRad);

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		_position += dir * kMoveSpeed;
	}
	else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		_position -= dir * kMoveSpeed;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		_position.x -= kMoveSpeed;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		_position.x += kMoveSpeed;
	}

	while (_angle < 0)
		_angle += 360;
	while (_angle > 360)
		_angle -= 360;
}
