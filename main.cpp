#include "CS3113/LevelA.h"
#include "CS3113/LevelB.h"
#include "CS3113/LevelC.h"
#include "CS3113/MenuLevel.h"
#include "CS3113/WinLevel.h"
#include "CS3113/LoseLevel.h"
#include "CS3113/TutorialLevel.h"

// Global Constants
constexpr int SCREEN_WIDTH = 1920,
              SCREEN_HEIGHT = 1080,
              FPS = 120;

constexpr Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f,
      gTimeAccumulator = 0.0f;

Camera2D gCamera = {0};
Scene *gCurrentScene = nullptr;
Vector2 gCurrentMouse = {0};

Scene *scenes[7];
int currentSceneIndex = 0;

Sound sfxStepsLth[4];
Sound sfxStepsMetal[4];
Sound sfxBoom;
Sound sfxHeal;
Sound sfxSpin; 

float footstepTimer = 0.0f;
Music gBackgroundMusic;
static Vector2 targetPos = { 0, 0 };
static bool isMoving = false;
float speed = 5.0f; 

Shader gBlindShader;
int playerPosLoc;
int isBlindedLoc;
RenderTexture2D gScreenTarget;
bool gShowShop = false;

// Function Prototypes
void initialise();
void processInput();
void update();
void render();
void shutdown();

