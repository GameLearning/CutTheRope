#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "vRope/vrope.h"

#define PTM_RATIO 32.0

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(HelloWorld);
    void update(float dt);
private:
    b2Body * createCandyAt(cocos2d::Vec2 pt);
    void createRopeWith(b2Body*bodyA, b2Vec2 anchorA, b2Body*bodyB, b2Vec2 anchorB, float sag);
    void initLevel();
    bool checkLineIntersection(Vec2 p1,Vec2 p2, Vec2 p3, Vec2 p4);
    
    cocos2d::Sprite* croc_;
    std::vector<VRope*> ropes;
    std::vector<b2Body*> candies;
    b2Body* groundBody;
    b2World *_world;
    cocos2d::SpriteBatchNode *ropeSpriteSheet;
};