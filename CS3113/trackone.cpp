#include "trackone.h"
#include "car_profiles.h"

TrackOne::TrackOne() : Scene{{0.0f, 0.0f}, nullptr} {
    mGameMode = 0;
}
TrackOne::TrackOne(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode } {
    mGameMode = 0;
}
TrackOne::TrackOne(Vector2 origin, const char *bgHexCode, int gameMode)
    : Scene{ origin, bgHexCode } {
    mGameMode = gameMode;
}

TrackOne::~TrackOne() {
    shutdown();
}

void TrackOne::initialise() {
    mGameState.nextSceneID = -1;  

    /*
        ----------- Audio -----------
    */
    SetMusicVolume(mGameState.bgm1, 0.33f);
    PlayMusicStream(mGameState.bgm1);
    mResultSoundPlayed = false;

    /*
        ----------- Shader -----------
    */
    mVignetteShader = LoadShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    mLightPositionLoc = GetShaderLocation(mVignetteShader, "lightPosition");

    //map setup

    mGameState.map = new Map(
        TRACK_WIDTH,
        TRACK_HEIGHT,
        (unsigned int*)mTrackData,
        "assets/track/tiles.png",
        TILE_SIZE,
        18, 18,
        mOrigin
    );

    mGameState.map->registerMultiTileObject(
        500,                               
        "assets/track/Objects/tribune_full.png",
        2, 1,                               
        { 0.0f, -32.0f },                   
        1.0f                               
    );
    mGameState.map->registerMultiTileObject(
        501,                               
        "assets/track/Objects/tribune_full.png",
        2, 1,                               
        { 0.0f, -32.0f },                   
        1.0f,
        90.0f                               
    );
    mGameState.map->registerMultiTileObject(
        502,                               
        "assets/track/Objects/tribune_full.png",
        2, 1,                               
        { 0.0f, -32.0f },                   
        1.0f,
        180.0f                               
    );
    mGameState.map->registerMultiTileObject(
        503,                               
        "assets/track/Objects/tribune_full.png",
        2, 1,                               
        { 0.0f, -32.0f },                   
        1.0f,
        270.0f                               
    );

    startLineTop    = mGameState.map->findTile(307);
    startLineBottom = mGameState.map->findTile(271);
    // Expand finish line by 2 blocks above and below for more generous detection
    startLineTop.y    -= TILE_SIZE * 2.5f;  // 2 blocks above + half tile
    startLineBottom.y += TILE_SIZE * 2.5f;  // 2 blocks below + half tile
    
    //setup for corner system

    // Corner 1 - Bottom Left
    corners.push_back({
        { {4,20}, {5,20}, {4,21}, {5,21} },
        90.0f
    });

    // Corner 2 - Top Left
    corners.push_back({
        { {4,8}, {5,8}, {4,9}, {5,9} },
        90.0f
    });

    // Corner 3 - Top Right
    corners.push_back({
        { {34,8}, {35,8}, {34,9}, {35,9} },
        90.0f
    });

     // Corner 4 - Bottom Right
    corners.push_back({
        { {34,20}, {35,20}, {34,21}, {35,21} },
        90.0f
    });

    //create player car

    Vector2 startPos = {
        mOrigin.x - 1200.0f,
        mOrigin.y + 2800.0f
    };

    mCar = new Car(
        startPos,
        {150.0f, 60.0f},
        "assets/sportscars/sprites/sport_car_03_white/car.png",
        PORSCHE_911
    );

    mCar->setAngle(180.0f); // facing left

    /*
        ----------- CAMERA -----------
    */
    mGameState.camera = {0};
    mGameState.camera.target = startPos;
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 0.2f;

    mGameState.player = mCar;

    /*
        ----------- AI Cars -----------
    */

    if (mGameMode == 1) {
        // AI Car 1
        Vector2 AI1Pos = {
            mOrigin.x - 600.0f,
            mOrigin.y + 2600.0f
        };
        Car* AI1 = new Car(
            AI1Pos,
            {150.0f, 60.0f},
            "assets/sportscars/sprites/sport_car_01_yellow/car.png",
            HONDA_NSX
        );
        AI1->setAngle(180.0f);
        mAICars.push_back(AI1);

        // AI Car 2
        Vector2 AI2Pos = {
            mOrigin.x - 800.0f,
            mOrigin.y + 2800.0f
        };
        Car* AI2 = new Car(
            AI2Pos,
            {150.0f, 60.0f},
            "assets/sportscars/sprites/sport_car_04_red/car.png",
            LAMBORGHINI_GALLARDO
        );
        AI2->setAngle(180.0f);
        mAICars.push_back(AI2);

        // AI Car 3
        Vector2 AI3Pos = {
            mOrigin.x - 1000.0f,
            mOrigin.y + 2600.0f
        };
        Car* AI3 = new Car(
            AI3Pos,
            {150.0f, 60.0f},
            "assets/sportscars/sprites/sport_car_05_black/car.png",
            FORD_GT
        );
        AI3->setAngle(180.0f);
        mAICars.push_back(AI3);

        // Initialize AI waypoint tracking
        aiCurrentWaypoint.resize(mAICars.size(), 0);

        // Initialize lap tracking
        mLapCount.resize(4, 0);
        mPrevCarPositions.resize(4, startPos);
        mPrevCarPositions[0] = startPos;
        mPrevCarPositions[1] = AI1Pos;
        mPrevCarPositions[2] = AI2Pos;
        mPrevCarPositions[3] = AI3Pos;
        mRaceFinished = false;
        mPlayerFinishPosition = 0;
        mRaceCurrentCorner = 0;
    }

    // Setup AI waypoints
    setupAIWaypoints();
}

