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
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* unused_event);
private:
    b2Body * createCandyAt(cocos2d::Vec2 pt);
    void createRopeWith(b2Body*bodyA, b2Vec2 anchorA, b2Body*bodyB, b2Vec2 anchorB, float sag);
    void initLevel();
    bool checkLineIntersection(cocos2d::Vec2 p1, cocos2d::Vec2 p2, cocos2d::Vec2 p3, cocos2d::Vec2 p4);
    
    cocos2d::Sprite* croc_;
    std::vector<VRope*> ropes;
    std::vector<b2Body*> candies;
    b2Body* groundBody;
    b2World *_world;
    cocos2d::SpriteBatchNode *ropeSpriteSheet;
    cocos2d::Size visibleSize;
};