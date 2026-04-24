#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { LEFT, UP, RIGHT, DOWN              }; // For walking
enum EntityStatus { ACTIVE, INACTIVE                   };
enum EntityType   { PLAYER, BLOCK, PLATFORM, NPC, NONE };
enum AIType       { WANDERER, FOLLOWER, LERPER         };
enum AIState { WALKING, IDLE, FOLLOWING, FLEEING, CAST_Q, CAST_W, CAST_E, CAST_R };

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mAcceleration;

    Vector2 mScale;
    Vector2 mColliderDimensions;
    
    Texture2D mTexture;
    TextureType mTextureType;
    Vector2 mSpriteSheetDimensions;
    
    std::map<Direction, std::vector<int>> mAnimationAtlas;
    std::vector<int> mAnimationIndices;
    Direction mDirection;
    int mFrameSpeed;

    int mCurrentFrameIndex = 0;
    float mAnimationTime = 0.0f;

    bool mIsJumping = false;
    float mJumpingPower = 0.0f;

    int mSpeed;
    float mAngle;

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType  mAIType;
    AIState mAIState;

    void checkCollisionY(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionY(Map *map);

    void checkCollisionX(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionX(Map *map);
    
    void resetColliderFlags() 
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);
    void AIWander();
    void AIFollow(Entity *target);
    void AILerp(Entity *target, float deltaTime);
    void Stop();

    float mQTimer = 0.0f, mQCooldown = 0.0f;
    float mWTimer = 0.0f, mWCooldown = 0.0f;
    float mETimer = 0.0f, mECooldown = 0.0f;
    float mRTimer = 0.0f, mRCooldown = 0.0f;
    bool mIsSpinning = false;
    bool mIsShielded = false;
    
    float mMaxHP = 100.0f;
    float mCurrentHP = 100.0f;

    float mEnemyAbilityTimer = 0.0f;
    float mEnemyAbilityCD = 3.0f;    
    float mCastDelay = 1.2f;         
    Vector2 mCastTargetPos = {0, 0};
    bool mIsCasting = false;

    float mGlobalCooldown = 2.0f; 
    float mGlobalTimer = 0.0f;
    
    // Ability Specifics
    float   mAbilityTimer = 0.0f;
    int     mRShotsLeft = 0;
    float   mRShotTimer = 0.0f; 
    
    // E Projectile
    Vector2 mEPos = {0,0}, mEVel = {0,0};
    bool    mEActive = false;
    float   mStunTimer = 0.0f;
    bool mEnemyHasHit = false;
    float mBlindTimer = 0.0f; 

    float mBonusDamage      = 0.0f;
    float mDamageReduction  = 0.0f;
    int   mGold             = 500;  
    int   mWeaponLevel      = 0;    
    int   mArmorLevel       = 0;    
    int   mHealthLevel      = 0;  

    Vector2 mSpawnPos;
    float   mAggroRange = 400.0f; 
    float   mLeashRange = 600.0f;
    bool    mIsResetting = false;

    void setTarget(Entity* target) { mTarget = target; }
    Texture2D mIdleTex, mRunTex, mQTex, mWTex, mETex, mRTex;
    
public:
    static constexpr int   DEFAULT_SIZE          = 250;
    static constexpr int   DEFAULT_SPEED         = 200;
    static constexpr int   DEFAULT_FRAME_SPEED   = 14;
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;

     bool isColliding(Entity *other) const;

    Entity();
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        EntityType entityType);
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, 
        std::map<Direction, std::vector<int>> animationAtlas, 
        EntityType entityType);
    ~Entity();

    void update(float deltaTime, Entity *player, Map *map, 
        Entity *collidableEntities, int collisionCheckCount);
    void render();
    void normaliseMovement() { Normalise(&mMovement); }

    void jump()       { mIsJumping = true;  }
    void activate()   { mEntityStatus  = ACTIVE;   }
    void deactivate() { mEntityStatus  = INACTIVE; }
    void displayCollider();

    bool isActive() { return mEntityStatus == ACTIVE ? true : false; }

    void moveUp()    { mMovement.y = -1; mDirection = UP;    }
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }
    void moveLeft()  { mMovement.x = -1; mDirection = LEFT;  }
    void moveRight() { mMovement.x =  1; mDirection = RIGHT; }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    Vector2     getPosition()              const { return mPosition;              }
    Vector2     getMovement()              const { return mMovement;              }
    Vector2     getVelocity()              const { return mVelocity;              }
    Vector2     getAcceleration()          const { return mAcceleration;          }
    Vector2     getScale()                 const { return mScale;                 }
    Vector2     getColliderDimensions()    const { return mScale;                 }
    Vector2     getSpriteSheetDimensions() const { return mSpriteSheetDimensions; }
    Texture2D   getTexture()               const { return mTexture;               }
    TextureType getTextureType()           const { return mTextureType;           }
    Direction   getDirection()             const { return mDirection;             }
    int         getFrameSpeed()            const { return mFrameSpeed;            }
    float       getJumpingPower()          const { return mJumpingPower;          }
    bool        isJumping()                const { return mIsJumping;             }
    int         getSpeed()                 const { return mSpeed;                 }
    float       getAngle()                 const { return mAngle;                 }
    EntityType  getEntityType()            const { return mEntityType;            }
    AIType      getAIType()                const { return mAIType;                }
    AIState     getAIState()               const { return mAIState;               }

    
    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }

    std::map<Direction, std::vector<int>> getAnimationAtlas() const { return mAnimationAtlas; }

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;                 }
    void setMovement(Vector2 newMovement)
        { mMovement = newMovement;                 }
    void setAcceleration(Vector2 newAcceleration)
        { mAcceleration = newAcceleration;         }
    void setScale(Vector2 newScale)
        { mScale = newScale;                       }
    void setTexture(const char *textureFilepath)
        { mTexture = LoadTexture(textureFilepath); }
    void setColliderDimensions(Vector2 newDimensions) 
        { mColliderDimensions = newDimensions;     }
    void setSpriteSheetDimensions(Vector2 newDimensions) 
        { mSpriteSheetDimensions = newDimensions;  }
    void setSpeed(int newSpeed)
        { mSpeed  = newSpeed;                      }
    void setFrameSpeed(int newSpeed)
        { mFrameSpeed = newSpeed;                  }
    void setJumpingPower(float newJumpingPower)
        { mJumpingPower = newJumpingPower;         }
    void setAngle(float newAngle) 
        { mAngle = newAngle;                       }
    void setEntityType(EntityType entityType)
        { mEntityType = entityType;                }
    void setDirection(Direction newDirection)
    { 
        if (mDirection != newDirection) {
            mDirection = newDirection;

            if (mTextureType == ATLAS) {
                mAnimationIndices = mAnimationAtlas.at(mDirection);
                mCurrentFrameIndex = 0; 
                mAnimationTime = 0.0f;
            }
        }
    }
    void setAIState(AIState newState)
        { mAIState = newState;                     }
    void setAIType(AIType newType)
        { mAIType = newType;                       }


    void triggerQ() { if (mQCooldown <= 0) { mQTimer = 3.5f; mQCooldown = 8.0f; } }
    void triggerW() { 
        if (mWCooldown <= 0) { 
            mWTimer = 4.0f; 
            mWCooldown = 20.0f; 
            mIsShielded = true; 
            heal(75.0f); 
        } 
    }
    void triggerE() { if (mECooldown <= 0) { mETimer = 3.0f; mECooldown = 9.0f; mIsSpinning = true; } }
    void triggerR() { if (mRCooldown <= 0) { mRTimer = 1.0f; mRCooldown = 60.0f; } }

    float getQCD() {return mQCooldown;}
    float getWCD() {return mWCooldown;}
    float getECD() {return mECooldown;}
    float getRCD() {return mRCooldown;}

    
    bool isSpinning() const { return mIsSpinning; }
    float getSpeedModifier() const { return (mQTimer > 0) ? 1.2f : 1.0f; } 

    void updateAbilityTimers(float deltaTime) {
        if (mQTimer > 0) mQTimer -= deltaTime;
        if (mWTimer > 0) mWTimer -= deltaTime; else mIsShielded = false;
        if (mETimer > 0) mETimer -= deltaTime; else mIsSpinning = false;
        if (mRTimer > 0) mRTimer -= deltaTime;

        if (mQCooldown > 0) mQCooldown -= deltaTime;
        if (mWCooldown > 0) mWCooldown -= deltaTime;
        if (mECooldown > 0) mECooldown -= deltaTime;
        if (mRCooldown > 0) mRCooldown -= deltaTime;

    };

    void  setHP(float max) { mMaxHP = max; mCurrentHP = max; }
    float getMaxHP() const { return mMaxHP; }
    float getCurrentHP() const { return mCurrentHP; }
    bool  isDead() const { return mCurrentHP <= 0; }
    void consumeQ() { mQTimer = 0.0f; }

    bool isRActive() const { return mRTimer > 0.0f; }
    
    // Stops the R so it only hit once
    void consumeR() { mRTimer = 0.0f; }

    void AIFlee(Entity *target, float deltaTime);
    bool isCasting() const { return mIsCasting; }
    Vector2 getCastPos() const { return mCastTargetPos; }
    float getCastTimer() const { return (mAIState == CAST_R) ? mRShotTimer : mAbilityTimer; }

    bool    isStunned() const { return mStunTimer > 0; }
    void    stun(float duration) { mStunTimer = duration; Stop(); }
    Vector2 getEPos() const { return mEPos; }
    bool    isEActive() const { return mEActive; }
    void    deactivateE() { mEActive = false; }

    bool hasEnemyHit() const { return mEnemyHasHit; }
    void consumeEnemyHit()   { mEnemyHasHit = true;  }
    void resetEnemyHit()     { mEnemyHasHit = false; }

    void  applyBlind(float duration) { mBlindTimer = duration; }
    float getBlindTimer() const { return mBlindTimer; }
    int   getGold()        const { return mGold; }
    int   getWeaponLevel() const { return mWeaponLevel; }
    int   getArmorLevel()  const { return mArmorLevel; }
    int   getHealthLevel() const { return mHealthLevel; }
    float getBonusDamage() const { return mBonusDamage; }

    void takeDamage(float amount); 
    void heal(float amount) {
        mCurrentHP += amount;
        if (mCurrentHP > mMaxHP) mCurrentHP = mMaxHP;
    }

    bool buyWeapon() {
        if (mWeaponLevel == 0 && mGold >= 150) {
            mWeaponLevel = 1; mBonusDamage = 20.0f; mGold -= 150; return true;
        }
        if (mWeaponLevel == 1 && mGold >= 400) {
            mWeaponLevel = 2; mBonusDamage = 50.0f; mGold -= 400; return true;
        }
        return false;
    }

    bool buyArmor() {
        if (mArmorLevel == 0 && mGold >= 150) {
            mArmorLevel = 1; mDamageReduction = 20.0f; mGold -= 150; return true;
        }
        if (mArmorLevel == 1 && mGold >= 350) {
            mArmorLevel = 2; mDamageReduction = 50.0f; mGold -= 350; return true;
        }
        return false;
    }

    bool buyHealth() {
        if (mHealthLevel == 0 && mGold >= 100) {
            mHealthLevel = 1; mMaxHP += 150; heal(150); mGold -= 100; return true;
        }
        if (mHealthLevel == 1 && mGold >= 300) {
            mHealthLevel = 2; mMaxHP += 400; heal(400); mGold -= 300; return true;
        }
        return false;
    }
    void setSpawn(Vector2 pos) { mSpawnPos = pos; }
    void AILeashMonster(Entity *target, float deltaTime);

    void AIActivate(Entity *target, float deltaTime);
    //Monster Ability Reset
    void resetAbilityTimer() { mAbilityTimer = 0.0f; }

    Entity* mTarget = nullptr;
    float mTowerRange = 600.0f;
    float mTowerCooldown = 0.0f;
    void AIHomingProjectile(Entity *target, float speed, float deltaTime);
    void addGold(int amount) { mGold += amount; }

    void loadWarriorSprites(const char* idle, const char* run, const char* q, const char* w, const char* e, const char* r);
    void syncStatsFrom(Entity* other);
    void setGold(int amount) { mGold = amount; }
    void setLevels(int wep, int arm, int hlt, float bonus) {
        mWeaponLevel = wep; mArmorLevel = arm; mHealthLevel = hlt; mBonusDamage = bonus;
    }
    void setCurrentHP(float hp) { mCurrentHP = hp; }
};
#endif // ENTITY_H