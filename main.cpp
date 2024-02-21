#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <ctype.h>
#include <unordered_map>
#include <queue>
#include "board.h"
#include "TextureManager.h"

using namespace std;

class ConfigFile{
    int rowCount;
    int colCount;
    int mineCount;
    string line;

public:

    ConfigFile () {
        rowCount = 0;
        colCount = 0;
        mineCount = 0;
    }

    void readConfig() {
        ifstream file("files/config.cfg");
        if (file.is_open()) {

            getline(file,line);
            rowCount = stoi(line);
            getline(file, line);
            colCount = stoi(line);
            getline(file, line);
            mineCount = stoi(line);
            file.close();
        }
    }

    int GetColCount() {
        return rowCount;
    }

    int GetRowCount() {
        return colCount;
    }

    int GetMineCount() {
        return mineCount;
    }
};

class Leader {
public:
    string name;
    int min;
    int sec;
    int playerStat;
};

void loadTextureAndSprites(const string& filePath, sf::Texture& texture, vector<sf::Sprite>& sprites, ConfigFile& configFile) {

    texture.loadFromFile(filePath);

    sprites.clear(); // Clear vect

    for (int i = 0; i < configFile.GetRowCount(); i++) {
        for (int j = 0; j < configFile.GetColCount(); j++) {
            sf::Sprite sprite(texture);
            sprite.setPosition(j * 32, i * 32);
            sprites.push_back(sprite);
        }
    }
}

void loadNumberTextureAndSprites(sf::Texture& texture, vector<sf::Sprite>& numberSprites, const string& filePath, ConfigFile& configFile) {
    texture.loadFromFile(filePath);

    float sizeOfTile = 32.0f;
    float xOff = 1.0f;
    float yOff = 1.0f;

    for (int i = 0; i < configFile.GetRowCount(); i++) {
        for (int j = 0; j < configFile.GetColCount(); j++) {
            sf::Sprite numberSprite(texture);
            float xPos = j * sizeOfTile + xOff;
            float yPos = i * sizeOfTile + yOff;
            numberSprite.setPosition(xPos, yPos);
            numberSprites.push_back(numberSprite);
        }
    }
}

void initializeBoardButtons(const string& filePath, sf::Texture& texture, sf::Sprite& sprite, ConfigFile& configFile, const sf::Vector2f& position) {
    texture.loadFromFile(filePath);
    sprite.setTexture(texture);
    sprite.setPosition(((configFile.GetColCount()) * 32) - position.x, 32 * ((configFile.GetRowCount()) + position.y));
}

void initializeClockDigits(sf::Texture& digitsTexture, sf::Sprite& sprite, int textureRectLeft, int position, ConfigFile& configFile) {
    sprite.setTexture(digitsTexture);
    sprite.setTextureRect(sf::IntRect(textureRectLeft, 0, 21, 32));
    sprite.setPosition(((configFile.GetColCount() * 32) - position), (32 * (configFile.GetRowCount() + 0.5f)) + 16);
}

void displayLeaderboardWindow(ConfigFile& configFile, sf::Font& font, sf::Text& leaderboardText) {
    int xWidthOfTile = configFile.GetColCount() * 16;
    int yHeightOfTile = configFile.GetRowCount() * 16 + 50;

    leaderboardText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    leaderboardText.setFillColor(sf::Color::White);
    leaderboardText.setPosition(xWidthOfTile / 2.0f, yHeightOfTile / 2.0f - 120);
    leaderboardText.setOrigin(leaderboardText.getLocalBounds().width / 2.0f, leaderboardText.getLocalBounds().height / 2.0f);
}

void handleFlaggedTile(int i, int j, sf::RenderWindow& mainWindow, Board& board, const vector<sf::Sprite>& flagsVector, ConfigFile& configFile, const vector<std::vector<Tile>>& tiles) {
    board.createHTiles(i, j, mainWindow);
    mainWindow.draw(flagsVector[i * configFile.GetColCount() + j]);
}

void handleLoseState(int i, int j, sf::RenderWindow& mainWindow, Board& board, const vector<sf::Sprite>& flagsVector, ConfigFile& configFile, const vector<std::vector<Tile>>& theTile, bool isPaused) {
    board.createHTiles(i, j, mainWindow);
    auto flagged = theTile[i][j].isTileFlagged();
    if (!flagged) {
        board.renderMine(i, j, mainWindow);
    } else {
        board.renderMine(i, j, mainWindow);
        mainWindow.draw(flagsVector[i * configFile.GetColCount() + j]);
    }

    isPaused = true;
}

