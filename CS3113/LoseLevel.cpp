#include "LoseLevel.h"

LoseLevel::LoseLevel(Vector2 origin, const char *bgHexCode) : Scene(origin, bgHexCode) {}
LoseLevel::~LoseLevel() { shutdown(); }

void LoseLevel::initialise() {
    mGameState.nextSceneID = -1;
}

void LoseLevel::update(float deltaTime) {
    //Return to menu
    if (IsKeyPressed(KEY_ENTER)) {
        mGameState.nextSceneID = 0; 
    }
}

void LoseLevel::render() {
    ClearBackground(RED); 
    
    DrawText("YOU LOSE", mOrigin.x - 100, mOrigin.y - 40, 40, WHITE);
    DrawText("Press ENTER for Menu", mOrigin.x - 125, mOrigin.y + 20, 20, RAYWHITE);
}
void LoseLevel::shutdown() {}