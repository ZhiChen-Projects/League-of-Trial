#include "WinLevel.h"

WinLevel::WinLevel(Vector2 origin, const char *bgHexCode) : Scene(origin, bgHexCode) {}
WinLevel::~WinLevel() { shutdown(); }

void WinLevel::initialise() {
    mGameState.nextSceneID = -1;
}

void WinLevel::update(float deltaTime) {
    // Return to main menu
    if (IsKeyPressed(KEY_ENTER)) {
        mGameState.nextSceneID = 0; 
    }
}

void WinLevel::render() {
    ClearBackground(ColorFromHex(mBGColourHexCode)); 

    DrawText("YOU WIN!", mOrigin.x - 100, mOrigin.y - 40, 40, BLACK);
    DrawText("Press ENTER for Menu", mOrigin.x - 125, mOrigin.y + 20, 20, BLACK);
}

void WinLevel::shutdown() {}