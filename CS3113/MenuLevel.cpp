#include "MenuLevel.h"

MenuLevel::MenuLevel(Vector2 mOrigin, const char *bgHexCode) : Scene(mOrigin, bgHexCode) {}
MenuLevel::~MenuLevel() { shutdown(); }

void MenuLevel::initialise() {
    mGameState.nextSceneID = -1;
    mGameState.player = nullptr; 
    mGameState.map = nullptr;    
    mGameState.enemyCount = 0; 
}

void MenuLevel::update(float deltaTime) {
    Rectangle playBtn = { mOrigin.x - 100, mOrigin.y - 50, 200, 60 };
    Rectangle tutorBtn = { mOrigin.x - 100, mOrigin.y + 30, 200, 60 };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, playBtn)) {
            mGameState.nextSceneID = 1; 
        }
        if (CheckCollisionPointRec(mouse, tutorBtn)) {
            mGameState.nextSceneID = 6; 
        }
    }
}

void MenuLevel::render() {
    ClearBackground(ColorFromHex(mBGColourHexCode));
    
    Rectangle playBtn = { mOrigin.x - 100, mOrigin.y - 50, 200, 60 };
    Rectangle tutorBtn = { mOrigin.x - 100, mOrigin.y + 30, 200, 60 };

    DrawText("League of Trials", mOrigin.x-200, mOrigin.y-200, 50, WHITE);
    DrawRectangleRec(playBtn, DARKGRAY);
    DrawRectangleLinesEx(playBtn, 3, RAYWHITE);
    DrawText("PLAY", playBtn.x + 60, playBtn.y + 15, 30, WHITE);

    DrawRectangleRec(tutorBtn, DARKGRAY);
    DrawRectangleLinesEx(tutorBtn, 3, RAYWHITE);
    DrawText("TUTORIAL", tutorBtn.x + 25, tutorBtn.y + 15, 30, WHITE);
}

void MenuLevel::shutdown() {}