#ifndef WINLEVEL_H
#define WINLEVEL_H

#include "Scene.h"

class WinLevel : public Scene {
public:
    WinLevel(Vector2 origin, const char *bgHexCode);
    ~WinLevel();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif