// RayCaster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SFML/Graphics.hpp>

#include "Map.h"
#include "Player.h"

const float kTickTime = 0.008f;

const uint32_t kWindowWidth = 320 * 2;
const uint32_t kWindowHeight = 200 * 2;

int main()
{
	Player player;
	
	Map map(&player);
	map.LoadFromFile("e1m1.txt");

	player.SetPosition(map.GetStartPosition());

	sf::RenderWindow window(sf::VideoMode(kWindowWidth, kWindowHeight), "Raycaster");

	sf::Font font;
	font.loadFromFile("arial.ttf");

	sf::Text fpsText;
	fpsText.setFont(font);
	fpsText.setCharacterSize(24);
	fpsText.setFillColor(sf::Color::Yellow);
	fpsText.setString("Hello world!");

	sf::Clock mainClock;
	float lastFrameTime = mainClock.getElapsedTime().asSeconds();
	float tickDeltaTime = lastFrameTime;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		
		const float currTime = mainClock.getElapsedTime().asSeconds();
		tickDeltaTime += currTime - lastFrameTime;
		while(tickDeltaTime > kTickTime)
		{
			player.TickUpdate();

			tickDeltaTime -= kTickTime;
		}
		
		// Render a frame
		window.clear();
		map.Render(window);
		
		// Calculate FPS		
		float fps = 1.f / (currTime - lastFrameTime);
		

		//char n[5];
		//snprintf(n, 5, "%02.0f", (currTime - lastFrameTime) * 10000);

		fpsText.setString("frame: " + std::to_string((currTime - lastFrameTime) * 10000) + " us");
		window.draw(fpsText);

		window.display();

		lastFrameTime = currTime;
	}

    return 0;
}