void handleDebugMode(int i, int j, sf::RenderWindow& mainWindow, Board& board, const vector<sf::Sprite>& vectorTilesHidden, ConfigFile& configFile, const vector<std::vector<Tile>>& theTiles) {
    if (!theTiles[i][j].isTileFlagged()) {
        mainWindow.draw(vectorTilesHidden[i * configFile.GetColCount() + j]);
        board.renderMine(i, j, mainWindow);
    }
}

void setText(sf::Text &text, float x, float y) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.width / 2.0f, textRect.height / 2.0f);
    text.setPosition(sf::Vector2f(x, y));
}

void createClockDigit(sf::Sprite& digitSprite, sf::Texture& digitTexture, int digitValue, int posX, int posY) {
    digitSprite.setTexture(digitTexture);
    digitSprite.setTextureRect(sf::IntRect(digitValue * 21, 0, 21, 32));
    digitSprite.setPosition(posX, posY);
}

vector<Leader> loadLeaderboard(const string& filepath) {
    vector<Leader> leaders;

    std::ifstream leaderBoardFile(filepath);
    if (!leaderBoardFile) {
        return leaders;
    }
    string playerString;
    while (std::getline(leaderBoardFile, playerString)) {
        istringstream isStringStream(playerString);
        Leader player;

        getline(isStringStream, player.name, ',');
        isStringStream >> player.min;
        isStringStream.ignore();
        isStringStream >> player.sec;

        leaders.push_back(player);
    }

    leaderBoardFile.close();
    return leaders;
}

sf::Text createWelcomeText(const sf::Font& font, const sf::RenderWindow& window) {
    sf::Text welcomeText("WELCOME TO MINESWEEPER!", font, 24);
    welcomeText.setFillColor(sf::Color::White);
    welcomeText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    welcomeText.setOrigin(welcomeText.getLocalBounds().width / 2.0f, welcomeText.getLocalBounds().height / 2.0f);
    welcomeText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 150);
    return welcomeText;
}

sf::Text createUserText(const sf::Font& font, const sf::RenderWindow& window) {
    sf::Text userText("Enter your name :", font, 20);
    userText.setFillColor(sf::Color::White);
    userText.setStyle(sf::Text::Bold);
    userText.setOrigin(userText.getLocalBounds().width / 2.0f, userText.getLocalBounds().height / 2.0f);
    userText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 75);
    return userText;
}

sf::Text createUserWrite(const sf::Font& font, const sf::RenderWindow& window) {
    sf::Text userWrite("", font, 18);
    userWrite.setFillColor(sf::Color::Yellow);
    userWrite.setStyle(sf::Text::Bold);
    userWrite.setOrigin(userWrite.getLocalBounds().width / 2.0f, userWrite.getLocalBounds().height / 2.0f);
    userWrite.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 45);
    return userWrite;
}

sf::Text createCursor(const sf::Font& font) {
    sf::Text cursor("|", font, 18);
    cursor.setFillColor(sf::Color::Yellow);
    cursor.setStyle(sf::Text::Bold);
    return cursor;
}

void drawRevealedTileAndNumber(int i, int j, sf::RenderWindow& mainWindow, const vector<vector<Tile>>& tiles, Board& board, ConfigFile& configFile, const vector<vector<sf::Sprite>>& vectorOfNumberSprites, const vector<sf::Sprite>& revealedTilesVector) {
    if (board.getSpecificTile(i, j).isTileRevealed()) {
        mainWindow.draw(revealedTilesVector[i * configFile.GetColCount() + j]);

        int amountOfMineAdj = board.getAdjMinesAmount(i, j);
        if (amountOfMineAdj > 0 && amountOfMineAdj <= 8) {
            mainWindow.draw(vectorOfNumberSprites[amountOfMineAdj - 1][i * configFile.GetColCount() + j]);
        }
    } else {
        board.createHTiles(i, j, mainWindow);
    }
}

void showWinningGameFace(sf::RenderWindow& window, sf::Sprite& winFaceSprite, sf::Texture& texture) {
    winFaceSprite.setTexture(texture);

    window.clear();
    window.draw(winFaceSprite);
    window.display();
}

