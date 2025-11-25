#ifndef TRACKSELECTION_H
#define TRACKSELECTION_H

#include "Scene.h"

class TrackSelection : public Scene {
public:
    TrackSelection();
    TrackSelection(Vector2 origin, const char *bgHexCode);
    ~TrackSelection();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
