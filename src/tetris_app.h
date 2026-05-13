#pragma once

#include "app.h"
#include "display_manager.h"

// Игралното поле: 10 колони x 14 реда
// Всяка клетка е 4x4 пиксела
// Полето заема 40x56 пиксела, вляво
// Вдясно (42-127) е за score, next piece и т.н.

#define TET_COLS  10
#define TET_ROWS  14
#define TET_CELL  4

// 7 тетромина
#define TET_PIECES 7

enum TetrisState {
    TET_START,
    TET_PLAYING,
    TET_GAME_OVER
};

struct TetPiece {
    int8_t blocks[4][2]; // 4 блока, всеки с [col, row] офсет
};

class TetrisApp : public App {
    private:
    DisplayManager* displayManager;
    bool redraw;

    uint8_t board[TET_ROWS][TET_COLS]; // 0=празно, 1=запълнено

    // Текуща фигура
    int curType;
    int curRot;
    int curCol;
    int curRow;

    // Следваща фигура
    int nextType;

    int score;
    int lines;
    int level;

    unsigned long lastFall;
    int fallInterval; // ms между падания

    TetrisState state;

    // Всички 7 тетромина в 4 ротации
    static const int8_t PIECES[TET_PIECES][4][4][2];

    void newPiece();
    void spawnNext();
    bool isValid(int type, int rot, int col, int row);
    void lockPiece();
    int  clearLines();
    void getBlocks(int type, int rot, int col, int row, int out[4][2]);

    void drawBoard();
    void drawCurrentPiece();
    void drawNextPiece();
    void drawHUD();
    void drawStartScreen();
    void drawGameOver();

    public:
    TetrisApp(DisplayManager* dm);

    void onEnter() override;
    void handleInput(InputEvent event) override;
    void update() override;
    void render() override;
    void forceRedraw() override { redraw = true; }

    bool needsRedraw() const override;
    void clearRedrawFlag() override;
};
