#include "TrackSelection.h"

TrackSelection::TrackSelection()
    : Scene({0.0f}, nullptr) {}

TrackSelection::TrackSelection(Vector2 origin, const char *bgHexCode)
    : Scene(origin, bgHexCode) {}

TrackSelection::~TrackSelection() {
    shutdown();
}

void TrackSelection::initialise() {
    mGameState.nextSceneID = -1;
    mGameState.gameMode = -1; // Don't override global game mode
    mGameState.map = nullptr;
    mGameState.player = nullptr;
    mGameState.AI = nullptr;
}

void TrackSelection::update(float deltaTime) {
    // Check for track selection (1, 2, 3 keys)
    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {
        mGameState.nextSceneID = 2; // go to TrackOne
    }

    if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
        mGameState.nextSceneID = 3; // go to TrackTwo
    }

    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {
        mGameState.nextSceneID = 4; // go to TrackThree (future)
    }

    // BACKSPACE to go back to main menu
    if (IsKeyPressed(KEY_BACKSPACE)) {
        mGameState.nextSceneID = 0;
    }
}

void TrackSelection::render() {
    ClearBackground(ColorFromHex(mBGColourHexCode));

    // Title
    DrawText("Select Track", 450, 150, 60, WHITE);

    // Track options
    DrawText("Press 1 for Track One", 450, 300, 30, WHITE);
    DrawText("Press 2 for Track Two", 450, 350, 30, WHITE);
    DrawText("Press 3 for Track Three", 450, 400, 30, WHITE);

    // Back option
    DrawText("Controls:", 250, 550, 30, WHITE);
    DrawText("BACKSPACE to return to Main Menu", 250, 600, 20, GRAY);
    DrawText("WASD to Drive", 250, 630, 20, GRAY);
}

void TrackSelection::shutdown() {

}
