#pragma once

#include "app.h"
#include "display_manager.h"

// Лабиринт: 16 колони x 7 реда (клетки)
// Всяка клетка е 7x7 пиксела
// Горният ред (8px) е за score
#define PM_COLS 16
#define PM_ROWS 7
#define PM_CELL 7

// Стойности на клетките
#define PM_WALL  1
#define PM_DOT   2
#define PM_EMPTY 0
#define PM_POWER 3  // голяма точка

enum PacDir { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE };

struct Ghost {
    int col, row;       // позиция в клетки
    int px, py;         // позиция в пиксели (за плавно движение)
    PacDir dir;
    bool frightened;    // синьо след power pellet
    unsigned long frightenedUntil;
    int moveTimer;
};

enum PacmanState {
    PM_PLAYING,
    PM_DYING,
    PM_WIN,
    PM_GAME_OVER,
    PM_START
};

class PacmanApp : public App {
    private:
    DisplayManager* displayManager;
    bool redraw;

    // Лабиринт — копие за текущото ниво
    uint8_t maze[PM_ROWS][PM_COLS];
    int dotsLeft;
    int score;
    int lives;

    // Pac-Man позиция в пиксели
    int pacX, pacY;
    PacDir pacDir;
    PacDir nextDir;
    int pacMoveTimer;
    int pacAnimFrame; // за анимация на устата

    // Призраци
    Ghost ghosts[2];

    PacmanState state;
    unsigned long stateTimer;
    int deathAnimFrame;

    // Константи
    static const int PAC_SPEED   = 3;  // пиксели на стъпка
    static const int GHOST_SPEED = 12;
    static const int POWER_DURATION_MS = 5000;

    // Лабиринт шаблон
    static const uint8_t MAZE_TEMPLATE[PM_ROWS][PM_COLS];

    void initLevel();
    void resetPositions();

    void updatePacman();
    void updateGhost(Ghost& g);
    void checkCollisions();

    bool canMove(int col, int row);
    int  pixelToCell(int px) { return px / PM_CELL; }
    int  cellToPixel(int c)  { return c * PM_CELL; }

    void drawMaze();
    void drawPacman();
    void drawGhost(Ghost& g);
    void drawHUD();
    void drawStartScreen();
    void drawGameOver();
    void drawWin();

    PacDir randomDir(Ghost& g);

    public:
    PacmanApp(DisplayManager* dm);

    void onEnter() override;
    void handleInput(InputEvent event) override;
    void update() override;
    void render() override;
    void forceRedraw() override { redraw = true; }

    bool needsRedraw() const override;
    void clearRedrawFlag() override;
};
