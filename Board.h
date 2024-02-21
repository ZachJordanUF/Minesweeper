#include <SFML/Graphics.hpp>
#include "tile.h"
using namespace std;
#pragma once



class Board {

    int rowAmount;
    int colAmount;
    int tilesRevealed;
    int mineAmount;
    int flagAmount;

public:
    vector<vector<Tile>> VectorofTiles;

    Board() {
        rowAmount = 0;
        colAmount = 0;
        mineAmount = 0;
        flagAmount = 0;
        tilesRevealed = 0;
    }

    Board(int _rowAmount, int _colAmount, int _mineAmount) : rowAmount(_rowAmount), colAmount(_colAmount), mineAmount(_mineAmount), flagAmount(0), tilesRevealed(0) {
        VectorofTiles.resize(rowAmount);
        for (auto& row : VectorofTiles) {
            row.resize(colAmount);
        }
        for (int i = 0; i < rowAmount; i++) {
            for (int j = 0; j < colAmount; j++) {
                VectorofTiles[i][j] = Tile(i, j);
            }
        }
    }

    void BoardInitialization() {
        for (int i = 0; i < mineAmount; i++) {
            placeMineRandomly();
        }
        calculateAdjacentMines();
    }

    void resetFlagsAndTiles() {
        flagAmount = 0;
        tilesRevealed = 0;
    }

    void resetTile(Tile& tile) {
        tile.updateRegBomb(false);
        tile.updateFlagTile(false);
        tile.upateTileReveal(false);
    }

    void boardRestart() {
        resetFlagsAndTiles();
        for (int i = 0; i < rowAmount; i++) {
            for (int j = 0; j < colAmount; j++) {
                resetTile(VectorofTiles[i][j]);
            }
        }
        BoardInitialization();
    }

    void createHTiles(int specRow, int specCol, sf::RenderWindow& window) {
        !VectorofTiles[specRow][specCol].isTileRevealed() ? create1HTile(specRow, specCol, window) : void();
    }

    void setGameLostState(bool& lose, bool& isPaused) {
        lose = true;
        isPaused = true;
    }

    int getTilesRevealed() {
        return tilesRevealed;
    }

    void drawSprites(sf::RenderWindow& window, const vector<sf::Sprite>& sprites) {
        window.clear(sf::Color::White);

        for (const auto& sprite : sprites) {
            window.draw(sprite);
        }

    }

    void create1HTile(int specRow, int specCol, sf::RenderWindow& window) {
        VectorofTiles[specRow][specCol].HiddenTileTexture();
        sf::Sprite tileSprite;
        tileSprite = VectorofTiles[specRow][specCol].getTileS();
        tileSprite.setPosition(specCol * 32, specRow * 32);
        window.draw(tileSprite);
    }

    void handlePauseClick(bool& paused) {
        paused = !paused;
    }

    void renderMine(int specRow, int specCol, sf::RenderWindow& window) {
        if (isMineAtPosition(specRow, specCol)) {
            setMineTextureAtPosition(specRow, specCol);
            drawSpriteAtPosition(specRow, specCol, window);
        }
    }

