#include <SFML/Graphics.hpp>
#include "Game.h"
#include <iostream>
#include <numeric>

#if PLAYER == 1
#define TILE_SIZE 40.0
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 800
#elif PLAYER == 2
#define TILE_SIZE 5.0
#define WINDOW_WIDTH 50
#define WINDOW_HEIGHT 100
#endif


class Tile: private Game{

protected:
	sf::RectangleShape tile;
	sf::RectangleShape lastLanded;
	float speed;
	int height;
	int holes;
	int gapCount;
	int nextCount;
	
	int bumpiness;
	int aggregateHeight;
	std::vector<std::vector<bool>> visited;
	std::vector<std::vector<sf::Color>> colorMap;
public:
	Tile();
	void updateMap(std::vector<std::vector<int>>& shape, int posX, int posY, sf::Color color);
	void drawTiles(sf::RenderWindow& window);
	void cleanLines();
	void calculateHolesNum();
	void calculateGapsNumber();
	void calculateBumpiness();
	void holeDFS(const int row, const int column);
	void checkMap();
	float getSpeed();
	std::vector<std::vector<sf::Color>> getColorMap();
	void resetMap();
	void resetHoles();
	int getHeight();
	int getHoles();
	int getGapCount();
	int getBumpiness();
	int getAggregateHeight();
	int getNextCount();
	bool isAnyRowCleared;

	~Tile();
};