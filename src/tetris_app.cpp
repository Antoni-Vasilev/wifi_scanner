#include "tetris_app.h"
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

extern void goBackApp();

// Тетромина: [тип][ротация][блок][col,row]
const int8_t TetrisApp::PIECES[TET_PIECES][4][4][2] = {
    // I
    {{{0,1},{1,1},{2,1},{3,1}}, {{2,0},{2,1},{2,2},{2,3}}, {{0,2},{1,2},{2,2},{3,2}}, {{1,0},{1,1},{1,2},{1,3}}},
    // O
    {{{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}},
    // T
    {{{1,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{2,1},{1,2}}, {{0,1},{1,1},{2,1},{1,2}}, {{1,0},{0,1},{1,1},{1,2}}},
    // S
    {{{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}}, {{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}}},
    // Z
    {{{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}}, {{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}}},
    // J
    {{{0,0},{0,1},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{1,2}}, {{0,1},{1,1},{2,1},{2,2}}, {{1,0},{1,1},{0,2},{1,2}}},
    // L
    {{{2,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{1,2},{2,2}}, {{0,1},{1,1},{2,1},{0,2}}, {{0,0},{1,0},{1,1},{1,2}}},
};

TetrisApp::TetrisApp(DisplayManager* dm)
  : displayManager(dm),
    redraw(true),
    curType(0), curRot(0), curCol(0), curRow(0),
    nextType(0),
    score(0), lines(0), level(1),
    lastFall(0), fallInterval(600),
    state(TET_START) {
    memset(board, 0, sizeof(board));
}

void TetrisApp::onEnter() {
    memset(board, 0, sizeof(board));
    score        = 0;
    lines        = 0;
    level        = 1;
    fallInterval = 600;
    state        = TET_START;
    nextType     = random(TET_PIECES);
    redraw       = true;
}

void TetrisApp::getBlocks(int type, int rot, int col, int row, int out[4][2]) {
    for (int i = 0; i < 4; i++) {
        out[i][0] = col + PIECES[type][rot][i][0];
        out[i][1] = row + PIECES[type][rot][i][1];
    }
}

bool TetrisApp::isValid(int type, int rot, int col, int row) {
    int blocks[4][2];
    getBlocks(type, rot, col, row, blocks);
    for (int i = 0; i < 4; i++) {
        int c = blocks[i][0];
        int r = blocks[i][1];
        if (c < 0 || c >= TET_COLS || r >= TET_ROWS) return false;
        if (r >= 0 && board[r][c]) return false;
    }
    return true;
}

void TetrisApp::spawnNext() {
    curType = nextType;
    curRot  = 0;
    curCol  = 3;
    curRow  = -1;
    nextType = random(TET_PIECES);

    if (!isValid(curType, curRot, curCol, curRow)) {
        state = TET_GAME_OVER;
    }
}

void TetrisApp::lockPiece() {
    int blocks[4][2];
    getBlocks(curType, curRot, curCol, curRow, blocks);
    for (int i = 0; i < 4; i++) {
        int c = blocks[i][0];
        int r = blocks[i][1];
        if (r >= 0 && r < TET_ROWS && c >= 0 && c < TET_COLS) {
            board[r][c] = 1;
        }
    }
    int cleared = clearLines();
    if (cleared > 0) {
        int pts[] = {0, 100, 300, 500, 800};
        score += pts[cleared] * level;
        lines += cleared;
        level = lines / 10 + 1;
        fallInterval = max(100, 600 - (level - 1) * 50);
    }
    spawnNext();
}

int TetrisApp::clearLines() {
    int cleared = 0;
    for (int r = TET_ROWS - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < TET_COLS; c++) {
            if (!board[r][c]) { full = false; break; }
        }
        if (full) {
            // Shift надолу
            for (int rr = r; rr > 0; rr--) {
                memcpy(board[rr], board[rr - 1], TET_COLS);
            }
            memset(board[0], 0, TET_COLS);
            cleared++;
            r++; // провери същия ред пак
        }
    }
    return cleared;
}

void TetrisApp::handleInput(InputEvent event) {
    if (state == TET_START) {
        if (event == EVENT_RIGHT) {
            spawnNext();
            lastFall = millis();
            state    = TET_PLAYING;
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == TET_GAME_OVER) {
        if (event == EVENT_RIGHT) {
            onEnter();
            spawnNext();
            lastFall = millis();
            state    = TET_PLAYING;
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == TET_PLAYING) {
        switch (event) {
            case EVENT_LEFT:
                if (isValid(curType, curRot, curCol - 1, curRow))
                    curCol--;
                break;
            case EVENT_RIGHT:
                if (isValid(curType, curRot, curCol + 1, curRow))
                    curCol++;
                break;
            case EVENT_UP: {
                // Ротация
                int newRot = (curRot + 1) % 4;
                if (isValid(curType, newRot, curCol, curRow))
                    curRot = newRot;
                else if (isValid(curType, newRot, curCol - 1, curRow)) {
                    curRot = newRot; curCol--;
                } else if (isValid(curType, newRot, curCol + 1, curRow)) {
                    curRot = newRot; curCol++;
                }
                break;
            }
            case EVENT_DOWN:
                // Ускори — падни с 1
                if (isValid(curType, curRot, curCol, curRow + 1)) {
                    curRow++;
                    score++;
                    lastFall = millis();
                }
                break;
            default: break;
        }
        redraw = true;
    }
}

void TetrisApp::update() {
    if (state != TET_PLAYING) {
        redraw = true;
        return;
    }

    unsigned long now = millis();
    if (now - lastFall >= (unsigned long)fallInterval) {
        lastFall = now;

        if (isValid(curType, curRot, curCol, curRow + 1)) {
            curRow++;
        } else {
            lockPiece();
        }
        redraw = true;
    }
}

// =========================
// Render
// =========================

void TetrisApp::drawBoard() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    const int offX = 0;
    const int offY = 0;

    // Бордер
    display.drawRect(offX, offY, TET_COLS * TET_CELL + 2, TET_ROWS * TET_CELL + 2, WHITE);

    for (int r = 0; r < TET_ROWS; r++) {
        for (int c = 0; c < TET_COLS; c++) {
            if (board[r][c]) {
                int x = offX + 1 + c * TET_CELL;
                int y = offY + 1 + r * TET_CELL;
                display.fillRect(x, y, TET_CELL - 1, TET_CELL - 1, WHITE);
            }
        }
    }
}

void TetrisApp::drawCurrentPiece() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    const int offX = 0;
    const int offY = 0;

    int blocks[4][2];
    getBlocks(curType, curRot, curCol, curRow, blocks);

    for (int i = 0; i < 4; i++) {
        int c = blocks[i][0];
        int r = blocks[i][1];
        if (r >= 0) {
            int x = offX + 1 + c * TET_CELL;
            int y = offY + 1 + r * TET_CELL;
            display.fillRect(x, y, TET_CELL - 1, TET_CELL - 1, WHITE);
        }
    }
}

void TetrisApp::drawNextPiece() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    const int offX = 48;
    const int offY = 30;

    display.setCursor(46, 22);
    display.print("Next");

    int blocks[4][2];
    getBlocks(nextType, 0, 0, 0, blocks);

    for (int i = 0; i < 4; i++) {
        int x = offX + blocks[i][0] * (TET_CELL - 1);
        int y = offY + blocks[i][1] * (TET_CELL - 1);
        display.fillRect(x, y, TET_CELL - 1, TET_CELL - 1, WHITE);
    }
}

void TetrisApp::drawHUD() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    // Score
    display.setCursor(46, 0);
    display.print("SC:");
    display.print(score);

    display.setCursor(46, 10);
    display.print("Lv:");
    display.print(level);

    display.setCursor(46, 18);  // Намалено разстояние
    display.print("Ln:");
    display.print(lines);

    // Next piece
    drawNextPiece();
}

void TetrisApp::drawStartScreen() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(30, 8);
    display.setTextSize(1);
    display.print("TETRIS");

    display.setCursor(10, 28);
    display.print("UP    = Rotate");
    display.setCursor(10, 38);
    display.print("L/R   = Move");
    display.setCursor(10, 48);
    display.print("DOWN  = Speed up");

    display.setCursor(10, 57);
    display.print("R=Start  L=Back");
}

void TetrisApp::drawGameOver() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(25, 8);
    display.print("GAME OVER");

    display.setCursor(20, 22);
    display.print("Score: ");
    display.print(score);

    display.setCursor(20, 32);
    display.print("Lines: ");
    display.print(lines);

    display.setCursor(20, 42);
    display.print("Level: ");
    display.print(level);

    display.setCursor(10, 54);
    display.print("R=Retry  L=Back");
}

void TetrisApp::render() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    switch (state) {
        case TET_START:
            drawStartScreen();
            break;
        case TET_GAME_OVER:
            drawGameOver();
            break;
        case TET_PLAYING:
            drawBoard();
            drawCurrentPiece();
            drawHUD();
            break;
    }
}

bool TetrisApp::needsRedraw() const { return redraw; }
void TetrisApp::clearRedrawFlag() { redraw = false; }
