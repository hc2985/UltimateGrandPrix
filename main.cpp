#include "CS3113/trackone.h"
#include "CS3113/tracktwo.h"
#include "CS3113/trackthree.h"
#include "CS3113/StartMenu.h"
#include "CS3113/TrackSelection.h"

// Screen configuration
constexpr int SCREEN_WIDTH  = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int FPS           = 120;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks = 0.0f;
float gTimeAccumulator   = 0.0f;

Scene* gCurrentScene = nullptr;
std::vector<Scene*> gScenes = {};
int gCurrentSceneID = 0;
int gGameMode = 0; // Persistent game mode across scenes

StartMenu* gStartMenu = nullptr;
TrackSelection* gTrackSelection = nullptr;
TrackOne* gTrackOne = nullptr;
TrackTwo* gTrackTwo = nullptr;
TrackThree* gTrackThree = nullptr;

// Shared audio resources
Music gBgm1;
Music gBgm2;
Music gBgm3;
Sound gWinSound;
Sound gLoseSound;
Sound gPingSound;

// Forward declarations
void initGame();
void processInput(float dt);
void update();
void render();
void shutdownGame();
void switchToScene(Scene* scene);

void initGame()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ultimate Grand Prix");
    InitAudioDevice();
    SetTargetFPS(FPS);

    // Load common audio once
    gBgm1 = LoadMusicStream("assets/game/song.mp3");
    gBgm2 = LoadMusicStream("assets/game/song2.mp3");
    gBgm3 = LoadMusicStream("assets/game/song3.mp3");
    gWinSound = LoadSound("assets/game/win.mp3");
    gLoseSound = LoadSound("assets/game/lose.mp3");
    gPingSound = LoadSound("assets/game/ping.mp3");

    // Create all scenes
    gStartMenu = new StartMenu(ORIGIN, "#1a1a1aff");
    gTrackSelection = new TrackSelection(ORIGIN, "#1a1a1aff");
    gTrackOne = new TrackOne(ORIGIN, "#315c15ff");
    gTrackTwo = new TrackTwo(ORIGIN, "#206e41ff");
    gTrackThree = new TrackThree(ORIGIN, "#4a7c59ff");

    gScenes.push_back(gStartMenu);      // ID 0 - Main Menu
    gScenes.push_back(gTrackSelection); // ID 1 - Track Selection
    gScenes.push_back(gTrackOne);       // ID 2 - Track One
    gScenes.push_back(gTrackTwo);       // ID 3 - Track Two
    gScenes.push_back(gTrackThree);     // ID 4 - Track Three

    // Set audio for all track scenes
    for (int i = 2; i < gScenes.size(); i++) {
        gScenes[i]->getState().bgm1 = gBgm1;
        gScenes[i]->getState().bgm2 = gBgm2;
        gScenes[i]->getState().bgm3 = gBgm3;
        gScenes[i]->getState().winSound = gWinSound;
        gScenes[i]->getState().loseSound = gLoseSound;
        gScenes[i]->getState().pingSound = gPingSound;
    }

    // Start with the menu
    gCurrentSceneID = 0;
    gCurrentScene = gStartMenu;
    gCurrentScene->initialise();
}

void processInput(float dt)
{
    if (!gCurrentScene) return;

    if (WindowShouldClose() || IsKeyPressed(KEY_Q))
        gAppStatus = TERMINATED;

    if(gCurrentSceneID > 1){
        if (IsKeyDown(KEY_W))
        gCurrentScene->getState().player->accelerate(dt, gCurrentScene->getState().map);

        if (IsKeyDown(KEY_A))
            gCurrentScene->getState().player->turnleft(dt);

        if (IsKeyDown(KEY_D))
            gCurrentScene->getState().player->turnright(dt);

        if (IsKeyDown(KEY_S)){
            if( gCurrentScene->getState().player->getForwardSpeed() > 1){
                gCurrentScene->getState().player->brake(dt);
            } else {
                gCurrentScene->getState().player->reverse(dt); 
            }
            
        }
    }
    
}
        

void switchToScene(Scene* scene)
{
    if (gCurrentScene)
    {
        gCurrentScene->shutdown();
    }

    gCurrentScene = scene;
    gCurrentScene->initialise();
}

void update()
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        processInput(FIXED_TIMESTEP);
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;

    // Check for scene transitions
    int nextSceneID = gCurrentScene->getState().nextSceneID;
    if (nextSceneID >= 0 && nextSceneID != gCurrentSceneID)
    {
        // Update global game mode if the scene set it
        if (gCurrentScene->getState().gameMode >= 0) {
            gGameMode = gCurrentScene->getState().gameMode;
        }

        gCurrentSceneID = nextSceneID;

        // Set game mode for TrackOne if transitioning to it
        if (gCurrentSceneID == 2)
        {
            gTrackOne->setGameMode(gGameMode);
        }

        // Set game mode for TrackTwo if transitioning to it
        if (gCurrentSceneID == 3)
        {
            gTrackTwo->setGameMode(gGameMode);
        }

        // Set game mode for TrackThree if transitioning to it
        if (gCurrentSceneID == 4)
        {
            gTrackThree->setGameMode(gGameMode);
        }

        switchToScene(gScenes[gCurrentSceneID]);
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);

    gCurrentScene->render();

    EndDrawing();
}

void shutdownGame()
{
    // Clean up all scenes
    for (Scene* scene : gScenes)
    {
        delete scene;
    }
    gScenes.clear();

    gCurrentScene = nullptr;
    gStartMenu = nullptr;
    gTrackSelection = nullptr;
    gTrackOne = nullptr;
    gTrackTwo = nullptr;

    // Unload shared audio resources
    UnloadMusicStream(gBgm1);
    UnloadMusicStream(gBgm2);
    UnloadMusicStream(gBgm3);
    UnloadSound(gWinSound);
    UnloadSound(gLoseSound);
    UnloadSound(gPingSound);

    CloseAudioDevice();
    CloseWindow();
}

int main()
{
    initGame();

    while (gAppStatus == RUNNING) {
        update();
        render();
    }

    shutdownGame();
    return 0;
}
