#include "Scene.h"
Scene::Scene() : mOrigin{0, 0}, mBGColourHexCode{"#000000"}, mGameState{} {}

Scene::Scene(Vector2 origin, const char *bgHexCode) 
    : mOrigin{origin}, mBGColourHexCode{bgHexCode}, mGameState{} 
{}