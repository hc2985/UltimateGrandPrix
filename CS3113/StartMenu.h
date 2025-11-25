#ifndef STARTMENU_H
#define STARTMENU_H

#include "Scene.h"

class StartMenu : public Scene {
public:
    StartMenu();
    StartMenu(Vector2 origin, const char *bgHexCode);
    ~StartMenu();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif