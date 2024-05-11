#include "RL_Play.h"


RL_Play::RL_Play()
{
	srand(time(0));
	window.create(sf::VideoMode(2 * WINDOW_WIDTH, WINDOW_HEIGHT), "Tetris", sf::Style::Default);
	first = true;
	for (int i = 0; i < 4; ++i) {
		torch::Tensor empty_frame = torch::zeros({ 84,84, 1 }, torch::kFloat);
		empty_frame = empty_frame.to(torch::kCUDA);
		framestack.push_back(empty_frame);
	}

}

torch::Tensor RL_Play::restart()
{

	isGameOver = false;
	hit = false;
	landed = false;

	elapsedSeconds = 0.0f;
	elapsedSecondsKey = 0.0f;
	elapsedSecondsLock = 0.0f;

	prevscore = 0;
	prevheight = 0;
	prevholes = 1;
	prevGapCount = 0;
	prevBumpiness = 0;
	prevAggregateHeight = 0;
	prevNextCount = 0;


	MOVE_INTERVAL = 1.0f;
	lockTime = 0.1f;

	tile.resetMap();
	tile.resetHoles();
	game.restartScore();

	game.clearQueue();
	game.refillTetrominoQueue();
	nextTetromino = game.tetrominoQueue.front();
	game.tetrominoQueue.pop();
	tetromino.reset(new Figures(Tetrominos[nextTetromino]));
	nextTetromino = game.tetrominoQueue.front();
	game.tetrominoQueue.pop();

	colorMap = tile.getColorMap();

	sf::Image windowImg = render();
	torch::Tensor observation = getObservation(windowImg, 'y', false);
	window.display();

	return observation;
}

std::tuple<torch::Tensor, float, bool, bool> RL_Play::step(int event, bool skipped)
{
	colorMap = tile.getColorMap();
	bool hardDrop = false;
	bool moved = false;
	tetromino->isDownPressed(false);

	float deltaTime = moveClock.restart().asSeconds();
	elapsedSeconds += deltaTime;
	elapsedSecondsKey += deltaTime;

	switch (event)
	{
	case(0):
		tetromino->moveLeft(colorMap);
		break;
	case(1):
		tetromino->moveRight(colorMap);
		break;
	case(2):
		tetromino->moveDown(colorMap);
		break;
	case(3):
		tetromino->rotate(colorMap, sf::Keyboard::C);
		break;
	case(4):
		tetromino->rotate(colorMap, sf::Keyboard::X);
		break;
	case(5):
		tetromino->swapTetrominos(tetromino, game.tetrominoQueue);
		break;
	case(6):
		moved = true;
		while (moved) {
			moved = tetromino->moveDown(colorMap);
		}
		hardDrop = true;
		break;
	default:
		break;
	}


	if (hardDrop || !tetromino->checkCollisions(colorMap)) {

		hit = true;
		elapsedSecondsLock += deltaTime;

		if (elapsedSecondsLock >= lockTime)
		{

			std::vector<std::vector<int>> shape = tetromino->getShape();
			int posX = tetromino->getPosX();
			int posY = tetromino->getPosY();
			color = tetromino->getColor();

			landed = true;

			tile.updateMap(shape, posX, posY, color);
			tetromino.reset(new Figures(Tetrominos[nextTetromino]));
			tetromino->didCtrlClick = false;


			//MOVE_INTERVAL = tile.getSpeed();

			nextTetromino = game.tetrominoQueue.front();
			game.tetrominoQueue.pop();

			if (game.tetrominoQueue.empty()) {
				game.refillTetrominoQueue();
			}

			elapsedSecondsLock -= lockTime;

		}
	}
	else
	{
		hit = false;
		elapsedSecondsLock = 0.0f;
	}
	if (elapsedSeconds >= MOVE_INTERVAL) {
		tetromino->moveDown(tile.getColorMap());
		elapsedSeconds -= MOVE_INTERVAL;
	}

	sf::Image windowImg = render();
	window.display();
	std::tuple<float, bool> eval = judge(event);
	torch::Tensor observation = getObservation(windowImg, 'n', skipped);


	return std::make_tuple(observation, std::get<0>(eval), std::get<1>(eval), scored);

}


