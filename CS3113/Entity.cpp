#include "Entity.h"

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction, 
        std::vector<int>> animationAtlas, EntityType entityType) : 
        mPosition {position}, mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f}, 
        mMovement { 0.0f, 0.0f }, mScale {scale}, mColliderDimensions {scale}, 
        mTexture {LoadTexture(textureFilepath)}, mTextureType {ATLAS}, 
        mSpriteSheetDimensions {spriteSheetDimensions}, mAnimationAtlas {animationAtlas}, 
        mDirection {RIGHT}, mAnimationIndices {animationAtlas.at(RIGHT)}, 
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f }, mSpeed { DEFAULT_SPEED }, 
        mEntityType {entityType} 
{
    mBlindTimer = 0.0f;
    mEnemyHasHit = false;
    mRShotTimer = 0.0f;
    mRShotsLeft = 0;
    mGlobalTimer = 0.0f;
    mGlobalCooldown = 2.5f;
    mAbilityTimer = 0.0f;
    mMaxHP = 700.0f;
    mCurrentHP = 700.0f;
    mGold = 300;
    mWeaponLevel = 0; mArmorLevel = 0; mHealthLevel = 0;
    mBonusDamage = 0.0f; mDamageReduction = 0.0f;
    mWCooldown = 0.0f; 
}

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {

            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - 
                              (collidableEntity->mColliderDimensions.y / 2.0f));
            
            if (mVelocity.y > 0) 
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (mVelocity.y < 0) 
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;

                if (collidableEntity->mEntityType == BLOCK)
                    collidableEntity->deactivate();
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {            
            // When standing on a platform, we're always slightly overlapping
            // it vertically due to gravity, which causes false horizontal
            // collision detections. So the solution I found is only resolve X
            // collisions if there's significant Y overlap, preventing the 
            // platform we're standing on from acting like a wall.
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            // Skip if barely touching vertically (standing on platform)
            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;

                // Collision!
                mIsCollidingRight = true;
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
 
                // Collision!
                mIsCollidingLeft = true;
            }
        }
    }
}

void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    Vector2 topCentreProbe    = { mPosition.x, mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topLeftProbe      = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topRightProbe     = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };

    Vector2 bottomCentreProbe = { mPosition.x, mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomLeftProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomRightProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ABOVE (jumping upward)
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap)) && mVelocity.y < 0.0f)
    {
        mPosition.y += yOverlap;   // push down
        mVelocity.y  = 0.0f;
        mIsCollidingTop = true;
    }

    // COLLISION BELOW (falling downward)
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) && mVelocity.y > 0.0f)
    {
        mPosition.y -= yOverlap;   // push up
        mVelocity.y  = 0.0f;
        mIsCollidingBottom = true;
    } 
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    Vector2 leftCentreProbe   = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y };

    Vector2 rightCentreProbe  = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ON RIGHT (moving right)
    if (map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) 
         && mVelocity.x > 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x -= xOverlap * 1.01f;   // push left
        mVelocity.x  = 0.0f;
        mIsCollidingRight = true;
    }

    // COLLISION ON LEFT (moving left)
    if (map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) 
         && mVelocity.x < 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x += xOverlap * 1.01;   // push right
        mVelocity.x  = 0.0f;
        mIsCollidingLeft = true;
    }
}

bool Entity::isColliding(Entity *other) const 
{
    if (!other->isActive() || other == this) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) - 
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) - 
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Entity::animate(float deltaTime)
{
    mAnimationIndices = mAnimationAtlas.at(mDirection);
    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;
        mCurrentFrameIndex++;
        
        if (mEntityType == PLAYER) {
            if (mCurrentFrameIndex >= 24) mCurrentFrameIndex = 0; 
        } else {
            if (mAnimationIndices.size() > 0) mCurrentFrameIndex %= mAnimationIndices.size();
        }
    }
}

void Entity::AIWander()
{
    if (mIsCollidingLeft) mDirection = RIGHT;
    else if (mIsCollidingRight) mDirection = LEFT;

    if (mDirection == UP || mDirection == DOWN) mDirection = RIGHT;

    if (mDirection == RIGHT) moveRight();
    else                     moveLeft();
}

