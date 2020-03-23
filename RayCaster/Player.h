#pragma once
#include <SFML/System/Vector2.hpp>

class Player
{
public:
	Player();
	~Player();

	void TickUpdate();

	sf::Vector2f GetPosition() const { return _position; }
	void SetPosition(const sf::Vector2f& pos) { _position = pos; }

	float GetRotation() const { return _angle; }

private:
	static const float kRotateSpeed;
	static const float kMoveSpeed;

	float _angle = 45.0f;
	sf::Vector2f _position;
};