void RL_Play::play()
{
	restart();
	for (; ; )
	{

		colorMap = tile.getColorMap();
		bool hardDrop = false;
		bool moved = false;
		tetromino->isDownPressed(false);

		float deltaTime = moveClock.restart().asSeconds();
		elapsedSeconds += deltaTime;
		elapsedSecondsKey += deltaTime;

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed)
			{
				switch (event.key.code)
				{
				case(sf::Keyboard::C):
					tetromino->rotate(colorMap, event.key.code);
					break;
				case(sf::Keyboard::X):
					tetromino->rotate(colorMap, event.key.code);
					break;
				case(sf::Keyboard::LControl):
					tetromino->swapTetrominos(tetromino, game.tetrominoQueue);
					break;
				case(sf::Keyboard::Space):
					moved = true;
					while (moved) {
						moved = tetromino->moveDown(colorMap);
					}
					hardDrop = true;
					break;
				}
			}
		}

		if (elapsedSecondsKey >= 1.0f / 15.0f)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				tetromino->moveDown(colorMap);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				tetromino->moveRight(colorMap);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				tetromino->moveLeft(colorMap);
			}
			elapsedSecondsKey -= 1.0f / 15.0f;
		}

		if (hardDrop || !tetromino->checkCollisions(colorMap)) {

			hit = true;
			elapsedSecondsLock += deltaTime;

			if (elapsedSecondsLock >= lockTime)
			{

				std::vector<std::vector<int>> shape = tetromino->getShape();
				int posX = tetromino->getPosX();
				int posY = tetromino->getPosY();
				color = tetromino->getColor();

				tile.updateMap(shape, posX, posY, color);
				tetromino.reset(new Figures(Tetrominos[nextTetromino]));
				tetromino->didCtrlClick = false;

				MOVE_INTERVAL = tile.getSpeed();

				nextTetromino = game.tetrominoQueue.front();
				game.tetrominoQueue.pop();

				if (game.tetrominoQueue.empty()) {
					game.refillTetrominoQueue();
				}

				elapsedSecondsLock -= lockTime;

			}
		}
		else
		{
			hit = false;
			elapsedSecondsLock = 0.0f;
		}
		if (elapsedSeconds >= MOVE_INTERVAL) {
			tetromino->moveDown(tile.getColorMap());
			elapsedSeconds -= MOVE_INTERVAL;
		}

		render();
		if (isGameOver) { restart(); }
		window.display();
	}
}

sf::Image RL_Play::render()
{
	window.clear();
	tile.drawTiles(window);
	tetromino->drawNext(window, nextTetromino);
	tetromino->drawInventory(window);
	game.DisplayScore(window);
	if (!tetromino->drawFigures(window, tile.getColorMap())) { isGameOver = true; }
	return window.capture();

}


torch::Tensor RL_Play::getObservation(sf::Image windowImg, char reset, bool skipped)
{

	cv::Mat img = cv::Mat(windowImg.getSize().y, windowImg.getSize().x, CV_8UC4, const_cast<sf::Uint8*>(windowImg.getPixelsPtr()));

	cv::cvtColor(img, img, cv::COLOR_RGBA2GRAY);

	cv::resize(img, img, cv::Size(84, 84), cv::INTER_AREA);

	torch::Tensor observation = torch::from_blob(img.data, { img.rows, img.cols, 1 }, at::kByte);

	if (!isGameOver && reset == 'n')
	{
		observation = prevObs;
	}

	prevObs = observation;

	if (skipped) { return torch::zeros({ 0 }); }

	observation = observation.toType(torch::kFloat32);
	observation = observation.to(torch::kCUDA);
	observation = observation / 255.f;

	framestack.pop_front();

	framestack.push_back(observation);

	if (first && reset == 'y') { for (int i = 0; i < 4; ++i) { framestack[i] = observation; } first = false; }

	std::vector<torch::Tensor> frame_tensors(framestack.begin(), framestack.end());

	torch::Tensor frame_stack = torch::cat(frame_tensors, 2);

	frame_stack = frame_stack.permute({ 2, 0, 1 }).contiguous();
	frame_stack = frame_stack.unsqueeze(0);

	return frame_stack;
}

std::tuple<float, bool> RL_Play::judge(const int& action)
{
	reward = 0.0f;
	done = false;
	scored = false;
	int score = game.getScore();
	int holes = tile.getHoles();
	int gapCount = tile.getGapCount();
	int bumpiness = tile.getBumpiness();
	int height = std::abs(tile.getHeight() - 19);
	int aggregateHeight = tile.getAggregateHeight();
	int nextCount = tile.getNextCount();

	if (isGameOver) { reward = -1.f;  done = true; return std::make_tuple(reward, done); }

	if (game.getScore() == 2000) { reward = 1000.f; done = true; return std::make_tuple(reward, done); }

	if (gapCount < prevGapCount) { reward += 0.60; }
	if (nextCount < prevNextCount && holes <= prevholes && gapCount <= prevGapCount) { reward += 0.25; }
	if (bumpiness > prevBumpiness) { reward -= 0.3; }
	if (height > 4 && height > prevheight) { reward -= 0.1; }
	if (holes > prevholes) { reward -= 0.6; }

	if (score > prevscore) { reward = std::pow(score - prevscore, 2) * 100; scored = true; }

	prevscore = score;
	prevholes = holes;
	prevGapCount = gapCount;
	prevheight = height;
	prevBumpiness = bumpiness;
	prevAggregateHeight = aggregateHeight;
	prevNextCount = nextCount;

	return std::make_tuple(reward, done);

}