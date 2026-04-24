#ifndef LOSELEVEL_H
#define LOSELEVEL_H

#include "Scene.h"

class LoseLevel : public Scene {
public:
    LoseLevel(Vector2 origin, const char *bgHexCode);
    ~LoseLevel();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif