#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    auto scene = Scene::create();
    auto layer = HelloWorld::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() ) return false;
    
    Size visibleSize = Director::getInstance()->getVisibleSize();

    SpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("CutTheVerlet.plist");
    Sprite *background = Sprite::createWithSpriteFrameName("bg.png");
    background->setPosition(visibleSize.width/2, visibleSize.height/2);
    this->addChild(background ,-1);
    
    croc_ = Sprite::createWithSpriteFrameName("croc_front_mouthclosed.png");
    croc_->setAnchorPoint(Vec2(1,0));
    croc_->setPosition(Vec2(320.0, 30.0));
    this->addChild(croc_ ,1);
    return true;
}