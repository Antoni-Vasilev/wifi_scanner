#include "snake_app.h"
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

extern void goBackApp();

SnakeApp::SnakeApp(DisplayManager* dm)
  : displayManager(dm),
    redraw(true),
    length(3),
    dir(SN_RIGHT),
    nextDir(SN_RIGHT),
    foodCol(0), foodRow(0),
    score(0),
    state(SN_START),
    lastMove(0),
    moveInterval(200) {}

void SnakeApp::onEnter() {
    // Змията стартира в средата
    length = 3;
    dir    = SN_RIGHT;
    nextDir = SN_RIGHT;
    score  = 0;
    moveInterval = 200;
    state  = SN_START;

    body[0] = { 17, 8 };
    body[1] = { 16, 8 };
    body[2] = { 15, 8 };

    spawnFood();
    redraw = true;
}

void SnakeApp::spawnFood() {
    bool valid = false;
    while (!valid) {
        foodCol = random(1, SN_COLS - 1);
        foodRow = random(1, SN_ROWS - 1);
        valid   = true;
        for (int i = 0; i < length; i++) {
            if (body[i].col == foodCol && body[i].row == foodRow) {
                valid = false;
                break;
            }
        }
    }
}

void SnakeApp::handleInput(InputEvent event) {
    if (state == SN_START) {
        if (event == EVENT_RIGHT) {
            lastMove = millis();
            state    = SN_PLAYING;
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == SN_GAME_OVER) {
        if (event == EVENT_RIGHT) {
            onEnter();
            lastMove = millis();
            state    = SN_PLAYING;
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == SN_PLAYING) {
        switch (event) {
            case EVENT_UP:
                if (dir != SN_DOWN)  nextDir = SN_UP;
                break;
            case EVENT_DOWN:
                if (dir != SN_UP)    nextDir = SN_DOWN;
                break;
            case EVENT_LEFT:
                if (dir != SN_RIGHT) nextDir = SN_LEFT;
                break;
            case EVENT_RIGHT:
                if (dir != SN_LEFT)  nextDir = SN_RIGHT;
                break;
            default: break;
        }
    }
}

void SnakeApp::update() {
    if (state != SN_PLAYING) {
        redraw = true;
        return;
    }

    unsigned long now = millis();
    if (now - lastMove < (unsigned long)moveInterval) return;
    lastMove = now;

    dir = nextDir;

    // Нова глава
    int newCol = body[0].col;
    int newRow = body[0].row;

    switch (dir) {
        case SN_UP:    newRow--; break;
        case SN_DOWN:  newRow++; break;
        case SN_LEFT:  newCol--; break;
        case SN_RIGHT: newCol++; break;
    }

    // Wrap around
    if (newCol < 0)        newCol = SN_COLS - 1;
    if (newCol >= SN_COLS) newCol = 0;
    if (newRow < 0)        newRow = SN_ROWS - 1;
    if (newRow >= SN_ROWS) newRow = 0;

    // Провери сблъсък с тялото
    for (int i = 0; i < length - 1; i++) {
        if (body[i].col == newCol && body[i].row == newRow) {
            state  = SN_GAME_OVER;
            redraw = true;
            return;
        }
    }

    // Провери дали яде храна
    bool ate = (newCol == foodCol && newRow == foodRow);

    // Измести тялото
    if (!ate) {
        for (int i = length - 1; i > 0; i--) {
            body[i] = body[i - 1];
        }
    } else {
        // Удължи
        if (length < SN_MAXLEN) {
            for (int i = length; i > 0; i--) {
                body[i] = body[i - 1];
            }
            length++;
        }
        score += 10;
        // Ускори малко
        moveInterval = max(80, moveInterval - 5);
        spawnFood();
    }

    body[0] = { newCol, newRow };
    redraw  = true;
}

// =========================
// Render
// =========================

void SnakeApp::drawBoard() {
    Adafruit_SSD1306& display = displayManager->getDisplay();
    display.drawRect(0, 0, SN_COLS * SN_CELL, SN_ROWS * SN_CELL, WHITE);
}

void SnakeApp::drawSnake() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    for (int i = 0; i < length; i++) {
        int x = body[i].col * SN_CELL;
        int y = body[i].row * SN_CELL;

        if (i == 0) {
            // Глава — запълнен квадрат
            display.fillRect(x, y, SN_CELL, SN_CELL, WHITE);
        } else {
            // Тяло — малко по-малък квадрат
            display.fillRect(x + 1, y + 1, SN_CELL - 2, SN_CELL - 2, WHITE);
        }
    }
}

void SnakeApp::drawFood() {
    Adafruit_SSD1306& display = displayManager->getDisplay();
    int x = foodCol * SN_CELL;
    int y = foodRow * SN_CELL;
    // Храната е малък кръст
    display.drawPixel(x + 2, y + 1, WHITE);
    display.drawPixel(x + 1, y + 2, WHITE);
    display.drawPixel(x + 2, y + 2, WHITE);
    display.drawPixel(x + 3, y + 2, WHITE);
    display.drawPixel(x + 2, y + 3, WHITE);
}

void SnakeApp::drawHUD() {
    // Score е вграден в бордера горе вдясно
    Adafruit_SSD1306& display = displayManager->getDisplay();
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", score);
    int x = 128 - strlen(buf) * 6 - 2;
    display.fillRect(x - 1, 0, strlen(buf) * 6 + 2, 7, BLACK);
    display.setCursor(x, 1);
    display.print(buf);
}

void SnakeApp::drawStartScreen() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(40, 8);
    display.print("SNAKE");

    display.setCursor(15, 24);
    display.print("UP/DOWN/L/R=Move");

    display.setCursor(15, 44);
    display.print("RIGHT = Start");
    display.setCursor(15, 54);
    display.print("LEFT  = Back");
}

void SnakeApp::drawGameOver() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(25, 8);
    display.print("GAME OVER");

    display.setCursor(25, 24);
    display.print("Score: ");
    display.print(score);

    display.setCursor(25, 34);
    display.print("Len: ");
    display.print(length);

    display.setCursor(10, 48);
    display.print("RIGHT = Retry");
    display.setCursor(10, 57);
    display.print("LEFT  = Back");
}

void SnakeApp::render() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    switch (state) {
        case SN_START:
            drawStartScreen();
            break;
        case SN_GAME_OVER:
            drawGameOver();
            break;
        case SN_PLAYING:
            drawBoard();
            drawFood();
            drawSnake();
            drawHUD();
            break;
    }
}

bool SnakeApp::needsRedraw() const { return redraw; }
void SnakeApp::clearRedrawFlag() { redraw = false; }
