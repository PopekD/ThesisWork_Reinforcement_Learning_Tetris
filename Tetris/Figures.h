#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include "Tile.h"
#include <thread>
#include <cmath>

#if PLAYER == 1
#define NEXTSIZE 35
#elif PLAYER == 2
#define NEXTSIZE 5
#endif


enum class FiguresType
{
	I,
	L,
	O,
	S,
	T,
	J,
	Z
};


class Figures
{
private:
	FiguresType type;
	std::vector<std::vector<int>> shape;
	std::vector<std::pair<int, int>> kickLeft;
	std::vector<std::pair<int, int>> kickRight;
	sf::Color color;
	sf::RectangleShape tileShape;
	sf::RectangleShape nextShape;
	sf::RectangleShape inventoryShape;
	static std::unique_ptr<Figures> inventoryTetromino;
	int minX;
	int maxX;
	int maxY;
	int minY;
	bool isSpaceKeyPressed;
	bool isDownKeyPressed;
	static bool inventoryEmpty;
	const int bottomBoundary = int(WINDOW_HEIGHT - TILE_SIZE);
	int rotationState = 1;
public: static bool didCtrlClick;

protected:
	int posX = (WINDOW_WIDTH / 2) - (2 * TILE_SIZE);
	int posY = 0;
public:
	Figures(FiguresType figureType, sf::Color figureColor, std::vector<std::vector<int>> figureShape, int initialRotationState, std::vector<std::pair<int, int>> KickLeft, std::vector<std::pair<int, int>> KickRight)
		: type(figureType), color(figureColor), shape(figureShape), rotationState(initialRotationState), kickLeft(KickLeft), kickRight(KickRight) {}

	Figures(const Figures& other)
		: type(other.type),
		shape(other.shape),
		color(other.color),
		rotationState(other.rotationState),
		kickLeft(other.kickLeft),
		kickRight(other.kickRight)
	{}

	FiguresType getType() const {
		return type;
	}
	sf::Color getColor() const {
		return color;
	}
	const std::vector<std::vector<int>>& getShape() const {
		return shape;
	}
	bool drawFigures(sf::RenderWindow& window, const std::vector<std::vector<sf::Color>>& colorMap);
	void setPosX(int posX);
	int getPosX();
	void setPosY(int posY);
	int getPosY();
	void moveFigures();
	bool checkCollisions(const std::vector<std::vector<sf::Color>>& colorMap);
	void moveLeft(const std::vector<std::vector<sf::Color>>& colorMap);
	void moveRight(const std::vector<std::vector<sf::Color>>& colorMap);
	bool moveDown(const std::vector<std::vector<sf::Color>>& colorMap);
	void rotate(const std::vector<std::vector<sf::Color>>& colorMap, sf::Keyboard::Key key);
	bool checkCollisionsAtSpawn();
	void drawNext(sf::RenderWindow& window, int next);
	void isSpacePressed(bool KeyPressed);
	void isDownPressed(bool KeyPressed);
	void swapTetrominos(std::unique_ptr<Figures>& tetromin, std::queue<int>& tetrominoQueue);
	void drawInventory(sf::RenderWindow& window);
	int getMinPosY();
};