int main(void)
{
    initialise();
    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }
    shutdown();
    return 0;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "League of Trials");
    InitAudioDevice();

    gBackgroundMusic = LoadMusicStream("assets/audio/New age.wav");
    gBackgroundMusic.looping = true;
    PlayMusicStream(gBackgroundMusic);
    SetMusicVolume(gBackgroundMusic, 0.3f);

    sfxStepsLth[0] = LoadSound("assets/audio/walking/step_lth1.wav");
    sfxStepsLth[1] = LoadSound("assets/audio/walking/step_lth2.wav");
    sfxStepsLth[2] = LoadSound("assets/audio/walking/step_lth3.wav");
    sfxStepsLth[3] = LoadSound("assets/audio/walking/step_lth4.wav");

    sfxStepsMetal[0] = LoadSound("assets/audio/speed/step_metal.wav");
    sfxStepsMetal[1] = LoadSound("assets/audio/speed/step_metal2.wav");
    sfxStepsMetal[2] = LoadSound("assets/audio/speed/step_metal3.wav");
    sfxStepsMetal[3] = LoadSound("assets/audio/speed/step_metal4.wav");

    // Load Ability Sounds
    sfxBoom = LoadSound("assets/audio/boom.mp3");
    sfxHeal = LoadSound("assets/audio/heal.wav");
    sfxSpin = LoadSound("assets/audio/spinning.wav"); 

    // Set Volumes
    for(int i=0; i<4; i++) {
        SetSoundVolume(sfxStepsLth[i], 0.9f);
        SetSoundVolume(sfxStepsMetal[i], 0.9f);
    }
    SetSoundVolume(sfxBoom, 0.3f);
    SetSoundVolume(sfxHeal, 0.4f);
    SetSoundVolume(sfxSpin, 0.3f);


    // All 6 scene
    scenes[0] = new MenuLevel(ORIGIN, "#2c3e50");
    scenes[1] = new LevelA({0,0}, "#d69f96");
    scenes[2] = new LevelB({0,0}, "#d69f96");
    scenes[3] = new LevelC({0,0}, "#d69f96");
    scenes[4] = new WinLevel(ORIGIN, "#27ae60");
    scenes[5] = new LoseLevel(ORIGIN, "#c0392b");
    scenes[6] = new TutorialLevel({0,0}, "#000000"); 

    currentSceneIndex = 0;
    gCurrentScene = scenes[currentSceneIndex];
    gCurrentScene->initialise();

    if (gCurrentScene->getState().player != nullptr)
    {
        gCamera.target = gCurrentScene->getState().player->getPosition();
    }
    else
    {
        gCamera.target = ORIGIN; 
    }
    gBlindShader = LoadShader("assets/shaders/blind_vertex.glsl", "assets/shaders/blind_fragment.glsl");
    playerPosLoc = GetShaderLocation(gBlindShader, "playerPos");
    isBlindedLoc = GetShaderLocation(gBlindShader, "isBlinded");
    gCamera.offset = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom = 1.0f;
    gScreenTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTargetFPS(FPS);
}
void processInput()
{
    Entity* player = gCurrentScene->getState().player;
    if (IsKeyPressed(KEY_B)) gShowShop = !gShowShop;

    if (player != nullptr)
    {
        if (!gShowShop) {
            player->resetMovement();

            // Abilities
            if (IsKeyPressed(KEY_Q)) {
                player->triggerQ();
            }
            if (IsKeyPressed(KEY_W)) {
                if (player->getWCD() <= 0) { 
                    player->triggerW();
                    PlaySound(sfxHeal);
                }
            }
            if (IsKeyPressed(KEY_E)) {
                if (player->getECD() <= 0) { 
                    player->triggerE();
                    PlaySound(sfxSpin);
                }
            }
            
            if (IsKeyPressed(KEY_R))
            {
                if (player->getRCD() <= 0) {
                    bool rTargetInRange = false;
                    Entity** enemies = gCurrentScene->getState().enemies;
                    int enemyCount = gCurrentScene->getState().enemyCount;

                    if (enemies != nullptr)
                    {
                        for (int i = 0; i < enemyCount; i++)
                        {
                            Entity* e = enemies[i];
                            if (e != nullptr && e->isActive())
                            {
                                // Check if target is within the 200 unit ultimate range
                                float d = Vector2Distance(player->getPosition(), e->getPosition());
                                if (d < 200.0f)
                                {
                                    rTargetInRange = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (rTargetInRange) {
                        player->triggerR();
                        PlaySound(sfxBoom);
                    }
                }
            }
        }
    }

    if (IsKeyPressed(KEY_ONE)) gCurrentScene->setNextSceneID(1);
    if (IsKeyPressed(KEY_TWO)) gCurrentScene->setNextSceneID(2);
    if (IsKeyPressed(KEY_THREE)) gCurrentScene->setNextSceneID(3);
    if (IsKeyPressed(KEY_ZERO)) gCurrentScene->setNextSceneID(0);
    if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) {
        gAppStatus = TERMINATED;
    }
}

void update()
{
    UpdateMusicStream(gBackgroundMusic);

    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    deltaTime += gTimeAccumulator;

    Vector2 currentPlayerPosition;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }
    if (gShowShop) {
        gTimeAccumulator = 0; // Stops Time
        return; 
    }
    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        if (gCurrentScene->getState().player != nullptr)
        {
            currentPlayerPosition = gCurrentScene->getState().player->getPosition();
            panCamera(&gCamera, &currentPlayerPosition);
        }
    }
    
    // Switch Scene
   if (gCurrentScene->getState().nextSceneID != -1)
    {
        int nextID = gCurrentScene->getState().nextSceneID;

        Entity* oldPlayer = gCurrentScene->getState().player;
        float sMaxHP = 700, sCurHP = 700, sBonus = 0;
        int sGold = 0, sWep = 0, sArm = 0, sHlt = 0;

        if (oldPlayer != nullptr) {
            sMaxHP = oldPlayer->getMaxHP();
            sCurHP = oldPlayer->getCurrentHP();
            sGold  = oldPlayer->getGold();
            sWep   = oldPlayer->getWeaponLevel();
            sArm   = oldPlayer->getArmorLevel();
            sHlt   = oldPlayer->getHealthLevel();
            sBonus = oldPlayer->getBonusDamage();
        }

        gCurrentScene->shutdown();
        currentSceneIndex = nextID;
        gCurrentScene = scenes[currentSceneIndex];
        gCurrentScene->initialise();


        Entity* newPlayer = gCurrentScene->getState().player;
        
        if (newPlayer != nullptr && oldPlayer != nullptr && nextID != 0 && nextID != 5) 
        {
            newPlayer->setHP(sMaxHP);         // Sets Max
            newPlayer->setCurrentHP(sCurHP);   // Sets Current
            newPlayer->setGold(sGold);
            newPlayer->setLevels(sWep, sArm, sHlt, sBonus);
        }
    }
    Entity *player = gCurrentScene->getState().player;
    if (player != nullptr) 
    {
        Vector2 movement = player->getMovement();

        float movementSq = (movement.x * movement.x) + (movement.y * movement.y);
        bool isActuallyMoving = (movementSq > 0.1f);

        // REMOVED: isCollidingBottom() - not needed for top-down games
        if (isActuallyMoving)
        {
            footstepTimer += FIXED_TIMESTEP;
            
            // Sync interval to Garen's speed
            float interval = (player->getQCD() > 0) ? 0.22f : 0.32f;

            if (footstepTimer >= interval) {    
                if (player->getQCD() > 0) {
                    PlaySound(sfxStepsMetal[GetRandomValue(0, 3)]);
                } else {
                    PlaySound(sfxStepsLth[GetRandomValue(0, 3)]);
                }
                footstepTimer = 0.0f;
            }
        }
        else 
        {
            // Reset so the first step of a new move starts immediately
            footstepTimer = 0.5f; 
        }
    }
}
void render()
{
    BeginDrawing();
    ClearBackground(BLACK); 

    Entity *player = gCurrentScene->getState().player;

    if (player == nullptr) {
        gCurrentScene->render();
    } else {
        //  Abilities HUD
        int boxSize = 80, padding = 20;
        int startX = (SCREEN_WIDTH - (4 * boxSize + 3 * padding)) / 2;
        int startY = SCREEN_HEIGHT - 120;
        float ranges[] = { 80.0f, 0.0f, 150.0f, 200.0f };


        BeginTextureMode(gScreenTarget);
        ClearBackground(RAYWHITE); 
        BeginMode2D(gCamera);
        gCurrentScene->render();
        
        // Draw range circles around player
        for (int i = 0; i < 4; i++) {
            Rectangle iconRec = { (float)(startX + i * (boxSize + padding)), (float)startY, (float)boxSize, (float)boxSize };
            if (CheckCollisionPointRec(GetMousePosition(), iconRec) && ranges[i] > 0) {
                DrawCircleLinesV(player->getPosition(), ranges[i], ColorAlpha(YELLOW, 0.5f));
                DrawCircleV(player->getPosition(), ranges[i], ColorAlpha(YELLOW, 0.1f));
            }
        }
        EndMode2D(); 
        EndTextureMode();

        // Shader
        Vector2 sPos = GetWorldToScreen2D(player->getPosition(), gCamera);

        Vector2 normPos = { sPos.x / (float)SCREEN_WIDTH, sPos.y / (float)SCREEN_HEIGHT };
        int blinded = (player->getBlindTimer() > 0.0f) ? 1 : 0;

        SetShaderValue(gBlindShader, playerPosLoc, &normPos, SHADER_UNIFORM_VEC2);
        SetShaderValue(gBlindShader, isBlindedLoc, &blinded, SHADER_UNIFORM_INT);

        BeginShaderMode(gBlindShader);
            DrawTextureRec(gScreenTarget.texture, { 0, 0, (float)SCREEN_WIDTH, (float)-SCREEN_HEIGHT }, { 0, 0 }, WHITE);
        EndShaderMode();
        

        //Draws Abilities HUD
        const char* labels[] = { "Q", "W", "E", "R" };
        float cds[] = { player->getQCD(), player->getWCD(), player->getECD(), player->getRCD() };
        float maxs[] = { 8.0f, 20.0f, 9.0f, 60.0f };

        for (int i = 0; i < 4; i++) {
            int xPos = startX + i * (boxSize + padding);
            DrawRectangle(xPos, startY, boxSize, boxSize, DARKGRAY);
            DrawRectangleLines(xPos, startY, boxSize, boxSize, RAYWHITE);
            //Based on the CD cooldown, CD is incremented by deltaTime
            if (cds[i] > 0) {
                float pct = cds[i] / maxs[i];
                DrawRectangle(xPos, startY + (boxSize - (int)(boxSize * pct)), boxSize, (int)(boxSize * pct), ColorAlpha(BLACK, 0.6f));
                DrawText(TextFormat("%.1f", cds[i]), xPos + 15, startY + 30, 25, YELLOW);
            }
            DrawText(labels[i], xPos + 5, startY + 5, 20, WHITE);
        }

        if (player->getBlindTimer() > 0) DrawText("BLINDED!", SCREEN_WIDTH/2 - 100, 150, 50, RED);
    }
    //Shop
    if (gShowShop) {
        int sWidth = 600, sHeight = 550;
        DrawRectangle(SCREEN_WIDTH/2 - 300, 200, sWidth, sHeight, ColorAlpha(BLACK, 0.9f));
        DrawRectangleLines(SCREEN_WIDTH/2 - 300, 200, sWidth, sHeight, GOLD);
        DrawText("SHOP (Press B to Close)", SCREEN_WIDTH/2 - 180, 220, 30, YELLOW);
        DrawText(TextFormat("GOLD: %i", player->getGold()), SCREEN_WIDTH/2 - 60, 265, 25, GOLD);

        Rectangle btnWep = { (float)SCREEN_WIDTH/2 - 250, 320, 500, 80 };
        Rectangle btnArm = { (float)SCREEN_WIDTH/2 - 250, 430, 500, 80 };
        Rectangle btnHlt = { (float)SCREEN_WIDTH/2 - 250, 540, 500, 80 };

        // Weapon Slot
        DrawRectangleRec(btnWep, DARKGRAY);
        const char* wepText = (player->getWeaponLevel() == 0) ? "Short Sword (150G)" : 
                            (player->getWeaponLevel() == 1) ? "Upgrade to Long Sword (400G)" : "Weapon Maxed";
        DrawText(wepText, btnWep.x + 20, btnWep.y + 25, 25, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), btnWep) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) player->buyWeapon();

        // Armor Slot
        DrawRectangleRec(btnArm, DARKGRAY);
        const char* armText = (player->getArmorLevel() == 0) ? "Buy Chestplate (150G)" : 
                            (player->getArmorLevel() == 1) ? "Upgrade to Armor Set (350G)" : "Armor Maxed";
        DrawText(armText, btnArm.x + 20, btnArm.y + 25, 25, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), btnArm) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) player->buyArmor();

        // Health Slot
        DrawRectangleRec(btnHlt, DARKGRAY);
        const char* hltText = (player->getHealthLevel() == 0) ? "Buy Hearts (100G)" : 
                            (player->getHealthLevel() == 1) ? "Upgrade to Vitality (300G)" : "Health Maxed";
        DrawText(hltText, btnHlt.x + 20, btnHlt.y + 25, 25, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), btnHlt) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) player->buyHealth();
    }
    EndDrawing();
}
void shutdown()
{
    UnloadMusicStream(gBackgroundMusic);
    UnloadShader(gBlindShader); 
    UnloadSound(sfxBoom);
    UnloadSound(sfxHeal);
    UnloadSound(sfxSpin);
    for (int i = 0; i < 4; i++) {
        UnloadSound(sfxStepsLth[i]);
        UnloadSound(sfxStepsMetal[i]);
    }


    for (int i = 0; i < 7; i++)
    {
        if (scenes[i] != nullptr)
        {
            delete scenes[i];
            scenes[i] = nullptr;
        }
    }

    CloseAudioDevice();
    CloseWindow();
}