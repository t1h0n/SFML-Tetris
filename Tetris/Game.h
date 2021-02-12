#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Block.h"

class Game
{
private:
	std::unique_ptr<sf::RenderWindow> m_Window;
	sf::Event event;
	sf::VideoMode videoMode;
	//game  vars
	bool m_SpaceKeyIsReleased;
	bool m_BlockArrows;
	Blocks::Block m_CurrentBlock;
	Blocks::Block m_NextBlock;
	Blocks::BlockGenerator m_BlockGenerator;
	Blocks::BlockMap m_BlockMap;
	//recources
	sf::Text m_Score;
	sf::Font m_Font;

	//game logic
	void handleBlockMovement();
	void initBlocks();
	void initWindow();
	void initText();
	void processKeys();
	void onUpdate(float dlt);
	void onRender();
public:
	Game();
	void run();
};