void drawLeaderboardWindow(sf::RenderWindow& window, sf::Text& informationOnLeaderboard, sf::Text& leaderboardText) {
    window.clear(sf::Color::Blue);
    window.draw(informationOnLeaderboard);
    window.draw(leaderboardText);
    window.display();
}

void drawScoreAndFlag(sf::RenderWindow& window, sf::Sprite& thousandsPlace, sf::Sprite& sTensPlace, sf::Sprite& sOnesPlace, sf::Sprite& hundredsPlace, sf::Sprite& flagOnes, sf::Sprite& flagTens, sf::Sprite& flagHundreds) {
    window.draw(thousandsPlace);
    window.draw(sTensPlace);
    window.draw(sOnesPlace);
    window.draw(hundredsPlace);
    window.draw(flagOnes);
    window.draw(flagTens);
    window.draw(flagHundreds);
}


int main() {

    ConfigFile configFile;
    configFile.readConfig();

    bool gamePaused = false;
    bool pressingDebug = false;
    int amountOfWins = 0;
    int winNumberCount = configFile.GetColCount() * configFile.GetRowCount() - configFile.GetMineCount();
    bool playerWon = false;
    bool playerLost = false;
    srand(time(NULL));
    sf::Time completeTime;
    completeTime = sf::Time::Zero;
    int rankOfPlayer = 1;
    int amountOfMineAdj = 0;
    string curText;

    vector<Leader> players;

    players = loadLeaderboard("files/leaderboard.txt");

    sf::Font font;
    font.loadFromFile("files/font.ttf");

    sf::RenderWindow welcomeWindow(sf::VideoMode(configFile.GetColCount() * 32, (configFile.GetRowCount() * 32) + 100), "Minesweeper");
    sf::Text welcomeText = createWelcomeText(font, welcomeWindow);
    sf::Text userText = createUserText(font, welcomeWindow);
    sf::Text userWrite = createUserWrite(font, welcomeWindow);
    sf::Text cursor = createCursor(font);


    while (welcomeWindow.isOpen()) {
        sf::Event event;
        while (welcomeWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                welcomeWindow.close();
                return 0;
            }
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) { // Unicode is ASCII values
                    if (event.text.unicode == 8){ // 8 = backspace
                        if(!userWrite.getString().isEmpty()) { // if str is not empty
                            curText = userWrite.getString();
                            curText.pop_back(); // get rid of last value in str
                            userWrite.setString(curText); // sets new str
                            setText(userWrite, welcomeWindow.getSize().x / 2.0f, welcomeWindow.getSize().y / 2.0f - 45);
                        }
                    }
                    else if (isalpha((char)(event.text.unicode))) {
                        // Append char
                        string currentText = userWrite.getString();

                        if (currentText.size() < 10) { //size of str

                            if (currentText.empty()) { // checks if str is empty
                                currentText += (char)(std::toupper(event.text.unicode)); //first char to upper case
                            }
                            else {
                                currentText += (char)(std::tolower(event.text.unicode)); //next characters to lower case
                            }
                            userWrite.setString(currentText);
                            setText(userWrite, welcomeWindow.getSize().x / 2.0f, welcomeWindow.getSize().y / 2.0f - 45);
                            cursor.setPosition(userWrite.getPosition().x + userWrite.getLocalBounds().width / 2.0f, userWrite.getPosition().y - 9);
                        }
                    }
                }
            }

            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    if (!userWrite.getString().isEmpty()) {
                        welcomeWindow.close();
                    }
                }
            }
        }

        cursor.setPosition(userWrite.getPosition().x + userWrite.getLocalBounds().width / 2.0f, userWrite.getPosition().y - 9); // resets the cursor whenever you backspace

        welcomeWindow.clear(sf::Color::Blue);
        welcomeWindow.draw(welcomeText);
        welcomeWindow.draw(userText);
        welcomeWindow.draw(userWrite);
        welcomeWindow.draw(cursor);
        welcomeWindow.display();
    }

    sf::RenderWindow mainWindow(sf::VideoMode(configFile.GetColCount() * 32, (configFile.GetRowCount() * 32) + 100), "Game Window");

    Board board(configFile.GetRowCount(), configFile.GetColCount(), configFile.GetMineCount());
    board.BoardInitialization();
    vector<vector<Tile>> allTiles = board.getVectorOfTiles();

    sf::Texture textureNumber1, textureNumber2, textureNumber3, textureNumber4, textureNumber5, textureNumber6, textureNumber7, textureNumber8;
    vector<sf::Sprite> vectorOf1, vectorOf2, vectorOf3, vectorOf4, vectorOf5, vectorOf6, vectorOf7, vectorOf8;

    loadNumberTextureAndSprites(textureNumber1, vectorOf1, "files/images/number_1.png", configFile);
    loadNumberTextureAndSprites(textureNumber2, vectorOf2, "files/images/number_2.png", configFile);
    loadNumberTextureAndSprites(textureNumber3, vectorOf3, "files/images/number_3.png", configFile);
    loadNumberTextureAndSprites(textureNumber4, vectorOf4, "files/images/number_4.png", configFile);
    loadNumberTextureAndSprites(textureNumber5, vectorOf5, "files/images/number_5.png", configFile);
    loadNumberTextureAndSprites(textureNumber6, vectorOf6, "files/images/number_6.png", configFile);
    loadNumberTextureAndSprites(textureNumber7, vectorOf7, "files/images/number_7.png", configFile);
    loadNumberTextureAndSprites(textureNumber8, vectorOf8, "files/images/number_8.png", configFile);

    vector<vector<sf::Sprite>> vectorOfNumberSprites;

    vectorOfNumberSprites.push_back(vectorOf1);
    vectorOfNumberSprites.push_back(vectorOf2);
    vectorOfNumberSprites.push_back(vectorOf3);
    vectorOfNumberSprites.push_back(vectorOf4);
    vectorOfNumberSprites.push_back(vectorOf5);
    vectorOfNumberSprites.push_back(vectorOf6);
    vectorOfNumberSprites.push_back(vectorOf7);
    vectorOfNumberSprites.push_back(vectorOf8);


    sf::Texture startGameFaceTexture, digitsForClockTexture, winningGameFaceTexture, losingGameFaceTexture, pauseButtonTexture, debugMinesButtonTexture, playButtonTexture, leaderboardButtonTexture;
    sf::Sprite winFaceSprite, loseFaceSprite, pauseSprite, debugSprite, playSprite, leaderboardButtonSprite;

    initializeBoardButtons("files/images/face_happy.png", startGameFaceTexture, winFaceSprite, configFile, {450, 0.5f});
    initializeBoardButtons("files/images/face_lose.png", losingGameFaceTexture, loseFaceSprite, configFile, {304, 0.5f});
    initializeBoardButtons("files/images/pause.png", pauseButtonTexture, pauseSprite, configFile, {240, 0.5f});
    initializeBoardButtons("files/images/debug.png", debugMinesButtonTexture, debugSprite, configFile, {304, 0.5f});
    initializeBoardButtons("files/images/play.png", playButtonTexture, playSprite, configFile, {64, 0.5f});
    initializeBoardButtons("files/images/leaderboard.png", leaderboardButtonTexture, leaderboardButtonSprite, configFile, {176, 0.5f});
    digitsForClockTexture.loadFromFile("files/images/digits.png");
    winningGameFaceTexture.loadFromFile("files/images/face_win.png");

    //creates the counter for the flags
    sf::Sprite flagHundreds, flagTens, flagOnes;
    createClockDigit(flagHundreds, digitsForClockTexture, 2, 33, 32 * (configFile.GetRowCount() + 0.5f) + 16);
    createClockDigit(flagTens, digitsForClockTexture, 3, 54, 32 * (configFile.GetRowCount() + 0.5f) + 16);
    createClockDigit(flagOnes, digitsForClockTexture, 5, 75, 32 * (configFile.GetRowCount() + 0.5f) + 16);

    sf::Texture textureOfHiddenTiles, textureOfFlags, textureOfRevealedTiles, textureOfMines;
    vector<sf::Sprite> hiddenTilesVector, flagsVector, revealedTilesVector, minesVector;

    loadTextureAndSprites("files/images/tile_hidden.png", textureOfHiddenTiles, hiddenTilesVector, configFile);
    loadTextureAndSprites("files/images/flag.png", textureOfFlags, flagsVector, configFile);
    loadTextureAndSprites("files/images/tile_revealed.png", textureOfRevealedTiles, revealedTilesVector, configFile);
    loadTextureAndSprites("files/images/mine.png", textureOfMines, minesVector, configFile);

    // creates clock digits for the timer
    sf::Sprite thousandsPlace, hundredsPlace, sTensPlace, sOnesPlace;
    initializeClockDigits(digitsForClockTexture, thousandsPlace, 0, 97, configFile);
    initializeClockDigits(digitsForClockTexture, hundredsPlace, 21, 76, configFile);
    initializeClockDigits(digitsForClockTexture, sTensPlace, 0, 54, configFile);
    initializeClockDigits(digitsForClockTexture, sOnesPlace, 21, 33, configFile);


    sf::Clock timer;

    while (mainWindow.isOpen()) {
        sf::Event event;
        while (mainWindow.pollEvent(event)) {

            //Timer and flag counter
            thousandsPlace.setTextureRect(sf::IntRect(21 * (((int)completeTime.asSeconds() / 60) / 10), 0, 21, 32));
            hundredsPlace.setTextureRect(sf::IntRect(21 * (((int)completeTime.asSeconds() / 60) % 10), 0, 21, 32));
            sTensPlace.setTextureRect(sf::IntRect(21 * (((int)(completeTime.asSeconds()) % 60) / 10), 0, 21, 32));
            sOnesPlace.setTextureRect(sf::IntRect(21 * (((int)(completeTime.asSeconds()) % 60) % 10), 0, 21, 32));
            flagHundreds.setTextureRect(sf::IntRect(21 * ((configFile.GetMineCount() - board.getFlagAmount()) / 100),0,21,32));
            flagTens.setTextureRect(sf::IntRect(21 * (((configFile.GetMineCount() - board.getFlagAmount()) / 10) % 10),0,21,32));
            flagOnes.setTextureRect(sf::IntRect(21 * ((configFile.GetMineCount() - board.getFlagAmount()) % 10),0,21,32));

            if (event.type == sf::Event::Closed) {
                mainWindow.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {

                int size = configFile.GetRowCount() * configFile.GetColCount();

                for (int i = 0; i < size; i++) {

                    int rowPlace;
                    rowPlace = i / configFile.GetColCount();
                    int colPlace;
                    colPlace = i % configFile.GetColCount();

                    float XtilePlace;
                    XtilePlace = colPlace * 32;
                    auto eventType = event.mouseButton.x >= XtilePlace && event.mouseButton.x < XtilePlace + 32;

                    float YtilePlace;
                    YtilePlace = rowPlace * 32;
                    auto eventType2 = event.mouseButton.y >= YtilePlace && event.mouseButton.y < YtilePlace + 32;

                    auto boardType = board.getSpecificTile(rowPlace, colPlace).isTileRevealed();
                    bool isMouseOverTile = (event.mouseButton.x >= XtilePlace && event.mouseButton.x < XtilePlace + 32 && event.mouseButton.y >= YtilePlace && event.mouseButton.y < YtilePlace + 32);


                    if (eventType) {
                        if (eventType2) {
                            if (!boardType) {
                                bool numBool = allTiles[rowPlace][colPlace].isTileFlagged();
                                bool Bool = !numBool;
                                allTiles[rowPlace][colPlace].updateFlagTile(Bool);
                                board.flagIncrementDecrement(Bool);
                            }
                        }
                    }
                }
            }

            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

                int size = configFile.GetRowCount() * configFile.GetColCount();

                for (int i = 0; i < size; i++) {
                    int rowPlace;
                    rowPlace = i / configFile.GetColCount();
                    int colPlace;
                    colPlace = i % configFile.GetColCount();
                    float XtilePlace;
                    XtilePlace = colPlace * 32;
                    float YtilePlace;
                    YtilePlace = rowPlace * 32;

                    bool isMouseOverTile = (event.mouseButton.x >= XtilePlace && event.mouseButton.x < XtilePlace + 32 && event.mouseButton.y >= YtilePlace && event.mouseButton.y < YtilePlace + 32);

                    if (isMouseOverTile) {

                        const auto& currentTile = board.getSpecificTile(rowPlace, colPlace);
                        const auto& currentTileSprite = allTiles[rowPlace][colPlace];
                        auto isBomb = currentTile.hasBomb() && !currentTileSprite.isTileFlagged();

                        if (isBomb) {
                            winFaceSprite.setTexture(losingGameFaceTexture);
                            board.setGameLostState(playerLost, gamePaused);
                        }

                        bool shouldReveal = !currentTile.isTileRevealed() && !currentTileSprite.isTileFlagged();

                        if (shouldReveal) {
                            amountOfWins = amountOfWins + 1;
                            board.TilesRevealed(rowPlace, colPlace);

                            if (!playerWon && board.getTilesRevealed() == configFile.GetColCount() * configFile.GetRowCount() - configFile.GetMineCount()) {
                                showWinningGameFace(mainWindow, winFaceSprite, winningGameFaceTexture);
                                playerWon = false;
                            }

                        }
                    }
                }

                if (board.MainButtonWasClicked(pauseSprite, event)) {

                    bool wasPaused = gamePaused;
                    board.handlePauseClick(gamePaused);

                    if (gamePaused != wasPaused) {
                        pauseSprite.setTexture(gamePaused ? playButtonTexture : pauseButtonTexture); // ? a conditional operator
                        completeTime += (gamePaused ? timer.getElapsedTime() : -timer.getElapsedTime()); // expression if true : if false
                    }
                }

                if (board.MainButtonWasClicked(winFaceSprite, event)) {

                    winFaceSprite.setTexture(startGameFaceTexture);
                    board.boardRestart();
                    pauseSprite.setTexture(pauseButtonTexture);

                    board.resetTimeAndGame(completeTime, timer, gamePaused, playerLost, amountOfWins);
                }


                if (board.MainButtonWasClicked(leaderboardButtonSprite, event)) {


                    sf::RenderWindow windowForLeaderBoard(sf::VideoMode(configFile.GetColCount() * 16, (configFile.GetRowCount() * 16) + 50), "Leaderboard Window");

                    string stats = "";

                    for (size_t i = 0; i < players.size(); ++i) {
                        players[i].playerStat = ++rankOfPlayer;
                        stringstream strstream;
                        strstream << setfill('0') << setw(2) << players[i].min << ":";
                        strstream << setfill('0') << setw(2) << players[i].sec;
                        stats = stats + to_string(players[i].playerStat) + ".    " + strstream.str() + "    " + players[i].name + "\n" + "\n";
                    }

                    sf::Text leaderboardText("LEADERBOARD", font, 20);
                    displayLeaderboardWindow(configFile, font, leaderboardText);


                    sf::Text informationOnLeaderboard(stats, font, 18);
                    board.setupLeaderboardText(informationOnLeaderboard, font, configFile.GetColCount() * 16, configFile.GetRowCount() * 16 + 50);

                    sf::Event leaderboardWindowEvent;

                    while (windowForLeaderBoard.isOpen()) {
                        while (windowForLeaderBoard.pollEvent(leaderboardWindowEvent)) {
                            if (leaderboardWindowEvent.type == sf::Event::Closed) {
                                windowForLeaderBoard.close();
                            }
                            else {
                                drawLeaderboardWindow(windowForLeaderBoard, informationOnLeaderboard, leaderboardText);
                            }
                        }
                    }
                }

                if (board.MainButtonWasClicked(debugSprite, event))
                    board.handleDebugClick(pressingDebug);
            }
        }
        completeTime += (!gamePaused) ? timer.getElapsedTime() : sf::Time::Zero; // experiment with ? ternary statement // if else statement
        timer.restart();

        vector<sf::Sprite> spritesToDraw = {winFaceSprite, leaderboardButtonSprite, debugSprite, pauseSprite};
        board.drawSprites(mainWindow, spritesToDraw);

        for (int i = 0; i < configFile.GetRowCount(); i++) {
            for (int j = 0; j < configFile.GetColCount(); j++) {
                auto tileFlagged = allTiles[i][j].isTileFlagged();
                auto specTileRevealed = board.getSpecificTile(i, j).isTileRevealed();

                if (playerLost) {
                    handleLoseState(i, j, mainWindow, board, flagsVector, configFile, allTiles, gamePaused);
                }
                else if (false) {
                    board.handlePausedState(i, j, mainWindow, board);
                }
                else if (tileFlagged) {
                    handleFlaggedTile(i, j, mainWindow, board, flagsVector, configFile, allTiles);
                }
                else if (pressingDebug) {
                    handleDebugMode(i, j, mainWindow, board, hiddenTilesVector, configFile, allTiles);
                }
                else if (specTileRevealed) {
                    drawRevealedTileAndNumber(i, j, mainWindow, allTiles, board, configFile, vectorOfNumberSprites, revealedTilesVector);
                }
                else
                    board.createHTiles(i, j, mainWindow);
            }
        }

        drawScoreAndFlag(mainWindow, thousandsPlace, sTensPlace, sOnesPlace, hundredsPlace, flagOnes, flagTens, flagHundreds);

        mainWindow.display();
    }

    return 0;
}
