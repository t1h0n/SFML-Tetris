#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <functional>

namespace Blocks {
	constexpr int BlockCountX = 12;
	constexpr int BlockCountY = 12;
	constexpr float blockSize = 64.f;
	constexpr float wallSize = 16.f;

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
		std::function<int()> m_GetRandomBlock;
		//recources
		std::vector<Block> m_BlockTypes;
		std::vector<sf::Texture> m_TextureTypes;
		std::default_random_engine m_Generator;
		std::uniform_int_distribution<int> m_UniformDistribution{ 0, 6 };
	};

	class BlockMap {
	public:
		enum class MoveCode
		{
			GameOver = -1,
			BlockMovedNormally = 0,
			BlockCollided = 1
		};
		BlockMap(float moveDownTime);
		void draw(sf::RenderWindow& w)const; //draws cells


		bool checkIfBlockCanBePlaced(Block& blck)const;
		//move
		MoveCode moveBlockDown(Block& blck, float dltTime);
		void moveBlockLeft(Block& blck)const;
		void moveBlockRight(Block& blck)const;
		void rotateBlock(Block& blck);
		//checks if rotation is possible and if it is, rotates block
		void resetBoard();
		sf::String getScore();
	private:
		void addBlockToMap(Block& blck);//should be in private
		bool blockCollidedAtZeroY(Block& blck)const;
		int destroyFullRows();//collided
		void swapRowsUpwards(size_t currentPosition);
		float m_MoveDownTime;
		float m_CurrentTime{ 0 };
		//score
		unsigned m_Score{ 0 };
		unsigned m_PreviousScore{ 1 };
		const sf::Vector2f m_InvalidPosition{ -10000.f, -10000.f };
		sf::String m_ChachedScore;
		//sprites map
		std::vector<std::vector<sf::Sprite>> m_SpriteMatrix;
		//recources
		sf::Texture m_WallsTexture;
		std::vector<sf::Sprite> m_WallsSprites;
	};
}