const std::vector<Figures> Tetrominos = {

	Figures(FiguresType::I, sf::Color(0, 128, 128), {
		{ 0, 0, 0 ,0 },
		{ 1, 1, 1, 1 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0 ,0 } //3
	}, 0,
	   { {-2, 0}, {1, 0}, {-2,-1}, {1, 2} },
	   { {1, 0}, {-2, 0}, {1, -2}, {-2, 1} }
		),
	Figures(FiguresType::I, sf::Color(0, 128, 128), {
		{ 0, 1, 0, 0 },
		{ 0, 1, 0, 0 }, //0
		{ 0, 1, 0, 0 },
		{ 0, 1, 0, 0 }
	}, 1,
	   { {-1,0}, {2,0}, {-1,2}, {2,-1} },
	   { {-2, 0}, {1, 0}, {-2, -1}, {1, 2} }
		),
   Figures(FiguresType::I, sf::Color(0, 128, 128), {
		{ 0, 0, 0 ,0 },
		{ 0, 0, 0, 0 }, // 1
		{ 1, 1, 1, 1 },
		{ 0, 0, 0 ,0 }
	}, 2,
	   { {2, 0}, {-1, 0}, {2, 1}, {-1, -2} },
	   { {-1,0}, {2, 0}, {-1, 2}, {2, -1} }
	   ),
	Figures(FiguresType::I, sf::Color(0, 128, 128), {
		{ 0, 0, 1, 0 },
		{ 0, 0, 1, 0 }, //2
		{ 0, 0, 1, 0 },
		{ 0, 0, 1, 0 }
	}, 3,
	   { {1, 0}, {-2,0}, {1,-2}, {-2,1} },
	   { {2, 0}, {-1, 0}, {2, 1}, {-1,-2} }
		),
	Figures(FiguresType::J, sf::Color(0,0,255), {
		{ 0, 0, 1},
		{ 1, 1, 1}, //3
		{ 0, 0, 0},
	}, 0,
	   { {-1,0}, {-1,-1}, {0,2}, {-1,2} },
	   { {-1,0}, {-1,-1}, {0,2}, {-1,2} }
	),
	Figures(FiguresType::J, sf::Color(0,0,255), {
		{ 1, 1, 0},
		{ 0, 1, 0}, //0
		{ 0, 1, 0}
	}, 1,
	   { {1,0},{1,1},{0,-2},{1,-2} },
	   { {-1,0},{-1,1},{0,-2},{-1,-2} }
		),
	Figures(FiguresType::J, sf::Color(0,0,255), {
		{ 0, 0, 0},
		{ 1, 1, 1}, //1
		{ 1, 0, 0},
	}, 2,
	   { {1,0},{1,-1},{0,2},{1,2}},
	   { {1,0}, {1,-1}, {0,2}, {1,2}}
		),
	Figures(FiguresType::J, sf::Color(0,0,255), {
		{ 0, 1, 0},
		{ 0, 1, 0},
		{ 0, 1, 1}, //2
	}, 3,
	   { {-1,0},{-1,1},{0,-2},{-1,-2} },
	   { {1,0}, {1,1}, {0, -2}, {1,-2} }
	),

	Figures(FiguresType::L, sf::Color(255, 127, 0), {
		{ 1, 0, 0},
		{ 1, 1, 1},
		{ 0, 0, 0},
	}, 0,
	   { {-1,0}, {-1,-1}, {0,2}, {-1,2} },
	   { {-1,0}, {-1,-1}, {0,2}, {-1,2} }
	),

	Figures(FiguresType::L, sf::Color(255, 127, 0), {
		{ 0, 1, 0},
		{ 0, 1, 0},
		{ 1, 1, 0},

	}, 1,
	   { {1,0},{1,1},{0,-2},{1,-2} },
	   { {-1,0},{-1,1},{0,-2},{-1,-2} }
	),
	Figures(FiguresType::L, sf::Color(255, 127, 0), {
		{ 0, 0, 0},
		{ 1, 1, 1},
		{ 0, 0, 1},
	}, 2,
	   { {1,0},{1,-1},{0,2},{1,2}},
	   { {1,0}, {1,-1}, {0,2}, {1,2}}
	),

	Figures(FiguresType::L, sf::Color(255, 127, 0), {
		{ 0, 1, 1},
		{ 0, 1, 0},
		{ 0, 1, 0},
	}, 3,
	   { {-1,0},{-1,1},{0,-2},{-1,-2} },
	   { {1,0}, {1,1}, {0, -2}, {1,-2} }
	),

	Figures(FiguresType::O, sf::Color(220, 184, 28), {
		{ 0, 0, 0, 0 },
		{ 1, 1, 0, 0 },
		{ 1, 1, 0, 0 }
	}, 0,
		{ {}, {}, {},  {} },
	   { {}, {}, {}, {} }
	),
	Figures(FiguresType::O, sf::Color(220, 184, 28), {
		{ 0, 0, 0, 0 },
		{ 1, 1, 0, 0 },
		{ 1, 1, 0, 0 }
	}, 1,
	   { {}, {}, {},  {} },
	   { {}, {}, {}, {} }
	),
	Figures(FiguresType::O, sf::Color(220, 184, 28), {
		{ 0, 0, 0, 0 },
		{ 1, 1, 0, 0 },
		{ 1, 1, 0, 0 }
	}, 2,
		{ {}, {}, {},  {} },
	   { {}, {}, {}, {} }
		),
	Figures(FiguresType::O, sf::Color(220, 184, 28), {
		{ 0, 0, 0, 0 },
		{ 1, 1, 0, 0 },
		{ 1, 1, 0, 0 }
	}, 3,
	   { {}, {}, {},  {} },
	   { {}, {}, {}, {} }
	),

	Figures(FiguresType::T, sf::Color(128, 0, 128), {
		{ 0, 1, 0 },
		{ 1, 1, 1 },
		{ 0, 0, 0 }
	}, 0,
	   { {-1,0}, {-1,-1}, {0,2}, {-1,2} },
	   { {-1,0}, {-1,-1}, {0,2}, {-1,2} }
	),
	Figures(FiguresType::T, sf::Color(128, 0, 128), {
		{ 0, 1, 0 },
		{ 1, 1, 0 },
		{ 0, 1, 0 }

	}, 1,
		{ {1,0},{1,1},{0,-2},{1,-2} },
		{ {-1,0},{-1,1},{0,-2},{-1,-2} }
	),
	Figures(FiguresType::T, sf::Color(128, 0, 128), {
		{ 0, 0, 0 },
		{ 1, 1, 1 },
		{ 0, 1, 0 }
	}, 2,
		{ {1,0},{1,-1},{0,2},{1,2} },
		{ {1,0}, {1,-1}, {0,2}, {1,2} }
	),
	Figures(FiguresType::T, sf::Color(128, 0, 128), {
		{ 0, 1, 0 },
		{ 0, 1, 1 },
		{ 0, 1, 0 }
	}, 3,
		{ {-1,0},{-1,1},{0,-2},{-1,-2} },
		{ {1,0}, {1,1}, {0, -2}, {1,-2} }
	),

	Figures(FiguresType::S, sf::Color(0, 128, 0), {
		{ 1, 1, 0 },
		{ 0, 1, 1 },
		{ 0, 0, 0 }
	}, 0,
		{ {-1,0}, {-1,-1}, {0,2}, {-1,2} },
		{ {-1,0}, {-1,-1}, {0,2}, {-1,2} }
		),
		Figures(FiguresType::S, sf::Color(0, 128, 0), {
			{ 0, 1, 0 },
			{ 1, 1, 0 },
			{ 1, 0, 0 }
	}, 1,
	   { {1,0},{1,1},{0,-2},{1,-2} },
	   { {-1,0},{-1,1},{0,-2},{-1,-2} }
	   ),
	   Figures(FiguresType::S, sf::Color(0, 128, 0), {
	   { 0, 0, 0 },
	   { 1, 1, 0 },
	   { 0, 1, 1 }
	}, 2,
	   { {1,0},{1,-1},{0,2},{1,2} },
	   { {1,0}, {1,-1}, {0,2}, {1,2} }
	   ),
	   Figures(FiguresType::S, sf::Color(0, 128, 0), {
		{ 0, 0, 1 },
		{ 0, 1, 1 },
		{ 0, 1, 0 }
	}, 3,
	   { {-1,0},{-1,1},{0,-2},{-1,-2} },
	   { {1,0}, {1,1}, {0, -2}, {1,-2} }
		),

	Figures(FiguresType::Z, sf::Color(128, 0, 0), {
		{ 0, 1, 1 },
		{ 1, 1, 0 },
		{ 0, 0, 0 }
		}, 0,
		{ {-1,0}, {-1,-1}, {0,2}, {-1,2} },
		{ {-1,0}, {-1,-1}, {0,2}, {-1,2} }

	),
		Figures(FiguresType::Z, sf::Color(128, 0, 0), {
			{ 1, 0, 0 },
			{ 1, 1, 0 },
			{ 0, 1, 0 }

			}, 1,
			{ {1,0},{1,1},{0,-2},{1,-2} },
			{ {-1,0},{-1,1},{0,-2},{-1,-2} }
		),
		Figures(FiguresType::Z, sf::Color(128, 0, 0), {
			{ 0, 0, 0 },
			{ 0, 1, 1 },
			{ 1, 1, 0 }
			}, 2,
			{ {1,0},{1,-1},{0,2},{1,2} },
			{ {1,0}, {1,-1}, {0,2}, {1,2} }
		),
		Figures(FiguresType::Z, sf::Color(128, 0, 0), {
		{ 0, 1, 0 },
		{ 0, 1, 1 },
		{ 0, 0, 1 }
	}, 3,
	   { {-1,0},{-1,1},{0,-2},{-1,-2} },
	   { {1,0}, {1,1}, {0, -2}, {1,-2} }
	)
};

