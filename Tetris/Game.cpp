#include "Game.h"
#include <memory>
#include <SFML/Graphics.hpp>

static constexpr float moveDownTime = 0.6f;

Game::Game():
	m_SpaceKeyIsReleased(true),
	m_BlockMap(moveDownTime)
{
	initText();
	initWindow();
	initBlocks();
}

Game::~Game()
{
}
void Game::initText() {
	if (!m_Font.loadFromFile("Recources/courbd.ttf"))
		static_assert(2, "failed to load font");

	m_Score.setFont(m_Font);
	m_Score.setCharacterSize(80);
	m_Score.setPosition({ 312.f , 804.f });
	m_Score.setFillColor({ 255, 255, 103 });
	m_Score.setString(m_BlockMap.getScore());
}

void Game::initBlocks()
{
	m_BlockArrows = false;
	m_CurrentBlock = m_BlockGenerator.getRandomBlock();
	m_CurrentBlock.move({ 272.f, 0.f });

	m_NextBlock = m_BlockGenerator.getRandomBlock();
	m_NextBlock.move({ 24.f , 792.f });
}

void Game::initWindow()
{
	videoMode.height = 944;
	videoMode.width = 800;
	m_Window.reset(new sf::RenderWindow(videoMode, "", sf::Style::Titlebar | sf::Style::Close));
	m_Window->setFramerateLimit(60);
}

void Game::run()
{
	sf::Clock clk;
	float msc{};
	while (m_Window->isOpen())
	{
		//handle user movement
		onUpdate();
		//handle block movement downwards
		msc = clk.restart().asSeconds();
		m_Window->setTitle(std::to_string(static_cast<int>(1 / msc)).c_str());
		m_BlockMap.moveBlockDown(m_CurrentBlock, msc);
		handleBlockMovement();
		if (m_BlockArrows)
		{
			m_BlockMap.moveBlockDown(m_CurrentBlock, msc * 5);
			handleBlockMovement();
		}
		m_Score.setString(m_BlockMap.getScore());

		//render
		onRender();
	}
}

void Game::handleBlockMovement()
{
	if (m_BlockMap.isBlockCollided()) {
		m_CurrentBlock = m_NextBlock;
		m_NextBlock = m_BlockGenerator.getRandomBlock();
		m_NextBlock.move({ 24.f , 792.f });
		m_CurrentBlock.move({ 248.f, -792.f });
		if (!m_BlockMap.checkIfBlockCanBePlaced(m_CurrentBlock))
		{
			m_BlockMap.resetBoard();
			initBlocks();
		}
		m_BlockArrows = false;
	}
	else if (m_BlockMap.isGameOver()) {
		m_BlockMap.resetBoard();
		initBlocks();
	}
}
void Game::onUpdate()
{
	while (m_Window->pollEvent(this->event)) {
		if (event.type == sf::Event::Closed)
			m_Window->close();
		else if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Left && !m_BlockArrows) {
				m_BlockMap.moveBlockLeft(m_CurrentBlock);
			}
			else if (event.key.code == sf::Keyboard::Right && !m_BlockArrows) {
				m_BlockMap.moveBlockRight(m_CurrentBlock);
			}
			else if (event.key.code == sf::Keyboard::Space && !m_BlockArrows && m_SpaceKeyIsReleased)
			{
				m_SpaceKeyIsReleased = false;
				m_BlockMap.rotateBlock(m_CurrentBlock);
			}
			else if (event.key.code == sf::Keyboard::Down) {
				m_BlockArrows = true;
			}
		}
		else if (event.type == sf::Event::KeyReleased) 
		{
			if (event.key.code == sf::Keyboard::Space)
				m_SpaceKeyIsReleased = true;
		}
	}
}

void Game::onRender()
{
	m_Window->clear();
	m_Window->draw(m_CurrentBlock);
	m_Window->draw(m_NextBlock);
	m_Window->draw(m_BlockMap);
	m_Window->draw(m_Score);
	m_Window->display();
}
