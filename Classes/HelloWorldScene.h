#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"

#define PTM_RATIO 32.0

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(HelloWorld);
private:
    b2Body * createCandyAt(cocos2d::Vec2 pt);
    
    cocos2d::Sprite* croc_;
    cocos2d::Array *ropes;
    cocos2d::Array *candies;
    b2Body* groundBody;
    b2World *_world;
    cocos2d::SpriteBatchNode *ropeSpriteSheet;
};