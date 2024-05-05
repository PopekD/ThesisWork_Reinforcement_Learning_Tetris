#include "Tile.h"


Tile::Tile()
{
    tile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    speed = 1;
    tile.setOutlineThickness(1);
    colorMap.resize(WINDOW_HEIGHT / TILE_SIZE, std::vector<sf::Color>(WINDOW_WIDTH / TILE_SIZE, sf::Color::Black));
    visited.resize(WINDOW_HEIGHT / TILE_SIZE, std::vector<bool>(WINDOW_WIDTH / TILE_SIZE, false));
}

void Tile::cleanLines()
{
    std::vector<std::vector<sf::Color>> tempColorMap = colorMap;
    int numClearedRows = 0;
    bool isAnyRowCleared = false;


    for (int rows = WINDOW_HEIGHT / TILE_SIZE - 1; rows >= 0; rows--) {
        bool isRowCleared = true;


        for (int columns = 0; columns < WINDOW_WIDTH / TILE_SIZE; columns++) {
            if (colorMap[rows][columns] == sf::Color::Black) {
                isRowCleared = false;
                break;
            }
        }


        if (isRowCleared) {
            numClearedRows++;
            if (speed >= 0.05) {
                speed -= 0.01;
            }
            isAnyRowCleared = true;
        }
        else {
            tempColorMap[rows + numClearedRows] = colorMap[rows];
        }
    }


    for (int rows = 0; rows < numClearedRows; rows++) {
        for (int columns = 0; columns < WINDOW_WIDTH / TILE_SIZE; columns++) {
            tempColorMap[rows][columns] = sf::Color::Black;
        }
    }

    if (isAnyRowCleared) {
        setScore(numClearedRows);
        colorMap = tempColorMap;
    }

    
}

void Tile::calculateBumpiness()
{
    bumpiness = 0;
    aggregateHeight = 0;

    std::vector<int> bumpinessLevel{ std::vector<int>(10, 0) };

    for (int i = height; i < 20; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (colorMap[i][j] != sf::Color::Black)
            {
                bumpinessLevel[j] = std::max(bumpinessLevel[j], std::abs(i - 19));
            }

        }
    }
   
    aggregateHeight = std::abs(std::accumulate(bumpinessLevel.begin(), bumpinessLevel.end(), 0));

    for (int j = 0; j < 9; j++) {
        int heightDiff = std::abs(bumpinessLevel[j] - bumpinessLevel[j + 1]);
        bumpiness += heightDiff;
    }

}
void Tile::calculateGapsNumber()
{
    gapCount = 0;
    nextCount = 0;
    for (int i = 19; i >= height; i--){
        int found = false;
        for (int j = 0; j < WINDOW_WIDTH / TILE_SIZE; j++)
        {
            if (colorMap[i][j] != sf::Color::Black) 
            { 
                if (found)
                {
                    gapCount++;
                    if (j + 1 > 9 || colorMap[i][j + 1] != sf::Color::Black) { found = false; }
                }
                if(!found && j+1 < 10 && colorMap[i][j+1] == sf::Color::Black){ found = true; }   

                if (j - 1 >= 0 && j + 1 < 10 && (colorMap[i][j - 1] == sf::Color::Black || colorMap[i][j + 1] == sf::Color::Black))
                {
                    nextCount++;
                }
            }
            
            //if ( (j - 1 < 0 && colorMap[i][j] == sf::Color::Black && colorMap[i][j + 1] != sf::Color::Black )|| (j + 1 > 9 && colorMap[i][j] == sf::Color::Black && colorMap[i][j - 1] != sf::Color::Black))
            //{
            //    gapCount++;
            //    continue;
            //}
            //if (j - 1 < 0 || j + 1 > 9) { continue; }
            //if ( colorMap[i][j] == sf::Color::Black &&  (colorMap[i][j + 1] != sf::Color::Black || colorMap[i][j - 1] != sf::Color::Black))
            //{
            //    gapCount++;
            //}
        }
    }
}

