#include "Game.h"
#include <iostream>

int Game::score = 0;

Game::Game() 
{
	if (!font.loadFromFile("bin/x420/NoWW/Tetris.ttf")) {
		std::cout << "Failed";
	}

	scoreText.setFont(font);
	scoreText.setCharacterSize(WINDOW_HEIGHT / 14);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setPosition(WINDOW_WIDTH + (WINDOW_WIDTH / 4), WINDOW_HEIGHT / 1.25);

}
void Game::DisplayScore(sf::RenderWindow& window)
{
	scoreText.setString("Score: " + std::to_string(score));
	window.draw(scoreText);
}
void Game::setScore(int numClearedRows)
{
	score+= numClearedRows;
	return;
}
void Game::restartScore()
{
	score = 0;
}


void Game::refillTetrominoQueue()
{

	tetrominoQueue.push(1);
	tetrominoQueue.push(5);
	tetrominoQueue.push(9);
	tetrominoQueue.push(13);
	tetrominoQueue.push(17);
	tetrominoQueue.push(21);
	tetrominoQueue.push(25);



	std::vector<int> tetrominoVector;
	while (!tetrominoQueue.empty()) {
		tetrominoVector.push_back(tetrominoQueue.front());
		tetrominoQueue.pop();
	}
	std::shuffle(tetrominoVector.begin(), tetrominoVector.end(), std::default_random_engine(std::random_device()()));
	for (const auto& tetromino : tetrominoVector) {
		tetrominoQueue.push(tetromino);
	}

}

void Game::clearQueue()
{
	while (!tetrominoQueue.empty()) {
		tetrominoQueue.pop();
	}
}

int Game::getScore()
{
	return score;
}