void Entity::AIActivate(Entity *target, float deltaTime) {
    if (mStunTimer > 0) { mStunTimer -= deltaTime; return; }
    
    // --- 1. MANUAL DISTANCE CALCULATION ---
    float diffX = target->getPosition().x - mPosition.x;
    float diffY = target->getPosition().y - mPosition.y;
    float d = sqrt(diffX * diffX + diffY * diffY);

    mGlobalTimer += deltaTime;
    if (mRCooldown > 0) mRCooldown -= deltaTime;

    // Movement
    bool isCurrentlyCasting = (mAIState == CAST_Q || mAIState == CAST_W || mAIState == CAST_E || mAIState == CAST_R);
    if (!isCurrentlyCasting) {
        if (d < 300.0f) { mAIState = FLEEING; AIFlee(target, deltaTime); }
        else if (d > 800.0f) { mAIState = WALKING; AIFollow(target); }
        else { mAIState = IDLE; resetMovement(); }
    }

    // Boss ability rotation
    if (mGlobalTimer >= mGlobalCooldown && !isCurrentlyCasting) {
        resetEnemyHit(); 
        if (mCurrentHP < 400 && mRCooldown <= 0) {
            mAIState = CAST_R;
            mRShotsLeft = 3;
            mRShotTimer = 0;
        } else {
            int choice = GetRandomValue(0, 2);
            if (choice == 0) mAIState = CAST_Q;
            else if (choice == 1) mAIState = CAST_W;
            else mAIState = CAST_E;
        }
        mAbilityTimer = 0;
        mGlobalTimer = 0;
        mCastTargetPos = target->getPosition();
    }

    mAbilityTimer += deltaTime;

    if (mAIState == CAST_Q && mAbilityTimer > 1.6f) mAIState = IDLE;
    if (mAIState == CAST_W && mAbilityTimer > 1.1f) mAIState = IDLE;
    
    // 
    if (mAIState == CAST_E && mAbilityTimer > 0.5f) {
        mEActive = true; 
        mEPos = mPosition;
        

        float eDiffX = target->getPosition().x - mPosition.x;
        float eDiffY = target->getPosition().y - mPosition.y;
        float eDist  = sqrt(eDiffX * eDiffX + eDiffY * eDiffY);

        if (eDist > 0) {
            // (Direction / Length) * Speed
            mEVel.x = (eDiffX / eDist) * 600.0f;
            mEVel.y = (eDiffY / eDist) * 600.0f;
        }
        
        mAIState = IDLE;
    }

    if (mEActive) {
        mEPos.x += mEVel.x * deltaTime;
        mEPos.y += mEVel.y * deltaTime;

        // orb deactivation
        float orbX = mEPos.x - mPosition.x;
        float orbY = mEPos.y - mPosition.y;
        if (sqrt(orbX * orbX + orbY * orbY) > 1200.0f) {
            mEActive = false;
        }
    }

    // Shoots 3 rings at player
    if (mAIState == CAST_R) {
        mRShotTimer += deltaTime;
        if (mRShotTimer >= 0.6f) { 
            resetEnemyHit(); 
            mCastTargetPos = target->getPosition(); 
            mRShotsLeft--;
            mRShotTimer = 0.0f;

            if (mRShotsLeft <= 0) { 
                mAIState = IDLE; 
                mRCooldown = 50.0f; 
            }
        }
    }
}

void Entity::AILerp(Entity *target, float deltaTime)
{
    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 250.0f) {
            mAIState = WALKING; 
        }
        break;

    case WALKING: 
    {   
        
        float lerpSpeed = .5f; 

        mPosition.x += (target->getPosition().x - mPosition.x) * lerpSpeed * deltaTime;
        mPosition.y += (target->getPosition().y - mPosition.y) * lerpSpeed * deltaTime;

        if (mPosition.x > target->getPosition().x) mDirection = LEFT;
        else                                       mDirection = RIGHT;
        
        break;
        
    }   
    
    default:
        break;
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map, 
    Entity *collidableEntities, int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;
    
    if (mBlindTimer > 0) mBlindTimer -= deltaTime;

    resetColliderFlags();

    mVelocity.x = mMovement.x * mSpeed;
    mVelocity.y = mMovement.y * mSpeed; 

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;


    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);
    checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);
    checkCollisionX(map);

    bool isCasting = (mQTimer > 0 || mWTimer > 0 || mETimer > 0 || mRTimer > 0);
    bool isMoving = (Vector2Length(mMovement) != 0);

    if (mTextureType == ATLAS && (isMoving || isCasting)) 
    {
        animate(deltaTime);
    }
}