Vector2 TrackOne::tileToWorld(int col, int row) {
    return {
        mGameState.map->getLeftBoundary() + (col + 0.5f) * TILE_SIZE,
        mGameState.map->getTopBoundary() + (row + 0.5f) * TILE_SIZE
    };
}

void TrackOne::setupAIWaypoints() {
    // First straight 
    aiWaypoints.push_back(tileToWorld(17, 21));
    aiWaypoints.push_back(tileToWorld(10, 21));

    // Corner 1 
    aiWaypoints.push_back(tileToWorld(6, 21));   // Entry
    aiWaypoints.push_back(tileToWorld(5, 20));   // Apex
    aiWaypoints.push_back(tileToWorld(4, 18));   // Exit

    // Left straight 
    aiWaypoints.push_back(tileToWorld(4, 14));

    // Corner 2 
    aiWaypoints.push_back(tileToWorld(4, 10));   // Entry
    aiWaypoints.push_back(tileToWorld(5, 9));    // Apex
    aiWaypoints.push_back(tileToWorld(7, 8));    // Exit

    // Top straight 
    aiWaypoints.push_back(tileToWorld(17, 8));

    // Corner 3
    aiWaypoints.push_back(tileToWorld(33, 8));   // Entry
    aiWaypoints.push_back(tileToWorld(34, 9));   // Apex
    aiWaypoints.push_back(tileToWorld(35, 11));  // Exit

    // Right straight 
    aiWaypoints.push_back(tileToWorld(35, 14));

    // Corner 4 
    aiWaypoints.push_back(tileToWorld(35, 19));  // Entry
    aiWaypoints.push_back(tileToWorld(34, 20));  // Apex
    aiWaypoints.push_back(tileToWorld(32, 21));  // Exit

    // Final Straight
    aiWaypoints.push_back(tileToWorld(24, 21));
}

