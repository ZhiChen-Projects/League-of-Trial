#include "LevelB.h"

extern Camera2D gCamera; 
extern Sound sfxDie; 

LevelB::LevelB() : Scene { {0.0f}, nullptr } {}
LevelB::LevelB(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
LevelB::~LevelB() { shutdown(); }

void LevelB::initialise()
{
   mGameState.nextSceneID = -1;

   mGameState.map = new Map(
      LEVEL_WIDTH, LEVEL_HEIGHT, (unsigned int *) mLevelData, 
      "assets/tileset.png", TILE_DIMENSION, 9, 6, mOrigin 
   );

   std::map<Direction, std::vector<int>> warriorAtlas = {
        {DOWN, {0,1,2,3,4,5,6,7}}, {UP, {0,1,2,3,4,5,6,7}}, 
        {RIGHT, {0,1,2,3,4,5,6,7}}, {LEFT, {0,1,2,3,4,5,6,7}}
    };

    mGameState.player = new Entity({mOrigin.x + 350, mOrigin.y + 937}, {120, 120}, "assets/Warrior/Warrior_Idle.png", ATLAS, {6, 1}, warriorAtlas, PLAYER);
    mGameState.player->setSpeed(mMoveSpeed);
    
    mGameState.player->loadWarriorSprites(
        "assets/Warrior/Warrior_Idle.png", 
        "assets/Warrior/Warrior_Run.png", 
        "assets/Warrior/Warrior_Attack1.png", // Q
        "assets/Warrior/Warrior_Guard.png",   // W
        "assets/Warrior/Warrior_Attack2.png", // E
        "assets/Warrior/Warrior_Attack1.png"  // R 
    );

   std::map<Direction, std::vector<int>> EnemyAtlas = {
        {DOWN, {0, 1, 2, 3}}, {UP, {0, 1, 2, 3}}, 
        {RIGHT, {0, 1, 2, 3}}, {LEFT, {0, 1, 2, 3}}
    };


   mGameState.player->setAcceleration({0.0f, 0.0f});

   // Enemy
    mGameState.enemyCount = 5;
    mGameState.enemies = new Entity*[mGameState.enemyCount];
    mEnemyPointers = mGameState.enemies;

    mGameState.enemies[0] = new Entity(
        {mOrigin.x + 3200.0f, mOrigin.y + 937.5f}, 
        {100.0f, 130.0f}, "assets/AdeptNecromancer.png", ATLAS, { 4,1}, EnemyAtlas, NPC 
    );

    Vector2 topBuffPos = {mOrigin.x + 1500.0f, mOrigin.y + 150.0f};
    Vector2 botBuffPos = {mOrigin.x + 1875.0f, mOrigin.y + 1650.0f};
    mGameState.enemies[4] = new Entity(
        botBuffPos, {120, 120}, "assets/CorruptedTreant.png", 
        ATLAS, {4,1}, EnemyAtlas, NPC
    );

    mGameState.enemies[1] = new Entity(topBuffPos, {120, 150}, "assets/CorruptedTreant.png", ATLAS, {4,1}, EnemyAtlas, NPC);
    mGameState.enemies[1]->setSpawn(topBuffPos); 
    mGameState.enemies[4]->setSpawn(botBuffPos);

    // Tower
    Vector2 towerPos = {mOrigin.x + 3500.0f, mOrigin.y + 937.5f};
    mGameState.enemies[2] = new Entity(towerPos, {150, 300}, "assets/IceGolem.png", ATLAS, {4, 1}, EnemyAtlas, NPC);
    
    // Missle
    mGameState.enemies[3] = new Entity({0,0}, {40, 40}, "assets/GlowingWisp.png", ATLAS, {4, 1}, EnemyAtlas, NPC);
    mGameState.enemies[3]->deactivate(); // Start invisible

    mGameState.player->setHP(600.0f); // Set Player HP

    mGameState.enemies[0]->setHP(1500.0f); // Set Enemy starting HP
    mGameState.enemies[1]->setHP(500.0f);
    mGameState.enemies[4]->setHP(500.0f);
}
void LevelB::update(float deltaTime)
{
    if (mGameState.player == nullptr) return;

    // Update Player Ability
    mGameState.player->updateAbilityTimers(deltaTime);

    // Right Click Movement 
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 mousePos = GetMousePosition();
        mTargetPos.x = (mousePos.x - gCamera.offset.x) + gCamera.target.x;
        mTargetPos.y = (mousePos.y - gCamera.offset.y) + gCamera.target.y;
        mIsMoving = true;
    }
    if (mIsMoving) {
        Vector2 currentPos = mGameState.player->getPosition();
        
        float diffX = mTargetPos.x - currentPos.x;
        float diffY = mTargetPos.y - currentPos.y;

        float distance = sqrt(diffX * diffX + diffY * diffY);

        // Only move if we aren't already at the target
        if (distance > 5.0f) {
            Vector2 dir = { diffX / distance, diffY / distance };
            
            // Speed Buff calculation
            float currentDynamicSpeed = mMoveSpeed * mGameState.player->getSpeedModifier();

            mGameState.player->setDirection(dir.x < 0 ? LEFT : RIGHT);
            mGameState.player->setMovement(dir);
            mGameState.player->setSpeed(currentDynamicSpeed); 

        } else { 
            mIsMoving = false; 
            mGameState.player->resetMovement(); 
            mGameState.player->setSpeed(mMoveSpeed);
        }
    }
    // Timer
    static float towerTimer = 0.0f;
    towerTimer -= deltaTime;

    // Enemy Logic
    for (int i = 0; i < mGameState.enemyCount; i++) {
        Entity* e = mGameState.enemies[i];
        if (!e || !e->isActive()) continue;

        float d = Vector2Distance(mGameState.player->getPosition(), e->getPosition());

        //Boss
        if (i == 0) {
            e->AIActivate(mGameState.player, deltaTime);

            float cTimer = e->getCastTimer();

            //His W
            if (e->getAIState() == CAST_W && cTimer >= 0.95f && !e->hasEnemyHit()) {
                if (Vector2Distance(mGameState.player->getPosition(), e->getCastPos()) < 120.0f) {
                    mGameState.player->takeDamage(150.0f);
                    e->consumeEnemyHit();
                }
            }
            //His Q
            if (e->getAIState() == CAST_Q && cTimer >= 1.45f && !e->hasEnemyHit()) {
                if (Vector2Distance(mGameState.player->getPosition(), e->getCastPos()) < 70.0f) {
                    mGameState.player->takeDamage(200.0f);
                    e->consumeEnemyHit();
                }
            }
            // His R
            if (e->getAIState() == CAST_R && cTimer >= 0.55f && !e->hasEnemyHit()) {
                if (Vector2Distance(mGameState.player->getPosition(), e->getCastPos()) < 180.0f) {
                    mGameState.player->takeDamage(250.0f);
                    mGameState.player->applyBlind(2.5f); 
                    e->consumeEnemyHit(); 
                }
            }
        } 
        else if (i == 1 || i == 4) {
            // Top Monster
            e->AILeashMonster(mGameState.player, deltaTime);

            // Hits every 10 sec
            if (d < 150.0f && e->getCastTimer() >= 10.0f) {
                mGameState.player->takeDamage(50.0f);
                e->resetAbilityTimer(); 
            }
        }

        // His E
        if (e->isEActive()) {
            if (Vector2Distance(e->getEPos(), mGameState.player->getPosition()) < 50.0f) {
                mGameState.player->takeDamage(150.0f);
                e->deactivateE();
            }
        }

        // Player's Damage
        float bonus = mGameState.player->getBonusDamage();
        if (mGameState.player->isSpinning() && d < 150) {
            mGameState.player->setAngle(mGameState.player->getAngle() + 1080 * deltaTime);
            e->takeDamage((25.0f + 1.5 * bonus) * deltaTime); 
        }
        if (!mGameState.player->isSpinning())mGameState.player->setAngle(0);
        if (mGameState.player->getSpeedModifier() > 1.0f && d < 80.0f) {
            e->takeDamage(75.0f + 1.2*bonus); 
            mGameState.player->consumeQ(); 
        }
        if (mGameState.player->isRActive() && d < 200.0f) {
            e->takeDamage(150.0f + bonus + (bonus * 2.0f));
            mGameState.player->consumeR();
        }

        e->update(deltaTime, mGameState.player, mGameState.map, nullptr, 0);

        // Update Gold everytime enemy dies
        if (e->isDead()) {
            if (i == 0) {
                mGameState.player->addGold(350);
                mGameState.nextSceneID = 3;
            }
                  // Boss 
            else if (i == 1 || i == 4) mGameState.player->addGold(75); // Monsters
            else if (i == 2) mGameState.player->addGold(1000); // Tower
            
            e->deactivate();
        }
        if (mGameState.player->isDead()) {
            mGameState.nextSceneID = 5; 
        }
    }

    // Tower and Missile
    Entity* tower = mGameState.enemies[2];
    Entity* missile = mGameState.enemies[3];

    // Missle shoots when tower is there
    if (tower != nullptr && tower->isActive()) {
        float distToTower = Vector2Distance(mGameState.player->getPosition(), tower->getPosition());
        if (distToTower < 600.0f && towerTimer <= 0.0f) {
            missile->activate();
            missile->setPosition(tower->getPosition());
            towerTimer = 2.0f; // Attack speed
        }
    }

    if (missile != nullptr && missile->isActive()) {
        missile->AIHomingProjectile(mGameState.player, 400.0f, deltaTime);

        if (Vector2Distance(missile->getPosition(), mGameState.player->getPosition()) < 30.0f) {
            mGameState.player->takeDamage(300.0f);
            missile->deactivate();
        }
    }

    mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
}

void LevelB::render() {
    ClearBackground(ColorFromHex(mBGColourHexCode));
    mGameState.map->render(); 
    
    // Draw Enemy
    if (mGameState.enemies != nullptr) {
        for (int i = 0; i < mGameState.enemyCount; i++) {
            if (mGameState.enemies[i]->isActive()) mGameState.enemies[i]->render();
        }
    }

    mGameState.player->render();
}

void LevelB::shutdown() {
   // Clean up Enemy array
   if (mEnemyPointers != nullptr) {
       for (int i = 0; i < mGameState.enemyCount; i++) {
           delete mEnemyPointers[i];
       }
       delete[] mEnemyPointers;
   }
   delete mGameState.player;
   delete mGameState.map;
}