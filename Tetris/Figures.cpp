#include "Figures.h"
#include <iostream>

bool Figures::inventoryEmpty = true;
std::unique_ptr<Figures> Figures::inventoryTetromino = std::make_unique<Figures>(Tetrominos[0]);
bool Figures::didCtrlClick = false;

bool Figures::drawFigures(sf::RenderWindow& window, const std::vector<std::vector<sf::Color>>& colorMap)
{
	tileShape.setFillColor(getColor());
	tileShape.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	tileShape.setOutlineColor(sf::Color(255,255,255));
	tileShape.setOutlineThickness(1);

	std::vector<std::vector<int>> shape = getShape();

	minX = std::numeric_limits<int>::max();
	maxX = std::numeric_limits<int>::min();
	maxY = std::numeric_limits<int>::min();
	minY = std::numeric_limits<int>::max();

	for (int i = 0; i < shape.size(); i++) {
		for (int j = 0 ; j < shape[i].size(); j++) {
			if (shape[i][j] == 1)
			{
				if (colorMap[(posY / TILE_SIZE) + j][(posX / TILE_SIZE) + i] != sf::Color::Black)
				{
					return false;
				}

				tileShape.setPosition(float(posX + i * TILE_SIZE), float(posY + (j * TILE_SIZE)));
				window.draw(tileShape);

				minX = std::min(minX, int(posX + i * TILE_SIZE));
				maxX = std::max(maxX, int(posX + i * TILE_SIZE));
				maxY = std::max(maxY, int(posY + j * TILE_SIZE));
				minY = std::min(minY, int(posY + j * TILE_SIZE));
			}
		}
	}

	return true;
}
void Figures::isSpacePressed(bool KeyPressed)
{
	isSpaceKeyPressed = KeyPressed;
}
void Figures::isDownPressed(bool KeyPressed)
{
	isDownKeyPressed = KeyPressed;
}

void Figures::moveFigures() {
	if (!isDownKeyPressed)
	{
		if (maxY < bottomBoundary) { setPosY(int(posY += TILE_SIZE)); }
	}
}


bool Figures::checkCollisions(const std::vector<std::vector<sf::Color>>& colorMap)
{

	//if (maxY >= bottomBoundary) { return false; }

	for (int i = 0; i < shape.size(); i++)
	{
		for (int j = 0; j < shape[i].size(); j++)
		{
			if (shape[i][j] == 1)
			{
				int mapX = (posX / TILE_SIZE) + i;
				int mapY = (posY / TILE_SIZE) + j;

				if (mapY + 1 >= 20) { return false; }

				if ((colorMap[mapY + 1][mapX] != sf::Color::Black))
				{
					return false;
				}
			}
		}
	}

	return true;
}
bool Figures::checkCollisionsAtSpawn()
{
	if (minY <= 0) { return false; }

	int InitposX = (WINDOW_WIDTH / 2) - TILE_SIZE;
	int InitposY = 0;

	if (posX == InitposX && InitposY == posY) {
		return false;
	}

	return true;
}

void Figures::moveLeft(const std::vector<std::vector<sf::Color>>& colorMap)
{
	if (minX - TILE_SIZE < 0) { return; }

	for (int i = 0; i < shape.size(); i++)
	{
		for (int j = 0; j < shape[i].size(); j++)
		{
			if (shape[i][j] == 1)
			{
				int tileX = (posX + i * TILE_SIZE) / TILE_SIZE;
				int tileY = (posY + j * TILE_SIZE) / TILE_SIZE;

				if (tileX == 0)
				{
					return;

				}

				if (colorMap[tileY][tileX - 1] != sf::Color::Black)
				{
					return;
				}
			}
		}
	}

	setPosX(int(posX - TILE_SIZE));
}
void Figures::moveRight(const std::vector<std::vector<sf::Color>>& colorMap)
{

	if (maxX + TILE_SIZE > (WINDOW_WIDTH - TILE_SIZE)) { return; }

	for (int i = 0; i < shape.size(); i++)
	{
		for (int j = 0; j < shape[i].size(); j++)
		{
			if (shape[i][j] == 1)
			{
				int tileX = (posX + i * TILE_SIZE) / TILE_SIZE;
				int tileY = (posY + j * TILE_SIZE) / TILE_SIZE;


				if (tileX >= (WINDOW_WIDTH / TILE_SIZE) - 1)
				{
					return;
				}

				if (colorMap[tileY][tileX + 1] != sf::Color::Black)
				{
					return;
				}
			}
		}
	}
	setPosX(int(posX + TILE_SIZE));
}

bool Figures::moveDown(const std::vector<std::vector<sf::Color>>& colorMap)
{

	if (maxY == bottomBoundary) { return false; }

	for (int i = 0; i < shape.size(); i++)
	{
		for (int j = 0; j < shape[i].size(); j++)
		{
			if (shape[i][j] == 1)
			{
				int tileX = (posX + i * TILE_SIZE) / TILE_SIZE;
				int tileY = (posY + j * TILE_SIZE) / TILE_SIZE;


				if (tileY == (WINDOW_HEIGHT / TILE_SIZE) - 1)
				{
					return false;
				}

				if (colorMap[tileY + 1][tileX] != sf::Color::Black)
				{
					return false;
				}
			}
		}
	}

	setPosY(int(posY + TILE_SIZE));
	return true;
}

void Figures::rotate(const std::vector<std::vector<sf::Color>>& colorMap, sf::Keyboard::Key key)
{
	int nextRotationState = rotationState;
	std::vector<std::pair<int, int>> kickTest;
	std::vector<std::pair<int, int>> nextKickLeft;
	std::vector<std::pair<int, int>> nextKickRight;

	switch (key)
	{
	case sf::Keyboard::C:
		kickTest = kickRight;
		nextRotationState = (rotationState + 1) % 4;
		break;
	case sf::Keyboard::X:
		kickTest = kickLeft;
		nextRotationState = (rotationState + 3) % 4;
		break;
	}
	
	std::vector<std::vector<int>> nextRotatedShape;

	for (const Figures& figure : Tetrominos)
	{
		if (figure.type == type && figure.rotationState == nextRotationState)
		{
			nextKickLeft = figure.kickLeft;
			nextKickRight = figure.kickRight;
			nextRotatedShape = figure.shape;
			break;
		}
	}

	//std::cout << "from: " << rotationState << " to: " << nextRotationState << std::endl;
	//for (const auto& kick : kickTest) { std::cout << kick.first << " " << kick.second << " -> "; }
	//std::cout << std::endl << posX / TILE_SIZE << std::endl;


	bool valid = true;
	std::vector<int> mapX;
	std::vector<int> mapY;
	for (int i = 0; i < nextRotatedShape.size(); i++)
	{
		for (int j = 0; j < nextRotatedShape[i].size(); j++)
		{
			if (nextRotatedShape[i][j] == 1)
			{
				mapX.push_back(int((posX / TILE_SIZE) + i));
				mapY.push_back(int((posY / TILE_SIZE) + j));

				//std::cout << int((posX / TILE_SIZE) + i) << " " << int((posY / TILE_SIZE) + j) << std::endl;
			}
		}
	}
	
	for (int i = 0; i < mapX.size(); i++)
	{
		if (mapX[i] >= 0 && mapX[i] < (WINDOW_WIDTH / TILE_SIZE) && mapY[i] >= 0 && mapY[i] < colorMap.size())
		{
			if (colorMap[mapY[i]][mapX[i]] != sf::Color::Black)
			{
				valid = false;
				break;
			}
		}
		else
		{
			valid = false;
			break;
		}
	}

	if (valid)
	{
		shape = nextRotatedShape;
		rotationState = nextRotationState;
		kickLeft = nextKickLeft;
		kickRight = nextKickRight;
		return;
	}


	for (const auto& kick : kickTest)
	{

		valid = true;

		for (int i = 0; i < mapX.size(); i++)
		{
			if (mapX[i]+kick.first >= 0 && mapX[i]+kick.first < 10 && mapY[i]+kick.second >= 0 && mapY[i]+kick.second < 20)
			{

				if (colorMap[mapY[i]+ kick.second ][mapX[i] + kick.first] != sf::Color::Black)
				{
					valid = false;
					break;
				}
			}
			else
			{
				valid = false;
				break;
			}
		}

		if (valid)
		{
			setPosX(int (  posX + (kick.first * TILE_SIZE ) ));
			setPosY(int ( posY + (kick.second * TILE_SIZE) ));
			shape = nextRotatedShape;
			rotationState = nextRotationState;
			kickLeft = nextKickLeft;
			kickRight = nextKickRight;
			return;
		}
	}
	
}