void TrackOne::updateAI(Car* aiCar, int aiIndex, float dt) {
    if (aiWaypoints.empty()) return;

    int currentWP = aiCurrentWaypoint[aiIndex];
    Vector2 targetWaypoint = aiWaypoints[currentWP];
    Vector2 carPos = aiCar->getPosition();

    // calculate distance to current waypoint
    float dx = targetWaypoint.x - carPos.x;
    float dy = targetWaypoint.y - carPos.y;
    float distToWaypoint = std::sqrt(dx * dx + dy * dy);

    // move to next waypoint if close enough
    if (distToWaypoint < TILE_SIZE * 2.0f) {
        aiCurrentWaypoint[aiIndex] = (currentWP + 1) % aiWaypoints.size();
        targetWaypoint = aiWaypoints[aiCurrentWaypoint[aiIndex]];
        dx = targetWaypoint.x - carPos.x;
        dy = targetWaypoint.y - carPos.y;
        distToWaypoint = std::sqrt(dx * dx + dy * dy);
    }

    // calculate target angle
    float targetAngle = std::atan2(dy, dx) * RAD2DEG;
    float carAngle = aiCar->getAngle();

    // normalize angle difference
    float angleDiff = targetAngle - carAngle;
    while (angleDiff > 180.0f) angleDiff -= 360.0f;
    while (angleDiff < -180.0f) angleDiff += 360.0f;

    // steering
    float steerStrength = 0.6f;
    float desiredSteer = angleDiff * steerStrength;

    // clamp to max steering 
    if (desiredSteer > 20.0f) desiredSteer = 20.0f;
    if (desiredSteer < -20.0f) desiredSteer = -20.0f;

    aiCar->setSteerAngle(desiredSteer);

    // speed control 
    float currentSpeed = aiCar->getSpeed();
    float targetSpeed = std::fmaxf(1500, 6*aiCar->getFrontGrip()/aiCar->getWeight()); // base target ~250 kph

    if (std::abs(angleDiff) > 45.0f) {
        targetSpeed = std::fmaxf(1500, 5*aiCar->getFrontGrip()/aiCar->getWeight()); 
    } else if (std::abs(angleDiff) > 20.0f) {
        targetSpeed = std::fmaxf(1500, 5.5*aiCar->getFrontGrip()/aiCar->getWeight()); 
    }

    // throttle/brake 
    if (currentSpeed < targetSpeed - 50.0f) {
        aiCar->accelerate(dt, mGameState.map);
    } else if (currentSpeed > targetSpeed + 50.0f) {
        aiCar->brake(dt);
    }
}

bool TrackOne::isInsideCorner(int col, int row, int cornerIndex) {
    if (cornerIndex >= corners.size()) return false;
    Corner& c = corners[cornerIndex];
    for (std::pair<int,int> p : c.tiles)
    {
        int tileCol = p.first;
        int tileRow = p.second;

        if (tileCol == col && tileRow == row)
            return true;
    }
    return false;
}

void TrackOne::update(float dt) {
    //return to menu
    if (IsKeyPressed(KEY_BACKSPACE)) {
        mGameState.nextSceneID = 0;
        return;
    }

    // collision tracking for all cars
    std::vector<Car*> allCars;
    if (mGameMode == 1) {
        allCars.push_back(mCar);
        for (size_t i = 0; i < mAICars.size(); i++) {
            allCars.push_back(mAICars[i]);
        }
    }

    if (mGameMode == 0) {
        if (
            mPrevCarPos.x > startLineTop.x &&
            mCar->getPosition().x < startLineTop.x &&
            mCar->getPosition().y >= startLineTop.y &&
            mCar->getPosition().y <= startLineBottom.y
        ){
            //crossed the line
            if(!mInLap){
                mInLap = true;
                mLapDisqualified = false;
                mCurrentLapTime = 0.0f;
            } else {
                // check if valid lap
                if (!mLapDisqualified && currentCorner >= corners.size()) {
                    if (mBestLapTime == 0.0f || mCurrentLapTime < mBestLapTime) {
                        mBestLapTime = mCurrentLapTime;
                        PlaySound(mGameState.pingSound);
                    }
                }
                mCurrentLapTime = 0.0f;
                mLapDisqualified = false;
            }
            currentCorner = 0;
        }

        if (mInLap) {
            mCurrentLapTime += dt;
        }

        if (mInLap && mGameState.map->getTileAtWorldPos(mCar->getPosition()) == 0) {
            mLapDisqualified = true;
        }

        mPrevCarPos = mCar->getPosition();

        int col = (mCar->getPosition().x - mGameState.map->getLeftBoundary()) / TILE_SIZE;
        int row = (mCar->getPosition().y - mGameState.map->getTopBoundary())  / TILE_SIZE;

        if (currentCorner < corners.size() && isInsideCorner(col, row, currentCorner))
        {
            currentCorner++;
        }

    } else {
        // race mode
        if (!mRaceFinished) {
            // track player corner progress
            Vector2 playerPos = mCar->getPosition();
            int col = (playerPos.x - mGameState.map->getLeftBoundary()) / TILE_SIZE;
            int row = (playerPos.y - mGameState.map->getTopBoundary())  / TILE_SIZE;

            if (mRaceCurrentCorner < corners.size() && isInsideCorner(col, row, mRaceCurrentCorner))
            {
                mRaceCurrentCorner++;
            }

            //crossed the line
            if (mPrevCarPositions[0].x > startLineTop.x &&
                playerPos.x < startLineTop.x &&
                playerPos.y >= startLineTop.y &&
                playerPos.y <= startLineBottom.y) {

                if (mLapCount[0] == 0) {
                    // start lap 1
                    mLapCount[0] = 1;
                    mRaceCurrentCorner = 0;
                    mIncompleteLapWarning = false;
                } else if (mRaceCurrentCorner >= corners.size()) {
                    // valid lap
                    mLapCount[0]++;
                    mRaceCurrentCorner = 0;  
                    mIncompleteLapWarning = false;

                    // check if player finished 5 laps
                    if (mLapCount[0] > 5) {
                        mRaceFinished = true;

                        // calculate player finish position
                        int position = 1;
                        for (size_t i = 1; i < mLapCount.size(); i++) {
                            if (mLapCount[i] > 5) {
                                position++;
                            }
                        }
                        mPlayerFinishPosition = position;

                        // play win/lose sound
                        if (!mResultSoundPlayed) {
                            if (mPlayerFinishPosition == 1) {
                                PlaySound(mGameState.winSound);
                            } else {
                                PlaySound(mGameState.loseSound);
                            }
                            mResultSoundPlayed = true;
                        }
                    }
                } else {
                    //invalid lap
                    mIncompleteLapWarning = true;
                }
            }
            mPrevCarPositions[0] = playerPos;

            // check AI car lap crossings
            for (size_t i = 0; i < mAICars.size(); i++) {
                Vector2 aiPos = mAICars[i]->getPosition();
                int carIndex = i + 1;

                if (mPrevCarPositions[carIndex].x > startLineTop.x &&
                    aiPos.x < startLineTop.x &&
                    aiPos.y >= startLineTop.y &&
                    aiPos.y <= startLineBottom.y) {
                    mLapCount[carIndex]++;
                }
                mPrevCarPositions[carIndex] = aiPos;
            }
        }

        // update AI cars 
        if (!mRaceFinished) {
            for (size_t i = 0; i < mAICars.size(); i++) {
                updateAI(mAICars[i], i, dt);

                // Build list of AI cars
                std::vector<Car*> otherCars;
                for (size_t j = 0; j < allCars.size(); j++) {
                    if (allCars[j] != mAICars[i]) {
                        otherCars.push_back(allCars[j]);
                    }
                }
                mAICars[i]->update(dt, mGameState.map, otherCars);
            }
        }
    }

    // update player car
    if (mGameMode == 1) {
        if (!mRaceFinished) {
            // Build list of AI cars
            std::vector<Car*> otherCars;
            for (size_t i = 0; i < mAICars.size(); i++) {
                otherCars.push_back(mAICars[i]);
            }
            mCar->update(dt, mGameState.map, otherCars);
        }
    } else {
        std::vector<Car*> otherCars;
        mCar->update(dt, mGameState.map, otherCars);
    }

    UpdateMusicStream(mGameState.bgm1);

    // camera follow
    mGameState.camera.target = mCar->getPosition();
    mGameState.camera.rotation = mGameState.camera.rotation = -(mCar->getAngle() + 90); // keep camera facing forward
}

void TrackOne::render() {
    ClearBackground(ColorFromHex(mBGColourHexCode));

    BeginMode2D(mGameState.camera);

    // apply shader
    if (mGameMode == 1 && mRaceFinished) {
        BeginShaderMode(mVignetteShader);
        Vector2 carPos = mCar->getPosition();
        float lightPos[2] = { carPos.x, carPos.y };
        SetShaderValue(mVignetteShader, mLightPositionLoc, lightPos, SHADER_UNIFORM_VEC2);
        DrawRectangle(-10000, -10000, 20000, 20000, ColorFromHex(mBGColourHexCode));
    }

    mGameState.map->render();

    // render cars
    for (Car* aiCar : mAICars) {
        aiCar->render();
    }

    mCar->render();

    if (mGameMode == 1 && mRaceFinished) {
        EndShaderMode();
    }

    EndMode2D();

    renderUI();
}


void TrackOne::renderUI() {
    // speed indicator 
    DrawText(TextFormat("Speed: %03i kph", (int)(mCar->getSpeed())/10), 1100, 50, 20, WHITE);

    // race UI
    if (mGameMode == 1) {

        if (!mRaceFinished) {
             // lap counter
            int displayLap = (mLapCount[0] == 0) ? 1 : mLapCount[0];
            if (displayLap > 5){
                displayLap = 5;
            }
            DrawText(TextFormat("Lap: %d/5", displayLap), 1100, 100, 24, WHITE);

            // incomplete lap warning
            if (mIncompleteLapWarning) {
                DrawText("Missed corner: incomplete lap", 850, 150, 20, RED);
            }
        } else {
            // finish screen
            const char* messages[] = {
                "1st place finish!",
                "2nd place finish!",
                "3rd place finish!",
                "4th place finish!"
            };
            const char* message = messages[mPlayerFinishPosition - 1];
            Color color = (mPlayerFinishPosition == 1) ? GOLD : WHITE;

            DrawText(message, 400, 300, 60, color);
        }
    }

    // hotlap UI
    if (mGameMode == 0) {
        // current lap time
        if (mInLap){
            DrawText(TextFormat("Current Laptime: %.3f", mCurrentLapTime), 1000, 100, 20, WHITE);
        }
        else{
            DrawText("Current Laptime: --.--", 1000, 100, 20, WHITE);
        }

        // best lap time
        if (mBestLapTime > 0.0f){
            DrawText(TextFormat("Best Laptime: %.3f", mBestLapTime), 1000, 150, 20, WHITE);
        }else{
            DrawText("Best Laptime: --.--", 1000, 150, 20, WHITE);
        }

        // dq indicator
        if (mLapDisqualified){
            DrawText("Lap Disqualified", 1000, 200, 20, RED);
        }
    }
}

void TrackOne::shutdown() {
    delete mCar;
    delete mGameState.map;

    // clean up AI cars
    for (Car* aiCar : mAICars) {
        delete aiCar;
    }
    mAICars.clear();

    // Stop music (don't unload - it's shared)
    StopMusicStream(mGameState.bgm1);

    // unload shader
    UnloadShader(mVignetteShader);

    // clear all tracking vectors
    aiWaypoints.clear();
    aiCurrentWaypoint.clear();
    corners.clear();
    mLapCount.clear();
    mPrevCarPositions.clear();
}
