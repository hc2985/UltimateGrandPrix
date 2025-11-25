#include "StartMenu.h"

StartMenu::StartMenu()
    : Scene({0.0f}, nullptr) {}

StartMenu::StartMenu(Vector2 origin, const char *bgHexCode)
    : Scene(origin, bgHexCode) {}

StartMenu::~StartMenu() {
    shutdown();
}

void StartMenu::initialise() {
    mGameState.nextSceneID = -1;
    mGameState.map = nullptr;
    mGameState.player = nullptr;
    mGameState.AI = nullptr;
}

void StartMenu::update(float deltaTime) {
    // Check for hotlap mode
    if (IsKeyPressed(KEY_ZERO)) {
        mGameState.nextSceneID = 1; // go to TrackSelection
        mGameState.gameMode = 0; // hotlap mode
    }

    // Check for race mode 
    if (IsKeyPressed(KEY_ONE)) {
        mGameState.nextSceneID = 1; // go to TrackSelection
        mGameState.gameMode = 1; // race mode
    }
}

void StartMenu::render() {
    ClearBackground(ColorFromHex(mBGColourHexCode));

    DrawText("Ultimate Grand Prix", 350, 300, 60, WHITE);

    DrawText("Press 0 for Hotlap Mode", 450, 450, 30, WHITE);
    DrawText("Press 1 for Quick Race", 450, 500, 30, WHITE);
}


void StartMenu::shutdown() {

}