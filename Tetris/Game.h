#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Player.h"
#include <queue>
#include <algorithm>
#include <random>

#if PLAYER == 1
#define TILE_SIZE 40.0
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 800
#elif PLAYER == 2
#define TILE_SIZE 5.0
#define WINDOW_WIDTH 50
#define WINDOW_HEIGHT 100
#endif

class Game {
private:
	static int score;
	sf::Text scoreText;
	sf::Font font;
	float time;
	public: std::queue<int> tetrominoQueue;
public:
	Game();
	void setScore(int numClearedRows);
	int getScore();
	void DisplayScore(sf::RenderWindow& window);
	void restartScore();
	void refillTetrominoQueue();
	void clearQueue();
};