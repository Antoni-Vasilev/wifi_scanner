#pragma once

#include "app.h"
#include "display_manager.h"

// Игрално поле: 32x16 клетки (всяка 4x4px = 128x64)
#define SN_COLS   32
#define SN_ROWS   16
#define SN_CELL   4
#define SN_MAXLEN 128

enum SnakeDir   { SN_UP, SN_DOWN, SN_LEFT, SN_RIGHT };
enum SnakeState { SN_START, SN_PLAYING, SN_GAME_OVER };

struct SnakeSegment {
    int col, row;
};

class SnakeApp : public App {
    private:
    DisplayManager* displayManager;
    bool redraw;

    SnakeSegment body[SN_MAXLEN];
    int length;

    SnakeDir dir;
    SnakeDir nextDir;

    int foodCol, foodRow;

    int score;
    SnakeState state;

    unsigned long lastMove;
    int moveInterval; // ms между стъпки

    void spawnFood();
    void drawBoard();
    void drawSnake();
    void drawFood();
    void drawHUD();
    void drawStartScreen();
    void drawGameOver();

    public:
    SnakeApp(DisplayManager* dm);

    void onEnter() override;
    void handleInput(InputEvent event) override;
    void update() override;
    void render() override;
    void forceRedraw() override { redraw = true; }

    bool needsRedraw() const override;
    void clearRedrawFlag() override;
};