void Entity::render()
{
    if (mEntityStatus == INACTIVE) return;

    Texture2D* activeTex = &mTexture; 
    Vector2 currentDims = mSpriteSheetDimensions;

    if (mEntityType == PLAYER) {
        if (mRTimer > 0)      { activeTex = &mRTex; currentDims = {4, 1}; } // Attack1
        else if (mETimer > 0) { activeTex = &mETex; currentDims = {4, 1}; } // Attack2
        else if (mWTimer > 0) { activeTex = &mWTex; currentDims = {6, 1}; } // Heal
        else if (mQTimer > 0) { activeTex = &mQTex; currentDims = {4, 1}; } // Attack1
        else if (Vector2Length(mMovement) > 0) { 
            activeTex = &mRunTex;  
            currentDims = {6, 1}; 
        }
        else { 
            activeTex = &mIdleTex; 
            currentDims = {8, 1}; 
        }
    }

    // Calculate frame
    int totalFrames = (int)currentDims.x;
    int frame = (totalFrames > 0) ? (mCurrentFrameIndex % totalFrames) : 0;

    Rectangle textureArea;
    if (mEntityType == PLAYER) {
        textureArea = getUVRectangle(activeTex, frame, (int)currentDims.y, (int)currentDims.x);
    } else {
        if (mAnimationIndices.size() > 0) {
            int npcFrame = mAnimationIndices[mCurrentFrameIndex % mAnimationIndices.size()];
            textureArea = getUVRectangle(&mTexture, npcFrame, mSpriteSheetDimensions.y, mSpriteSheetDimensions.x);
        }
    }

    if (mDirection == LEFT) textureArea.width *= -1.0f;

    // Where to draw
    Rectangle destinationArea = { mPosition.x, mPosition.y, (float)mScale.x, (float)mScale.y };
    Vector2 originOffset = { (float)mScale.x / 2.0f, (float)mScale.y / 2.0f };

    DrawTexturePro(*activeTex, textureArea, destinationArea, originOffset, mAngle, WHITE);

    // Ability Hud
    float barW = mScale.x * 0.8f;
    float barH = 10.0f;
    Vector2 barPos = { mPosition.x - barW / 2, mPosition.y - mScale.y / 2 - 20 };

    if (mAIState == CAST_Q) {
        float diffX = mCastTargetPos.x - mPosition.x;
        float diffY = mCastTargetPos.y - mPosition.y;

        float length = sqrt(diffX * diffX + diffY * diffY);

        if (length > 0) {
            float unitX = diffX / length;
            float unitY = diffY / length;
            
            float lineEndX = mPosition.x + (unitX * 1000.0f);
            float lineEndY = mPosition.y + (unitY * 1000.0f);

            Vector2 startPos = mPosition;
            Vector2 endPos   = { lineEndX, lineEndY };
            float thickness  = 20.0f * (mAbilityTimer / 1.5f);

            DrawLineEx(startPos, endPos, thickness, ColorAlpha(BLUE, 0.5f));
        }
    }
    else if (mAIState == CAST_W) {
        float chargePercent = mAbilityTimer / 1.0f;
        DrawCircleV(mCastTargetPos, 120, ColorAlpha(BLUE, 0.2f));
        DrawCircleLinesV(mCastTargetPos, 120 * chargePercent, BLUE);
    }
    else if (mAIState == CAST_R) {
        float radius = 180.0f;
        float progress = mRShotTimer / 0.6f; 
        DrawCircleV(mCastTargetPos, radius, ColorAlpha(RED, 0.2f));
        DrawCircleLinesV(mCastTargetPos, radius * progress, RED);
    }

    if (mEActive) DrawCircleV(mEPos, 15, PURPLE);

    if (mEntityType == NPC) {
        // Draw Aggro Range
        DrawCircleLinesV(mPosition, mAggroRange, ColorAlpha(YELLOW, 0.8f));
        // Tower Range
        if (mScale.y > 200) DrawCircleLinesV(mPosition, 600.0f, ColorAlpha(RED, 0.8f));
    }

    // HP Bar Drawing
    DrawRectangle(barPos.x, barPos.y, barW, barH, BLACK);
    float hpPercent = mCurrentHP / mMaxHP;
    DrawRectangle(barPos.x, barPos.y, barW * hpPercent, barH, GREEN);
    DrawRectangleLines(barPos.x, barPos.y, barW, barH, RAYWHITE);
}

void Entity::displayCollider() 
{
    // draw the collision box
    Rectangle colliderBox = {
        mPosition.x - mColliderDimensions.x / 2.0f,  
        mPosition.y - mColliderDimensions.y / 2.0f,  
        mColliderDimensions.x,                        
        mColliderDimensions.y                        
    };

    DrawRectangleLines(
        colliderBox.x,      // Top-left X
        colliderBox.y,      // Top-left Y
        colliderBox.width,  // Width
        colliderBox.height, // Height
        GREEN               // Color
    );
}

void Entity::AIFlee(Entity *target, float deltaTime) {
    if (mPosition.x > target->getPosition().x) moveRight(); // Flee opposite X
    else moveLeft();

    if (mPosition.y > target->getPosition().y) moveDown();  // Flee opposite Y
    else moveUp();
}
void Entity::Stop() {
    mMovement = { 0.0f, 0.0f };
    mVelocity = { 0.0f, 0.0f };
}