void Tile::calculateHolesNum()
{
    resetHoles();
    for (int i = 0; i < WINDOW_HEIGHT / TILE_SIZE; i++) {
        for (int j = 0; j < WINDOW_WIDTH / TILE_SIZE; j++) {
            if (!visited[i][j] && colorMap[i][j] == sf::Color::Black) {
                holes++;
                holeDFS(i, j);
            }
        }
    }
}

void Tile::holeDFS(const int row,const int column)
{

    if (row < 0 || row >= WINDOW_HEIGHT / TILE_SIZE || column < 0 || column >= WINDOW_WIDTH / TILE_SIZE || visited[row][column])
        return;

    visited[row][column] = true;
    
    if (colorMap[row][column] == sf::Color::Black)
    {
        holeDFS(row + 1, column); 
        holeDFS(row - 1, column); 
        holeDFS(row, column + 1);
        holeDFS(row, column - 1);
    }
}

void Tile::drawTiles(sf::RenderWindow& window)
{
    cleanLines();
    calculateHolesNum();

    std::vector<std::vector<int>> TetrominoPosition;
    bool minDone = false;
    height = 19;
    for (int rows = 0; rows < WINDOW_HEIGHT / TILE_SIZE; rows++)
    {
        for (int columns = 0; columns <  WINDOW_WIDTH  / TILE_SIZE; columns++)
        {
            tile.setPosition(sf::Vector2f(float(columns * TILE_SIZE), float(rows * TILE_SIZE)));
            
            if (colorMap[rows][columns] != sf::Color::Black && columns != WINDOW_WIDTH / TILE_SIZE) {
                if (!minDone) { height = rows; minDone = true; }
                TetrominoPosition.push_back({ rows, columns} ); 
            }
            tile.setOutlineColor(sf::Color::Black);
            tile.setFillColor(colorMap[rows][columns]);

            window.draw(tile);
        }
    }
    for (auto& position : TetrominoPosition)
    {

        tile.setPosition(sf::Vector2f(float(position[1] * TILE_SIZE), float(position[0] * TILE_SIZE)));
        tile.setOutlineColor(sf::Color::White);
        tile.setFillColor(colorMap[position[0]][position[1]]);
        window.draw(tile);
    }

   calculateGapsNumber();
   calculateBumpiness();

}


void Tile::updateMap(std::vector<std::vector<int>>& shape, int posX, int posY, sf::Color color)
{
    getColorMap();
    for (int i = 0; i < shape.size(); i++) {
        for (int j = 0; j < shape[i].size(); j++) {
            if (shape[i][j] == 1) {
                int row = (posY + (j * TILE_SIZE)) / TILE_SIZE;
                int column = (posX + (i * TILE_SIZE)) / TILE_SIZE;
                colorMap[row][column] = color;
            }
        }
    }
}


std::vector<std::vector<sf::Color>> Tile::getColorMap(){
    return colorMap;
}
float Tile::getSpeed()
{
    return speed;
}
void Tile::resetMap()
{
    for (int i = 0; i < colorMap.size(); i++) {
        for (int j = 0; j < colorMap[i].size(); j++) {
            colorMap[i][j] = sf::Color::Black;
        }
    }
}
void Tile::resetHoles()
{
    holes = 0;
    for (int i = 0; i < visited.size(); i++) {
        for (int j = 0; j < visited[i].size(); j++) {
            visited[i][j] = false;
        }
    }
}

int Tile::getBumpiness()
{
    return bumpiness;
}

int Tile::getHeight()
{
    return height;
}
int Tile::getHoles()
{
    return holes;
}
int Tile::getGapCount()
{
    return gapCount;
}

int Tile::getAggregateHeight()
{
    return aggregateHeight;
}
int Tile::getNextCount()
{
    return nextCount;
}

Tile::~Tile()
{

}