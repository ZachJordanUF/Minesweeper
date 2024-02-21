#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
using namespace std;
#pragma once


class Tile {

    int tileRow;
    int tileCol;
    bool flag;
    bool bomb;
    int adjacentBombs;
    bool reveal;

    sf::Texture rTexture;
    sf::Sprite spriteRect;
    sf::Texture mTileTexture;
    sf::Texture hTileTexture;
    sf::Texture fTexture;


public:

    void initializeTextures(){

        hTileTexture.loadFromFile("files/images/tile_hidden.png");
        rTexture.loadFromFile("files/images/tile_revealed.png");
        mTileTexture.loadFromFile("files/images/mine.png");
        fTexture.loadFromFile("files/images/flag.png");
        spriteRect.setTexture(hTileTexture);
    }

    void resetMineAndRevealed() {
        bomb = false;
        reveal = false;
    }

    Tile() {


        tileRow = 0;
        tileCol = 0;
        flag = false;
        bomb = false;
        adjacentBombs = 0;
        reveal = false;

        resetMineAndRevealed();
        initializeTextures();
    }

    Tile(int _tileRow, int _tileCol) {
        tileRow = _tileRow;
        tileCol = _tileCol;
        adjacentBombs = 0;
        bomb = false;
        initializeTextures();
    }

    void updateFlagTile(bool _flag) {
        flag = _flag;
    }

    bool hasBomb() const {
        return bomb;
    }

    void updateRegBomb(bool _bomb) {
        bomb = _bomb;
    }

    bool isTileRevealed() const {
        return reveal;
    }

    void upateTileReveal(bool _reveal) {
        reveal = _reveal;
    }

    bool isTileFlagged() const {
        return flag;
    }

    sf::Sprite& getTileS() {
        return spriteRect;
    }

    void updateBombsAdj(int value) {
        adjacentBombs = value;
    }

    int getBombCountAdjacent() const {
        return adjacentBombs;
    }

    void HiddenTileTexture() {
        spriteRect.setTexture(hTileTexture);
    }

    void bombTileTexture() {
        spriteRect.setTexture(mTileTexture);
    }

    Tile &operator=(const Tile &other) {
        if (this == &other)
            return *this;

        bomb = other.bomb;
        reveal = other.reveal;
        return *this;
    }

    ~Tile(){}

    Tile(const Tile &other) {
        bomb = other.bomb;
        reveal = other.reveal;
    }


};

