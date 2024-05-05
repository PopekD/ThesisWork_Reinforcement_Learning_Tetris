#pragma once
#include <SFML/Graphics.hpp>
#include "Figures.h"
#include <iostream>
#include <cstdio>
#include <torch/torch.h>
#include <opencv2/opencv.hpp>


class RL_Play
{
private:
	torch::Tensor getObservation(sf::Image windowImg, char reset, bool skipped);
	Tile tile;
	Game game;
	int nextTetrmino;
	bool isGameOver;
	bool done;
	int nextTetromino;
	sf::Clock moveClock;
	sf::RenderWindow window;
	torch::Tensor prevObs;
	cv::Mat prevImg;

	float elapsedSeconds;
	float elapsedSecondsKey;
	float elapsedSecondsLock;
	float lockTime;
	bool hit;
	bool scored;


	int prevscore;
	int prevheight;
	int prevholes;
	int prevGapCount;
	int prevBumpiness;
	int prevAggregateHeight;
	int prevNextCount;

	bool landed;
	int action;
	int frame = 0;
	std::deque<torch::Tensor> framestack;
	int posY;
	float reward;
	float MOVE_INTERVAL;
	std::vector<std::vector<sf::Color>> colorMap;
	sf::Color color;
	std::unique_ptr<Figures> tetromino;

public:
	RL_Play();
	void play();
	torch::Tensor restart();
	std::tuple<torch::Tensor, float, bool, bool> step(int event, bool skipped);
	sf::Image render();
	std::tuple<float, bool> judge(const int& action);
};