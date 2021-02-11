#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <functional>

namespace Blocks {

	struct Block
	{
		Block(const std::array<sf::Vector2f, 4>& _SpritesPosition, const sf::Vector2f& _RotationCenter, sf::Texture& texture);
		Block() = default;

		void draw(sf::RenderWindow& w)const;
		void move(const sf::Vector2f& amount);
		void rotate();
	private:
		std::array<sf::Vector2f, 4> getCoordsAfterRotation()const;

		std::array<sf::Sprite, 4> m_Sprites;
		sf::Vector2f m_RotationCenter;
		friend class BlockMap;
	};

	class BlockGenerator {
	public:
		BlockGenerator();
		Block getRandomBlock()const;
	private:
		std::function<int()> m_GetRandomBlockIndex;
		//recources
		std::vector<Block> m_BlockTypes;
		std::vector<sf::Texture> m_TextureTypes;
		std::default_random_engine m_Generator;
		std::uniform_int_distribution<int> m_UniformDistribution;
	};

	class BlockMap {
	public:
		BlockMap(float moveDownTime);
		void draw(sf::RenderWindow& w)const;
		//check the state of current block
		bool isGameOver()const;
		bool isBlockMovedNormally()const;
		bool isBlockCollided()const;

		sf::String getScore();

		bool checkIfBlockCanBePlaced(Block& blck)const;
		//move
		void moveBlockDown(Block& blck, float dltTime);
		void moveBlockLeft(Block& blck)const;
		void moveBlockRight(Block& blck)const;
		void rotateBlock(Block& blck);
		void resetBoard();
	private:
		enum class MoveCode
		{
			GameOver = -1,
			BlockMovedNormally = 0,
			BlockCollided = 1
		};
		void addBlockToMap(Block& blck);
		bool blockCollidedAtZeroY(Block& blck)const;
		int destroyFullRows();
		void swapRowsUpwards(int currentPosition);
		//block move
		MoveCode m_StateOfCurrentBlock;
		float m_MoveDownTime;
		float m_CurrentTime;
		//score
		unsigned m_Score;
		unsigned m_PreviousScore;
		sf::String m_ChachedScore;

		const sf::Vector2f m_InvalidPosition;
		//sprites map
		std::vector<std::vector<sf::Sprite>> m_SpriteMatrix;
		//recources
		sf::Texture m_WallsTexture;
		std::vector<sf::Sprite> m_WallsSprites;
	};
}