void Entity::takeDamage(float amount) {
    float finalDamage = amount - mDamageReduction;
    if (finalDamage < 0) finalDamage = 0; 
    
    mCurrentHP -= finalDamage; 
    if (mCurrentHP < 0) mCurrentHP = 0; 
}

void Entity::AILeashMonster(Entity *target, float deltaTime) {
    float distToPlayer = Vector2Distance(mPosition, target->getPosition());
    float distToSpawn  = Vector2Distance(mPosition, mSpawnPos);
    
    setAIState(IDLE); 

    if (distToSpawn > mLeashRange) mIsResetting = true;

    if (mIsResetting) {
        // Reset back to the spawn
        if (mPosition.x > mSpawnPos.x + 5.0f) moveLeft();
        else if (mPosition.x < mSpawnPos.x - 5.0f) moveRight();
        
        if (mPosition.y > mSpawnPos.y + 5.0f) moveUp();
        else if (mPosition.y < mSpawnPos.y - 5.0f) moveDown();

        // Heal when deaggrroed
        if (distToSpawn < 10.0f) {
            mIsResetting = false;
            mCurrentHP = mMaxHP;
            resetMovement();
            Stop();
        }
        return; 
    }

    if (distToPlayer < mAggroRange) {
        // Aggro
        if (mPosition.x > target->getPosition().x + 5.0f) moveLeft();
        else if (mPosition.x < target->getPosition().x - 5.0f) moveRight();

        if (mPosition.y > target->getPosition().y + 5.0f) moveUp();
        else if (mPosition.y < target->getPosition().y - 5.0f) moveDown();
        
        mAbilityTimer += deltaTime; 
    } else {
        // Return home if player leaves range
        if (distToSpawn > 10.0f) {
            if (mPosition.x > mSpawnPos.x + 5.0f) moveLeft();
            else if (mPosition.x < mSpawnPos.x - 5.0f) moveRight();
            if (mPosition.y > mSpawnPos.y + 5.0f) moveUp();
            else if (mPosition.y < mSpawnPos.y - 5.0f) moveDown();
        } else {
            resetMovement();
        }
    }
}

void Entity::AIFollow(Entity *target) {
    if (mPosition.x > target->getPosition().x + 5.0f) moveLeft();
    else if (mPosition.x < target->getPosition().x - 5.0f) moveRight();

    if (mPosition.y > target->getPosition().y + 5.0f) moveUp();
    else if (mPosition.y < target->getPosition().y - 5.0f) moveDown();
}


void Entity::AIHomingProjectile(Entity *target, float speed, float deltaTime) {
    if (target == nullptr || !target->isActive()) {
        this->deactivate(); 
        return;
    }

    // Distance
    float diffX = target->getPosition().x - mPosition.x;
    float diffY = target->getPosition().y - mPosition.y;

    // Total distance
    float distance = sqrt(diffX * diffX + diffY * diffY);

    if (distance > 0) {
        float unitX = diffX / distance;
        float unitY = diffY / distance;

        // Apply movement
        mPosition.x += unitX * speed * deltaTime;
        mPosition.y += unitY * speed * deltaTime;

        // geometry :(
        float pi = 3.14;
        float rad_to_deg = 180.0f / pi;
        
        // Prevent crash
        if (unitX > 1.0f) unitX = 1.0f;
        if (unitX < -1.0f) unitX = -1.0f;

        float angleInRadians = acos(unitX);

        // If unitY is negative, we negate the angle.
        if (unitY < 0) {
            angleInRadians = -angleInRadians;
        }

        mAngle = angleInRadians * rad_to_deg;
    }
}
void Entity::loadWarriorSprites(const char* idle, const char* run, const char* q, const char* w, const char* e, const char* r) 
{
    mIdleTex = LoadTexture(idle);
    mRunTex  = LoadTexture(run);
    mQTex    = LoadTexture(q);
    mWTex    = LoadTexture(w);
    mETex    = LoadTexture(e);
    mRTex    = LoadTexture(r);
}
void Entity::syncStatsFrom(Entity* other) {
    if (!other) return;
    
    // Copy Health
    this->mMaxHP = other->getMaxHP();
    this->mCurrentHP = other->getCurrentHP();
    
    // Copy Economy
    this->mGold = other->getGold();
    
    // Copy Items and Levels
    this->mWeaponLevel = other->getWeaponLevel();
    this->mArmorLevel = other->getArmorLevel();
    this->mHealthLevel = other->getHealthLevel();
    
    // Copy Combat Multipliers
    this->mBonusDamage = other->getBonusDamage();
}
