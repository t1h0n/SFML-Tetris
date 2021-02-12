#include "Block.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Blocks {
	static constexpr int BlockCountX = 12;
	static constexpr int BlockCountY = 12;
	static constexpr float blockSize = 64.f;
	static constexpr float wallSize = 16.f;

	Block::Block(const std::array<sf::Vector2f, 4>& _SpritesPosition, const sf::Vector2f& _RotationCenter, sf::Texture& texture)
		: m_RotationCenter(_RotationCenter* blockSize)
	{
		for (int i{ 0 }; i < 4; ++i) {
			m_Sprites[i].setTexture(texture);
			m_Sprites[i].setPosition(_SpritesPosition[i] * blockSize);
		}
	}

	void Block::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		for (const auto& sprite : m_Sprites)
			target.draw(sprite, states);
	}

	void Block::move(const sf::Vector2f& amount)
	{
		for (auto& sprite : m_Sprites)
			sprite.move(amount);
		m_RotationCenter += amount;
	}

	void Block::rotate()
	{
		for (auto& sprite : m_Sprites) {
			auto rotVector = sprite.getPosition() - m_RotationCenter;
			auto newPos = sf::Vector2f{ rotVector.y * (-1.f), rotVector.x } + m_RotationCenter;//rotation matrix for pi/2
			sprite.setPosition(newPos);
		}
	}

	std::array<sf::Vector2f, 4> Block::getCoordsAfterRotation()const
	{
		std::array<sf::Vector2f, 4> tmp;
		for (int i = 0; i < 4; ++i) {
			auto rotVector = m_Sprites[i].getPosition() - m_RotationCenter;
			tmp[i] = sf::Vector2f{ rotVector.y * (-1.f), rotVector.x } + m_RotationCenter;//rotation matrix for pi/2
		}
		return tmp;
	}


	bool BlockMap::isGameOver() const
	{
		return m_StateOfCurrentBlock == MoveCode::GameOver;
	}

	bool BlockMap::isBlockMovedNormally() const
	{
		return m_StateOfCurrentBlock == MoveCode::BlockMovedNormally;
	}

	bool BlockMap::isBlockCollided() const
	{
		return m_StateOfCurrentBlock == MoveCode::BlockCollided;
	}

	BlockMap::BlockMap(float moveDownTime)
		:m_MoveDownTime(moveDownTime), m_CurrentTime(0), m_Score(0), m_PreviousScore(1), m_InvalidPosition( -10000.f, -10000.f )
	{
		//generate space for dynammic containers //TODO: rewrite with pregenerated walls
		m_WallsSprites.resize(183);//9+9+18
		m_SpriteMatrix.resize(BlockCountY);
		for (auto& row : m_SpriteMatrix)
			row.resize(BlockCountX);
		//set not visible coords
		resetBoard();

		//load wall textures
		if (!m_WallsTexture.loadFromFile("Recources/block_textures.png", sf::IntRect(352, 248, 16, 16)))
			static_assert(1, "failed to load textures");
		//generate left&right walls
		for (int i{ 0 }; i < 48; ++i) {
			m_WallsSprites[i].setPosition({ 0.f, static_cast<float>(i) * wallSize });
			m_WallsSprites[i + 48].setPosition({ 784.f, static_cast<float>(i) * wallSize });
		}
		//generate bottom wall
		for (int i{ 0 }; i < 50; ++i) {
			m_WallsSprites[i + 96].setPosition({ static_cast<float>(i) * wallSize, 768.f });
		}
		//generate place for showing next block vertical(9x9) horizontal(0x18)
		//bottom horizontal
		for (int i{ 0 }; i < 19; ++i) {
			m_WallsSprites[i + 146].setPosition({ static_cast<float>(i) * wallSize, 928.f });
		}
		//vertical
		for (int i{ 0 }; i < 9; ++i) {
			m_WallsSprites[i + 165].setPosition({ 0.f, static_cast<float>(i + 49) * wallSize });
			m_WallsSprites[i + 174].setPosition({ 288.f, static_cast<float>(i + 49) * wallSize });
		}
		//assign textures
		for (auto& wall : m_WallsSprites)
			wall.setTexture(m_WallsTexture);
	}

	void BlockMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		for (const auto& sprite_vec : m_SpriteMatrix)
			for (const auto& sprite : sprite_vec)
				target.draw(sprite, states);
		for (const auto& wall : m_WallsSprites)
			target.draw(wall, states);
	}

	void BlockMap::addBlockToMap(Block& blck)
	{
		for (auto& sprite : blck.m_Sprites) {
			m_SpriteMatrix[static_cast<size_t>(sprite.getPosition().y / blockSize)][static_cast<size_t>((sprite.getPosition().x - wallSize) / blockSize)] = std::move(sprite);
		}
	}

	bool BlockMap::checkIfBlockCanBePlaced(Block& blck)const
	{
		for (const auto& sprite : blck.m_Sprites) {
			size_t x = static_cast<size_t>((sprite.getPosition().x - wallSize) / blockSize), y = static_cast<size_t>((sprite.getPosition().y) / blockSize);
			if (m_SpriteMatrix[y][x].getPosition() != m_InvalidPosition)
				return false;
		}
		return true;
	}

	void BlockMap::moveBlockDown(Block& blck, float dltTime)//-1 game over; 0 block is moved dowm normally; 1 block has collided with another block
	{
		m_CurrentTime += dltTime;
		if (m_CurrentTime >= m_MoveDownTime)
		{
			for (const auto& sprite : blck.m_Sprites) {
				//map blocks position to a grid
				auto newPos = sprite.getPosition() + sf::Vector2f{ 0, blockSize };
				int x = static_cast<int>((newPos.x - wallSize) / blockSize), y = static_cast<int>((newPos.y) / blockSize);
				if (y == BlockCountY || m_SpriteMatrix[y][x].getPosition() != m_InvalidPosition) {
					//add block to map and reset godown time
					addBlockToMap(blck);
					m_CurrentTime = 0;
					//increase score depending on nr of rows collided
					m_Score += destroyFullRows();
					//checking if the game is over
					if (blockCollidedAtZeroY(blck))
						m_StateOfCurrentBlock = MoveCode::GameOver;
					else
						m_StateOfCurrentBlock = MoveCode::BlockCollided;
					return;
				}
			}
			blck.move({ 0, blockSize });
			m_CurrentTime = 0;
		}
		m_StateOfCurrentBlock = MoveCode::BlockMovedNormally;
	}

	void BlockMap::moveBlockLeft(Block& blck)const
	{
		for (const auto& sprite : blck.m_Sprites)
		{
			auto newPos = sprite.getPosition() + sf::Vector2f{ -blockSize, 0.f };
			int x = static_cast<int>((newPos.x - wallSize) / blockSize), y = static_cast<int>((newPos.y) / blockSize);
			if (x < 0 || m_SpriteMatrix[y][x].getPosition() != m_InvalidPosition)
				return;
		}
		blck.move({ -blockSize, 0.f });
	}

	void BlockMap::moveBlockRight(Block& blck)const
	{
		for (const auto& sprite : blck.m_Sprites)
		{
			auto newPos = sprite.getPosition() + sf::Vector2f{ blockSize, 0.f };
			int x = static_cast<int>((newPos.x - wallSize) / blockSize), y = static_cast<int>((newPos.y) / blockSize);
			if (x >= BlockCountX || m_SpriteMatrix[y][x].getPosition() != m_InvalidPosition)
				return;
		}
		blck.move({ blockSize, 0.f });
	}

	void BlockMap::rotateBlock(Block& blck)
	{
		auto newPositionOfBlock = blck.getCoordsAfterRotation();
		for (const auto& newPos : newPositionOfBlock)
		{
			int x = static_cast<int>((newPos.x - wallSize) / blockSize), y = static_cast<int>((newPos.y) / blockSize);
			if (x >= BlockCountX || y >= BlockCountY || x < 0 || y < 0 || m_SpriteMatrix[y][x].getPosition() != m_InvalidPosition)
				return;
		}
		blck.rotate();
	}

	bool BlockMap::blockCollidedAtZeroY(Block& blck)const
	{
		for (const auto& sprite : blck.m_Sprites)
			if (sprite.getPosition().y == 0)
				return true;
		return false;
	}

	void BlockMap::resetBoard()
	{
		for (auto& row : m_SpriteMatrix)
			for (auto& sprite : row)
				sprite.setPosition(m_InvalidPosition);
		m_Score = 0;
	}

	sf::String BlockMap::getScore()
	{
		if (m_PreviousScore != m_Score) {
			m_PreviousScore = m_Score;
			std::stringstream ss;
			ss << std::setw(10) << std::setfill('0') << m_Score;
			m_ChachedScore = ss.str().c_str();
		}
		return m_ChachedScore;
	}

	int BlockMap::destroyFullRows()
	{
		unsigned numberOfFullRows{ 0 };
		for (size_t i{ 0 }; i < m_SpriteMatrix.size(); ++i)
		{
			bool rowIsFull{ true };
			for (auto sprite = m_SpriteMatrix[i].begin(); sprite != m_SpriteMatrix[i].end(); ++sprite) {
				if (sprite->getPosition() == m_InvalidPosition)
				{
					rowIsFull = false;
					break;
				}
			}
			if (rowIsFull) {
				numberOfFullRows += 1;
				for (auto& sprite : m_SpriteMatrix[i])
					sprite.setPosition(m_InvalidPosition);
				swapRowsUpwards(i);
			}
		}
		return numberOfFullRows * numberOfFullRows * 100;
	}

	void BlockMap::swapRowsUpwards(int currentPosition)
	{
		int nextPosition{ currentPosition - 1 };
		while (nextPosition > 0)
		{
			for (auto sprite = m_SpriteMatrix[nextPosition].begin(); sprite != m_SpriteMatrix[nextPosition].end(); ++sprite) {
				if (sprite->getPosition() != m_InvalidPosition) {
					sprite->setPosition(sprite->getPosition() + sf::Vector2f{ 0.f, blockSize });
				}
			}
			std::swap(m_SpriteMatrix[currentPosition], m_SpriteMatrix[nextPosition]);
			nextPosition--;
			currentPosition--;
		}
	}
	BlockGenerator::BlockGenerator() :
		m_UniformDistribution{ 0, 6 }
	{
		m_BlockTypes.reserve(7);
		m_TextureTypes.resize(3);
		//seeding m_Generator
		m_Generator.seed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
		//binding generator to distribution
		m_GetRandomBlockIndex = std::bind(std::ref(m_UniformDistribution), std::ref(m_Generator));
		//loading block textures
		for (size_t i{ 0 }; i < 3; ++i)
			if (!m_TextureTypes[i].loadFromFile("Recources/block_textures.png", sf::IntRect(80, 8 + static_cast<int>(blockSize + 8) * i, 64, 64)))
				static_assert(1, "failed to load textures");
		//creating block types
		m_BlockTypes.push_back(Block({ { {0.0f, 0.0f}, {1.0f, 0.0f}, {2.0f, 0.0f}, {3.0f, 0.0f}  } }, { 1.0f, 0.0f }, m_TextureTypes[0]));//I
		m_BlockTypes.push_back(Block({ { {0.0f, 1.0f}, {1.0f, 1.0f}, {2.0f, 1.0f}, {2.0f, 0.0f}  } }, { 1.0f, 1.0f }, m_TextureTypes[1]));//L
		m_BlockTypes.push_back(Block({ { {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {2.0f, 0.0f}  } }, { 1.0f, 1.0f }, m_TextureTypes[1]));//S
		m_BlockTypes.push_back(Block({ { {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {2.0f, 1.0f}  } }, { 1.0f, 1.0f }, m_TextureTypes[0]));//J
		m_BlockTypes.push_back(Block({ { {0.0f, 1.0f}, {1.0f, 1.0f}, {2.0f, 1.0f}, {1.0f, 0.0f}  } }, { 1.0f, 1.0f }, m_TextureTypes[2]));//T
		m_BlockTypes.push_back(Block({ { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {2.0f, 1.0f}  } }, { 1.0f, 1.0f }, m_TextureTypes[2]));//Z
		m_BlockTypes.push_back(Block({ { {1.0f, 0.0f}, {2.0f, 0.0f}, {1.0f, 1.0f}, {2.0f, 1.0f}  } }, { 1.0f, 1.0f }, m_TextureTypes[1]));//O
	}

	Block BlockGenerator::getRandomBlock()const
	{
		return m_BlockTypes[m_GetRandomBlockIndex()];
	}
}