#include "TutorialLevel.h"
extern Camera2D gCamera;

void TutorialLevel::initialise() {
    mGameState.nextSceneID = -1;

    mGameState.map = new Map(
      LEVEL_WIDTH, LEVEL_HEIGHT, (unsigned int *) mLevelData, 
      "assets/tileset.png", TILE_DIMENSION, 9, 6, mOrigin 
    );

    std::map<Direction, std::vector<int>> warriorAtlas = {
        {DOWN, {0,1,2,3,4,5,6,7}}, {UP, {0,1,2,3,4,5,6,7}}, 
        {RIGHT, {0,1,2,3,4,5,6,7}}, {LEFT, {0,1,2,3,4,5,6,7}}
    };

    std::map<Direction, std::vector<int>> EnemyAtlas = {
        {DOWN, {0, 1, 2, 3}}, {UP, {0, 1, 2, 3}}, 
        {RIGHT, {0, 1, 2, 3}}, {LEFT, {0, 1, 2, 3}}
    };


    mGameState.player = new Entity({400, 500}, {120, 120}, "assets/Warrior/Warrior_Idle.png", ATLAS, {6, 1}, warriorAtlas, PLAYER);


    mGameState.player->loadWarriorSprites(
        "assets/Warrior/Warrior_Idle.png", 
        "assets/Warrior/Warrior_Run.png", 
        "assets/Warrior/Warrior_Attack1.png", // Q
        "assets/Warrior/Warrior_Guard.png",   // W
        "assets/Warrior/Warrior_Attack2.png", // E
        "assets/Warrior/Warrior_Attack1.png"  // R 
    );

    mGameState.enemyCount = 4;
    mGameState.enemies = new Entity*[4];
    mGameState.enemies[0] = new Entity({800, 500}, {100, 130}, "assets/AdeptNecromancer.png", ATLAS, {4,1}, EnemyAtlas, NPC);
    mGameState.enemies[1] = new Entity({1100, 500}, {120, 150}, "assets/CorruptedTreant.png", ATLAS, {4,1}, EnemyAtlas, NPC);
    mGameState.enemies[2] = new Entity({1400, 500}, {150, 300}, "assets/IceGolem.png", ATLAS, {4,1}, EnemyAtlas, NPC);
    mGameState.enemies[3] = new Entity({1700, 500}, {40, 40}, "assets/GlowingWisp.png", ATLAS, {4,1}, EnemyAtlas, NPC);
}

void TutorialLevel::update(float deltaTime) {
    if (mGameState.player == nullptr) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        mTargetPos = GetScreenToWorld2D(GetMousePosition(), gCamera); 
        mIsMoving = true;
    }

    if (mIsMoving) {
        Vector2 currentPos = mGameState.player->getPosition();
        

        float diffX = mTargetPos.x - currentPos.x;
        float diffY = mTargetPos.y - currentPos.y;


        float distance = sqrt(diffX * diffX + diffY * diffY);

        if (distance > 5.0f) {
            Vector2 dir = { diffX / distance, diffY / distance };
            
            mGameState.player->setDirection(dir.x < 0 ? LEFT : RIGHT);
            mGameState.player->setMovement(dir);
            mGameState.player->setSpeed(mMoveSpeed); 
        } else { 
            mIsMoving = false; 
            mGameState.player->resetMovement(); 
        }
    }

    mGameState.player->updateAbilityTimers(deltaTime);
    mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
    
    if (IsKeyPressed(KEY_BACKSPACE)) mGameState.nextSceneID = 0;
}
void TutorialLevel::render() {
    ClearBackground(BLACK);
    if (mGameState.map != nullptr) mGameState.map->render();
    
    mGameState.player->render();
    for(int i=0; i<4; i++) mGameState.enemies[i]->render();

    DrawRectangle(70, 600, 1820, 280, ColorAlpha(BLACK, 0.8f)); 
    
    DrawText("TUTORIAL MODE: Press BACKSPACE to Return", 70, 50, 30, YELLOW);
    DrawText(" RIGHT CLICK TO MOVE AROUND", 150, 300, 40, WHITE);
    DrawText("ABILITIES GUIDE (HOVER OVER ABILITY TO SEE RANGE):" , 100, 620, 40, GOLD);
    DrawText("- [Q] Sprinting Man: Speed boost + contact damage.", 150, 680, 25, WHITE);
    DrawText("- [W] Breathe: Instant health restoration.", 150, 720, 25, WHITE);
    DrawText("- [E] Beyblade: Spin for Damage Over Time.", 150, 760, 25, WHITE);
    DrawText("- [R] Explode: Heavy ultimate damage. (YOU MUST AIM AT HIM!!! OTHERWISE IT WON'T WORK)", 150, 800, 25, WHITE);
    DrawText("- [B] Shop: Top (Sword) increase damage | Middle (Armor) increase damage reduction | Bottom (Health) increase max health.", 150, 840, 25, WHITE);
    DrawText("Goal: Beat all 3 stages without dying once :). GLHF! ", 200, 900, 40, GOLD);
    DrawText("Boss (More HP): kill to progress", 650, 580, 25, WHITE);
    DrawText("Monster (Less HP): it gives gold :)", 850, 375, 25, WHITE);
    DrawText("Tower (Gulp): it does big dmg... But rewarding :)", 1200, 275, 25, WHITE);
    DrawText("Tower's missle (Can't hit): it shoots...", 1600, 420, 25, WHITE);
}
void TutorialLevel::shutdown() {
    if (mGameState.map != nullptr) delete mGameState.map;
    if (mGameState.player != nullptr) delete mGameState.player;
    for (int i = 0; i < mGameState.enemyCount; i++) {
        if (mGameState.enemies[i] != nullptr) delete mGameState.enemies[i];
    }
    delete[] mGameState.enemies;
}
