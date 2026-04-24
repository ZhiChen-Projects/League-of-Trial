#ifndef MENULEVEL_H
#define MENULEVEL_H

#include "Scene.h"

class MenuLevel : public Scene {
public:
    MenuLevel(Vector2 origin, const char *bgHexCode);
    ~MenuLevel();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif