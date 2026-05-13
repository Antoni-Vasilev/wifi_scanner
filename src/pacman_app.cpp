#include "pacman_app.h"
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

extern void goBackApp();

// Лабиринт шаблон
// 1=стена, 2=точка, 3=power pellet, 0=празно
const uint8_t PacmanApp::MAZE_TEMPLATE[PM_ROWS][PM_COLS] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 3, 2, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 3, 1 },
    { 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1 },
    { 1, 2, 2, 1, 2, 1, 1, 0, 0, 1, 1, 2, 1, 2, 2, 1 },
    { 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1 },
    { 1, 3, 2, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 3, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
};

PacmanApp::PacmanApp(DisplayManager* dm)
  : displayManager(dm),
    redraw(true),
    dotsLeft(0),
    score(0),
    lives(3),
    pacX(0), pacY(0),
    pacDir(DIR_RIGHT),
    nextDir(DIR_RIGHT),
    pacMoveTimer(0),
    pacAnimFrame(0),
    state(PM_START),
    stateTimer(0),
    deathAnimFrame(0) {
    memset(maze, 0, sizeof(maze));
}

void PacmanApp::onEnter() {
    score  = 0;
    lives  = 3;
    state  = PM_START;
    stateTimer = millis();
    initLevel();
    redraw = true;
}

void PacmanApp::initLevel() {
    dotsLeft = 0;
    for (int r = 0; r < PM_ROWS; r++) {
        for (int c = 0; c < PM_COLS; c++) {
            maze[r][c] = MAZE_TEMPLATE[r][c];
            if (maze[r][c] == PM_DOT || maze[r][c] == PM_POWER) dotsLeft++;
        }
    }
    resetPositions();
}

void PacmanApp::resetPositions() {
    // Pac-Man стартира в долната средна зона
    pacX      = cellToPixel(7);
    pacY      = cellToPixel(5);  // ред 5 вместо 3
    pacDir    = DIR_RIGHT;
    nextDir   = DIR_RIGHT;
    pacMoveTimer  = 0;
    pacAnimFrame  = 0;
    deathAnimFrame = 0;

    // Призрак 1 — горе вляво от центъра
    ghosts[0].col = 6;
    ghosts[0].row = 2;
    ghosts[0].px  = cellToPixel(6);
    ghosts[0].py  = cellToPixel(2);
    ghosts[0].dir = DIR_LEFT;
    ghosts[0].frightened = false;
    ghosts[0].frightenedUntil = 0;
    ghosts[0].moveTimer = 0;

    // Призрак 2 — горе вдясно от центъра
    ghosts[1].col = 9;
    ghosts[1].row = 2;
    ghosts[1].px  = cellToPixel(9);
    ghosts[1].py  = cellToPixel(2);
    ghosts[1].dir = DIR_RIGHT;
    ghosts[1].frightened = false;
    ghosts[1].frightenedUntil = 0;
    ghosts[1].moveTimer = 0;
}

void PacmanApp::handleInput(InputEvent event) {
    if (state == PM_START) {
        if (event == EVENT_RIGHT) {
            state = PM_PLAYING;
            stateTimer = millis();
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == PM_GAME_OVER || state == PM_WIN) {
        if (event == EVENT_RIGHT) {
            score = 0;
            lives = 3;
            initLevel();
            state = PM_PLAYING;
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == PM_PLAYING) {
        switch (event) {
            case EVENT_UP:    nextDir = DIR_UP;    break;
            case EVENT_DOWN:  nextDir = DIR_DOWN;  break;
            case EVENT_LEFT:  nextDir = DIR_LEFT;  break;
            case EVENT_RIGHT: nextDir = DIR_RIGHT; break;
            default: break;
        }
    }
}

bool PacmanApp::canMove(int col, int row) {
    if (col < 0 || col >= PM_COLS || row < 0 || row >= PM_ROWS) return false;
    return maze[row][col] != PM_WALL;
}

void PacmanApp::updatePacman() {
    pacMoveTimer++;
    if (pacMoveTimer < PAC_SPEED) return;
    pacMoveTimer = 0;

    // Опитай да смениш посоката
    int nc = pixelToCell(pacX);
    int nr = pixelToCell(pacY);

    int tryCol = nc, tryRow = nr;
    switch (nextDir) {
        case DIR_UP:    tryRow--; break;
        case DIR_DOWN:  tryRow++; break;
        case DIR_LEFT:  tryCol--; break;
        case DIR_RIGHT: tryCol++; break;
        default: break;
    }

    if (canMove(tryCol, tryRow)) {
        pacDir = nextDir;
    }

    // Движи се в текущата посока
    int nx = pacX, ny = pacY;
    switch (pacDir) {
        case DIR_UP:    ny -= 1; break;
        case DIR_DOWN:  ny += 1; break;
        case DIR_LEFT:  nx -= 1; break;
        case DIR_RIGHT: nx += 1; break;
        default: break;
    }

    int newCol = pixelToCell(nx + 3);
    int newRow = pixelToCell(ny + 3);

    if (canMove(newCol, newRow)) {
        pacX = nx;
        pacY = ny;

        // Wrap around
        if (pacX < 0) pacX = cellToPixel(PM_COLS - 1);
        if (pacX >= cellToPixel(PM_COLS)) pacX = 0;
    }

    // Яж точки
    int cellCol = pixelToCell(pacX + 3);
    int cellRow = pixelToCell(pacY + 3);

    if (cellRow >= 0 && cellRow < PM_ROWS && cellCol >= 0 && cellCol < PM_COLS) {
        if (maze[cellRow][cellCol] == PM_DOT) {
            maze[cellRow][cellCol] = PM_EMPTY;
            score += 10;
            dotsLeft--;
        } else if (maze[cellRow][cellCol] == PM_POWER) {
            maze[cellRow][cellCol] = PM_EMPTY;
            score += 50;
            dotsLeft--;
            // Уплаши призраците
            unsigned long now = millis();
            for (int i = 0; i < 2; i++) {
                ghosts[i].frightened      = true;
                ghosts[i].frightenedUntil = now + POWER_DURATION_MS;
            }
        }
    }

    pacAnimFrame = (pacAnimFrame + 1) % 4;
}

PacDir PacmanApp::randomDir(Ghost& g) {
    PacDir dirs[4] = { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
    // Опитай произволна посока
    int start = random(4);
    for (int i = 0; i < 4; i++) {
        PacDir d = dirs[(start + i) % 4];
        int nc = g.col, nr = g.row;
        switch (d) {
            case DIR_UP:    nr--; break;
            case DIR_DOWN:  nr++; break;
            case DIR_LEFT:  nc--; break;
            case DIR_RIGHT: nc++; break;
            default: break;
        }
        if (canMove(nc, nr)) return d;
    }
    return g.dir;
}

void PacmanApp::updateGhost(Ghost& g) {
    g.moveTimer++;
    int speed = g.frightened ? GHOST_SPEED + 1 : GHOST_SPEED;
    if (g.moveTimer < speed) return;
    g.moveTimer = 0;

    // Освободи от frightened ако е изтекло
    if (g.frightened && millis() >= g.frightenedUntil) {
        g.frightened = false;
    }

    // Изчисли следващата клетка
    int nc = g.col, nr = g.row;
    switch (g.dir) {
        case DIR_UP:    nr--; break;
        case DIR_DOWN:  nr++; break;
        case DIR_LEFT:  nc--; break;
        case DIR_RIGHT: nc++; break;
        default: break;
    }

    if (canMove(nc, nr)) {
        g.col = nc;
        g.row = nr;
        g.px  = cellToPixel(nc);
        g.py  = cellToPixel(nr);
    } else {
        g.dir = randomDir(g);
    }

    // Смени посока на кръстовища
    if (random(3) == 0) {
        g.dir = randomDir(g);
    }
}

void PacmanApp::checkCollisions() {
    for (int i = 0; i < 2; i++) {
        int dx = abs(pacX - ghosts[i].px);
        int dy = abs(pacY - ghosts[i].py);

        if (dx < 5 && dy < 5) {
            if (ghosts[i].frightened) {
                // Изяж призрака
                score += 200;
                ghosts[i].col = 7;
                ghosts[i].row = 3;
                ghosts[i].px  = cellToPixel(7);
                ghosts[i].py  = cellToPixel(3);
                ghosts[i].frightened = false;
            } else {
                // Умри
                lives--;
                state      = PM_DYING;
                stateTimer = millis();
            }
        }
    }
}

void PacmanApp::update() {
    if (state == PM_START || state == PM_GAME_OVER || state == PM_WIN) {
        redraw = true;
        return;
    }

    if (state == PM_DYING) {
        deathAnimFrame++;
        redraw = true;
        if (deathAnimFrame > 20) {
            if (lives <= 0) {
                state = PM_GAME_OVER;
            } else {
                resetPositions();
                state = PM_PLAYING;
            }
        }
        return;
    }

    if (state == PM_PLAYING) {
        updatePacman();
        updateGhost(ghosts[0]);
        updateGhost(ghosts[1]);
        checkCollisions();

        if (dotsLeft <= 0) {
            state = PM_WIN;
            stateTimer = millis();
        }

        redraw = true;
    }
}

// =========================
// Render
// =========================

void PacmanApp::drawHUD() {
    Adafruit_SSD1306& display = displayManager->getDisplay();
    display.setCursor(0, 0);
    display.print("SC:");
    display.print(score);

    // Живота
    for (int i = 0; i < lives; i++) {
        display.fillCircle(90 + i * 10, 3, 3, WHITE);
    }
}

void PacmanApp::drawMaze() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    const int offsetY = 8; // под HUD-а

    for (int r = 0; r < PM_ROWS; r++) {
        for (int c = 0; c < PM_COLS; c++) {
            int x = c * PM_CELL;
            int y = r * PM_CELL + offsetY;

            if (maze[r][c] == PM_WALL) {
                display.fillRect(x, y, PM_CELL, PM_CELL, WHITE);
            } else if (maze[r][c] == PM_DOT) {
                display.fillRect(x + 3, y + 3, 1, 1, WHITE);
            } else if (maze[r][c] == PM_POWER) {
                display.fillCircle(x + 3, y + 3, 2, WHITE);
            }
        }
    }
}

void PacmanApp::drawPacman() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    const int offsetY = 8;
    int cx = pacX + 3;
    int cy = pacY + 3 + offsetY;

    if (state == PM_DYING) {
        // Анимация на смъртта — свиващ се кръг
        int r = 3 - (deathAnimFrame / 5);
        if (r > 0) display.fillCircle(cx, cy, r, WHITE);
        return;
    }

    // Pac-Man с уста
    bool mouthOpen = (pacAnimFrame < 2);

    if (!mouthOpen) {
        display.fillCircle(cx, cy, 3, WHITE);
        return;
    }

    // Рисувай кръг с уста в зависимост от посоката
    display.fillCircle(cx, cy, 3, WHITE);

    // Изтрий "устата" — триъгълник
    switch (pacDir) {
        case DIR_RIGHT:
            display.fillTriangle(cx, cy, cx + 4, cy - 2, cx + 4, cy + 2, BLACK);
            break;
        case DIR_LEFT:
            display.fillTriangle(cx, cy, cx - 4, cy - 2, cx - 4, cy + 2, BLACK);
            break;
        case DIR_UP:
            display.fillTriangle(cx, cy, cx - 2, cy - 4, cx + 2, cy - 4, BLACK);
            break;
        case DIR_DOWN:
            display.fillTriangle(cx, cy, cx - 2, cy + 4, cx + 2, cy + 4, BLACK);
            break;
        default: break;
    }
}

void PacmanApp::drawGhost(Ghost& g) {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    const int offsetY = 8;
    int x = g.px;
    int y = g.py + offsetY;

    uint16_t color = WHITE;

    if (g.frightened) {
        // Мига в края на frightened времето
        unsigned long remaining = g.frightenedUntil - millis();
        if (remaining < 1500 && (millis() / 200) % 2 == 0) {
            color = BLACK; // мига
        }
    }

    // Тяло на призрака — закръглен правоъгълник
    display.fillRoundRect(x, y + 1, 6, 5, 2, color);
    display.fillRect(x, y + 3, 6, 3, color);

    // Долен ръб — "крачета"
    display.fillRect(x,     y + 5, 2, 2, color);
    display.fillRect(x + 4, y + 5, 2, 2, color);

    if (!g.frightened) {
        // Очи
        display.fillRect(x + 1, y + 2, 1, 1, BLACK);
        display.fillRect(x + 4, y + 2, 1, 1, BLACK);
    }
}

void PacmanApp::drawStartScreen() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(35, 10);
    display.setTextSize(1);
    display.print("PAC-MAN");

    display.fillCircle(64, 30, 6, WHITE);
    display.fillTriangle(64, 30, 72, 26, 72, 34, BLACK);

    display.setCursor(20, 44);
    display.print("RIGHT = Start");
    display.setCursor(20, 54);
    display.print("LEFT  = Back");
}

void PacmanApp::drawGameOver() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(28, 10);
    display.print("GAME OVER");

    display.setCursor(20, 26);
    display.print("Score: ");
    display.print(score);

    display.setCursor(10, 44);
    display.print("RIGHT = Retry");
    display.setCursor(10, 54);
    display.print("LEFT  = Back");
}

void PacmanApp::drawWin() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(35, 10);
    display.print("YOU WIN!");

    display.setCursor(20, 26);
    display.print("Score: ");
    display.print(score);

    display.setCursor(10, 44);
    display.print("RIGHT = Play again");
    display.setCursor(10, 54);
    display.print("LEFT  = Back");
}

void PacmanApp::render() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    switch (state) {
        case PM_START:
            drawStartScreen();
            break;
        case PM_GAME_OVER:
            drawGameOver();
            break;
        case PM_WIN:
            drawWin();
            break;
        case PM_PLAYING:
        case PM_DYING:
            drawHUD();
            drawMaze();
            drawGhost(ghosts[0]);
            drawGhost(ghosts[1]);
            drawPacman();
            break;
    }
}

bool PacmanApp::needsRedraw() const { return redraw; }
void PacmanApp::clearRedrawFlag() { redraw = false; }