    bool MainButtonWasClicked(const sf::Sprite& sprite, const sf::Event& event) {
        return sprite.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y);
    }

    bool isMineAtPosition(int specRow, int specCol) {
        return VectorofTiles[specRow][specCol].hasBomb();
    }

    void setupLeaderboardText(sf::Text& text, sf::Font& font, int xWidthOfTile, int yHeightOfTile) {
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);
        text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
        text.setPosition((float)(xWidthOfTile) / 2.0f, (float)(yHeightOfTile) / 2.0f + 20);
    }

    void setMineTextureAtPosition(int specRow, int specCol) {
        VectorofTiles[specRow][specCol].bombTileTexture();
    }

    void resetTimeAndGame(sf::Time& time, sf::Clock& timer, bool& Paused, bool& playerLost, int& amountofWins) {
        time -= timer.getElapsedTime();
        time = sf::Time::Zero;

        Paused = false;
        playerLost = false;
        amountofWins = 0;
    }

    void drawSpriteAtPosition(int specRow, int specCol, sf::RenderWindow& window) {
        sf::Sprite sprite = VectorofTiles[specRow][specCol].getTileS();
        sprite.setPosition(specCol * 32, specRow * 32);
        window.draw(sprite);
    }

    void TilesRevealed(int specRow, int specCol) {
        Tile& aSingleTile = VectorofTiles[specRow][specCol];

        auto singleTileMine = aSingleTile.hasBomb();

        if (singleTileMine) {
            return;
        }

        if (aSingleTile.getBombCountAdjacent() == 0) {
            revealAdjacentTiles(specRow, specCol);
        }

        else {
            revealSingleTile(specRow, specCol);
        }
    }

    void revealAdjacentTiles(int specRow, int specCol) {
        Tile& aSingleTile = VectorofTiles[specRow][specCol];

        aSingleTile.upateTileReveal(true);
        tilesRevealed = tilesRevealed + 1;

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nextRow;
                nextRow = specRow + i;
                int nextCol;
                nextCol = specCol + j;

                if (isValidTile(nextRow, nextCol)) {
                    bool isTileToReveal;
                    Tile& nextTile = VectorofTiles[nextRow][nextCol];
                    isTileToReveal = !nextTile.isTileRevealed() && !nextTile.hasBomb() && !nextTile.isTileFlagged();
                    if (isTileToReveal) {
                        TilesRevealed(nextRow, nextCol);
                    }
                }
            }
        }
    }

    void revealSingleTile(int specRow, int specCol) {
        Tile& aSingleTile = VectorofTiles[specRow][specCol];
        tilesRevealed = tilesRevealed + 1;
        aSingleTile.upateTileReveal(true);
    }

    bool isValidTile(int specRow, int specCol) {
        bool isRowInRange = specRow >= 0 && specRow < rowAmount;
        bool isColInRange = specCol >= 0 && specCol < colAmount;

        return isRowInRange && isColInRange;
    }

    void handlePausedState(int i, int j, sf::RenderWindow& mainWindow, Board& board) {
        board.createHTiles(i, j, mainWindow);
    }

    vector <vector<Tile>> getVectorOfTiles() {
        return VectorofTiles;
    }

    void placeMineRandomly() {
        int randomRow;
        randomRow = rand() % rowAmount;
        int randomCol;
        randomCol = rand() % colAmount;

        if (VectorofTiles[randomRow][randomCol].hasBomb()) {
            placeMineRandomly();
        } else {
            VectorofTiles[randomRow][randomCol].updateRegBomb(true);
        }
    }

    void calculateAdjacentMines() {
        for (int i = 0; i < rowAmount; i++) {
            for (int j = 0; j < colAmount; j++) {
                int count = countAdjacentMines(i, j);
                VectorofTiles[i][j].updateBombsAdj(count);
            }
        }
    }

    int countAdjacentMines(int specRow, int specCol) {
        int mineCount = 0;

        for (int i : {-1, 0, 1}) {
            for (int j : {-1, 0, 1}) {
                int nextRow;
                nextRow = specRow + i;
                int nextCol;
                nextCol = specCol + j;

                if (isValidTile(nextRow, nextCol) && VectorofTiles[nextRow][nextCol].hasBomb()) {
                    mineCount = mineCount + 1;
                }
            }
        }

        return mineCount;
    }

    Tile getSpecificTile(int specRow, int specCol) {
        auto tile = VectorofTiles[specRow][specCol];
        return tile;
    }

    void handleDebugClick(bool& debugPressed) {
        debugPressed = !debugPressed;
    }

    void flagIncrementDecrement(bool flag) {
        if (!flag)
            flagAmount = flagAmount - 1;
        else
            flagAmount = flagAmount + 1;
    }

    int getFlagAmount() {
        return flagAmount;
    }

    int getAdjMinesAmount(int specRow, int specCol) {
        int minesAdjacent = 0;

        for (int i = -1; i <= 1; i++) {     // neighboring positions
            for (int j = -1; j <= 1; j++) {
                if (i != 0 || j != 0) {     // Skip the center tile
                    int nextRow;
                    nextRow = specRow + i;
                    int nextCol;
                    nextCol = specCol + j;

                    if (isValidTile(nextRow, nextCol)) {    // Check neighboring tile valid
                        if (VectorofTiles[nextRow][nextCol].hasBomb()) {    // Check if tile is a mine
                            minesAdjacent++;
                        }
                    }
                }
            }
        }

        return minesAdjacent;
    }

};

