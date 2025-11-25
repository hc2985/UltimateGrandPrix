#ifndef SCENE_H
#define SCENE_H

#include "car.h"

struct GameState
{
    Car* player;
    std::vector<Car>* AI;
    Map* map;

    Music bgm1;
    Music bgm2;
    Music bgm3;
    Sound winSound;
    Sound loseSound;
    Sound pingSound;

    Camera2D camera;
    Vector2 spawnPoint;

    int nextSceneID;
    int gameMode;
};

class Scene 
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char* mBGColourHexCode;

public:
    Scene();
    Scene(Vector2 origin, const char* bgHexCode);

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    GameState& getState() { return mGameState; }
    Vector2 getOrigin() const { return mOrigin; }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif
