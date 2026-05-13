#include "flappy_app.h"
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

extern void goBackApp();

FlappyApp::FlappyApp(DisplayManager* dm)
  : displayManager(dm),
    redraw(true),
    birdY(32), birdVel(0),
    score(0), highScore(0),
    state(FB_START),
    lastUpdate(0),
    deathTimer(0) {
    initPipes();
}

void FlappyApp::onEnter() {
    birdY   = 32;
    birdVel = 0;
    score   = 0;
    state   = FB_START;
    deathTimer = 0;
    initPipes();
    redraw  = true;
}

void FlappyApp::initPipes() {
    for (int i = 0; i < FB_PIPE_COUNT; i++) {
        pipes[i].x    = SCREEN_W + i * FB_PIPE_SPACING;
        pipes[i].gapY = 10 + random(30);
    }
}

void FlappyApp::flap() {
    birdVel = FLAP_VEL;
}

bool FlappyApp::checkCollision() {
    int bx = FB_BIRD_X;
    int by = (int)birdY;
    int bs = FB_BIRD_SIZE;

    // Горна и долна граница
    if (by <= 0 || by + bs >= SCREEN_H) return true;

    // Тръби
    for (int i = 0; i < FB_PIPE_COUNT; i++) {
        int px = pipes[i].x;
        int gapTop    = pipes[i].gapY;
        int gapBottom = pipes[i].gapY + FB_PIPE_GAP;

        if (bx + bs > px && bx < px + FB_PIPE_WIDTH) {
            if (by < gapTop || by + bs > gapBottom) {
                return true;
            }
        }
    }
    return false;
}

void FlappyApp::handleInput(InputEvent event) {
    if (state == FB_START) {
        if (event == EVENT_UP || event == EVENT_RIGHT) {
            state   = FB_PLAYING;
            lastUpdate = millis();
            flap();
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == FB_GAME_OVER) {
        if (event == EVENT_RIGHT) {
            onEnter();
            state = FB_PLAYING;
            lastUpdate = millis();
            flap();
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
        return;
    }

    if (state == FB_PLAYING) {
        if (event == EVENT_UP || event == EVENT_RIGHT) {
            flap();
        } else if (event == EVENT_LEFT) {
            goBackApp();
        }
    }
}

void FlappyApp::update() {
    if (state == FB_START) {
        // Анимирана птица на start екрана
        redraw = true;
        return;
    }

    if (state == FB_DEAD) {
        deathTimer++;
        birdVel += GRAVITY;
        if (birdVel > MAX_VEL) birdVel = MAX_VEL;
        birdY += birdVel;
        redraw = true;
        if (deathTimer > 30) {
            state = FB_GAME_OVER;
        }
        return;
    }

    if (state == FB_GAME_OVER) {
        redraw = true;
        return;
    }

    if (state == FB_PLAYING) {
        unsigned long now = millis();
        if (now - lastUpdate < 30) return; // ~33fps
        lastUpdate = now;

        // Гравитация
        birdVel += GRAVITY;
        if (birdVel > MAX_VEL) birdVel = MAX_VEL;
        birdY += birdVel;

        // Движи тръбите
        for (int i = 0; i < FB_PIPE_COUNT; i++) {
            pipes[i].x -= FB_PIPE_SPEED;

            // Рецикъл на тръбата
            if (pipes[i].x + FB_PIPE_WIDTH < 0) {
                // Намери най-дясната тръба
                int maxX = 0;
                for (int j = 0; j < FB_PIPE_COUNT; j++) {
                    if (pipes[j].x > maxX) maxX = pipes[j].x;
                }
                pipes[i].x    = maxX + FB_PIPE_SPACING;
                pipes[i].gapY = 10 + random(30);
                score++;
            }
        }

        // Провери колизия
        if (checkCollision()) {
            if (score > highScore) highScore = score;
            state      = FB_DEAD;
            deathTimer = 0;
        }

        redraw = true;
    }
}

// =========================
// Render
// =========================

void FlappyApp::drawBird() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    int by = (int)birdY;

    // Тяло
    display.fillCircle(FB_BIRD_X + 2, by + 2, 3, WHITE);

    // Клюн
    if (state != FB_DEAD) {
        display.drawPixel(FB_BIRD_X + 5, by + 2, WHITE);
        display.drawPixel(FB_BIRD_X + 6, by + 3, WHITE);
    }

    // Крило — анимирано спрямо скоростта
    if (birdVel < 0) {
        // Махване нагоре
        display.drawPixel(FB_BIRD_X + 1, by - 1, WHITE);
        display.drawPixel(FB_BIRD_X + 2, by - 2, WHITE);
    } else {
        // Махване надолу
        display.drawPixel(FB_BIRD_X + 1, by + 5, WHITE);
        display.drawPixel(FB_BIRD_X + 2, by + 6, WHITE);
    }

    // Oko
    display.drawPixel(FB_BIRD_X + 3, by + 1, BLACK);
}

void FlappyApp::drawPipes() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    for (int i = 0; i < FB_PIPE_COUNT; i++) {
        int px = pipes[i].x;
        int gapTop    = pipes[i].gapY;
        int gapBottom = pipes[i].gapY + FB_PIPE_GAP;

        // Горна тръба
        if (gapTop > 0) {
            display.fillRect(px, 0, FB_PIPE_WIDTH, gapTop, WHITE);
            // Капачка
            display.fillRect(px - 1, gapTop - 3, FB_PIPE_WIDTH + 2, 3, WHITE);
        }

        // Долна тръба
        if (gapBottom < SCREEN_H) {
            display.fillRect(px, gapBottom, FB_PIPE_WIDTH, SCREEN_H - gapBottom, WHITE);
            // Капачка
            display.fillRect(px - 1, gapBottom, FB_PIPE_WIDTH + 2, 3, WHITE);
        }
    }
}

void FlappyApp::drawHUD() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    // Score горе в средата
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", score);
    int x = (SCREEN_W - strlen(buf) * 6) / 2;
    display.setCursor(x, 1);
    display.print(buf);
}

void FlappyApp::drawStartScreen() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(32, 5);
    display.print("FLAPPY BIRD");

    // Анимирана птица
    int by = 30 + (int)(3 * sin(millis() / 300.0));
    display.fillCircle(64, by, 4, WHITE);
    display.drawPixel(68, by, WHITE);
    display.drawPixel(69, by + 1, WHITE);

    display.setCursor(15, 44);
    display.print("UP/RIGHT = Flap");
    display.setCursor(15, 54);
    display.print("LEFT     = Back");
}

void FlappyApp::drawGameOver() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.setCursor(25, 5);
    display.print("GAME OVER");

    display.setCursor(25, 20);
    display.print("Score: ");
    display.print(score);

    display.setCursor(25, 30);
    display.print("Best:  ");
    display.print(highScore);

    display.setCursor(10, 46);
    display.print("RIGHT = Retry");
    display.setCursor(10, 56);
    display.print("LEFT  = Back");
}

void FlappyApp::render() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    switch (state) {
        case FB_START:
            drawStartScreen();
            break;
        case FB_GAME_OVER:
            drawGameOver();
            break;
        case FB_PLAYING:
        case FB_DEAD:
            drawPipes();
            drawBird();
            drawHUD();
            // Земя
            display.drawLine(0, SCREEN_H - 1, SCREEN_W, SCREEN_H - 1, WHITE);
            break;
    }
}

bool FlappyApp::needsRedraw() const { return redraw; }
void FlappyApp::clearRedrawFlag() { redraw = false; }