void Figures::drawNext(sf::RenderWindow& window, int next)
{
	Figures p = Tetrominos[next];
	std::vector<std::vector<int>> shape = p.getShape();

	nextShape.setFillColor(sf::Color::White);
	nextShape.setSize(sf::Vector2f(NEXTSIZE, NEXTSIZE));
	nextShape.setOutlineColor(sf::Color::Blue);
	nextShape.setOutlineThickness(1);

	for (int i = 0; i < shape.size(); i++) {
		for (int j = 0; j < shape[i].size(); j++) {
			if (shape[i][j] == 1)
			{
				nextShape.setPosition(float(WINDOW_WIDTH + (WINDOW_WIDTH / 2) + i * NEXTSIZE), float(WINDOW_HEIGHT / 10 + (j * NEXTSIZE)));
				window.draw(nextShape);
			}
		}
	}
}

void Figures::swapTetrominos(std::unique_ptr<Figures>& tetromino, std::queue<int>& tetrominoQueue)
{
	if (inventoryEmpty)
	{
		int nextTetromino = tetrominoQueue.front();
		tetrominoQueue.pop();

		if (tetrominoQueue.empty())
		{
			Game().refillTetrominoQueue(); 
		}
		tetromino->setPosX((WINDOW_WIDTH / 2) - (2 * TILE_SIZE));
		tetromino->setPosY(0);

		for (const Figures& figure : Tetrominos)
		{
			if (figure.type == type && figure.rotationState == 1)
			{
				tetromino->shape =  figure.shape;
				break;
			}
		}

		inventoryTetromino = std::move(tetromino);
		tetromino = std::make_unique<Figures>(Tetrominos[nextTetromino]);
		inventoryEmpty = false;
		didCtrlClick = true;
		return;
	}
	else if (!didCtrlClick)
	{

		didCtrlClick = true;
		std::swap(tetromino, inventoryTetromino);
		inventoryTetromino->setPosX((WINDOW_WIDTH / 2) - (2 * TILE_SIZE));
		inventoryTetromino->setPosY(0);
		for (const Figures& figure : Tetrominos)
		{
			if (figure.type == type && figure.rotationState == 1)
			{
				inventoryTetromino->shape = figure.shape;
				break;
			}
		}
		
	}

}
void Figures::drawInventory(sf::RenderWindow& window)
{
	if (!inventoryEmpty)
	{
		std::vector<std::vector<int>> shape = inventoryTetromino->getShape();

		inventoryShape.setFillColor(sf::Color::Magenta);
		inventoryShape.setSize(sf::Vector2f(NEXTSIZE, NEXTSIZE));
		inventoryShape.setOutlineColor(sf::Color::White);
		inventoryShape.setOutlineThickness(1);

		for (int i = 0; i < shape.size(); i++) {
			for (int j = 0; j < shape[i].size(); j++) {
				if (shape[i][j] == 1)
				{
					inventoryShape.setPosition(float(WINDOW_WIDTH + (WINDOW_WIDTH / 2) + i * NEXTSIZE), float(WINDOW_WIDTH + (j * NEXTSIZE)));
					window.draw(inventoryShape);
				}
			}
		}
	}
}

void Figures::setPosY(int posY)
{
	this->posY = posY;

}

void Figures::setPosX(int posX)
{

	this->posX = posX;
	
}
int Figures::getPosY()
{
	return posY;
}
int Figures::getMinPosY()
{
	return minY;
}
int Figures::getPosX()
{
	return posX;
}