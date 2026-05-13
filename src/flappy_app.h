#pragma once

#include "app.h"
#include "display_manager.h"

#define FB_PIPE_COUNT  3
#define FB_PIPE_WIDTH  8
#define FB_PIPE_GAP    18
#define FB_PIPE_SPEED  2
#define FB_PIPE_SPACING 50

#define FB_BIRD_X    20
#define FB_BIRD_SIZE  5

enum FlappyState {
    FB_START,
    FB_PLAYING,
    FB_DEAD,
    FB_GAME_OVER
};

struct FBPipe {
    int x;
    int gapY; // горен край на отвора
};

class FlappyApp : public App {
    private:
    DisplayManager* displayManager;
    bool redraw;

    // Птицата
    float birdY;
    float birdVel;

    // Тръби
    FBPipe pipes[FB_PIPE_COUNT];

    int score;
    int highScore;
    FlappyState state;

    unsigned long lastUpdate;
    int deathTimer;

    static const int SCREEN_H = 64;
    static const int SCREEN_W = 128;

    static constexpr float GRAVITY   = 0.35f;
    static constexpr float FLAP_VEL  = -2.8f;
    static constexpr float MAX_VEL   = 4.0f;

    void initPipes();
    void flap();
    bool checkCollision();

    void drawBird();
    void drawPipes();
    void drawHUD();
    void drawStartScreen();
    void drawGameOver();

    public:
    FlappyApp(DisplayManager* dm);

    void onEnter() override;
    void handleInput(InputEvent event) override;
    void update() override;
    void render() override;
    void forceRedraw() override { redraw = true; }

    bool needsRedraw() const override;
    void clearRedrawFlag() override;
};
