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
    
    ropes = Array::create();
    candies = Array::create();
    ropeSpriteSheet = SpriteBatchNode::create("rope_texture.png",100);
    this->addChild(ropeSpriteSheet);
    
    _world = new b2World(b2Vec2(0.0f, -8.0f));
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, 0.0f);
    groundBody = _world->CreateBody(&groundBodyDef);
    b2EdgeShape groundShape;
    b2FixtureDef groundShapeDef;
    groundShapeDef.shape = &groundShape;
    groundShape.Set(b2Vec2(0,0),b2Vec2(visibleSize.width /PTM_RATIO,0));
    groundBody->CreateFixture(&groundShapeDef);
    groundShape.Set(b2Vec2(0,0),b2Vec2(0,visibleSize.height /PTM_RATIO));
    groundBody->CreateFixture(&groundShapeDef);
    groundShape.Set(b2Vec2(visibleSize.width /PTM_RATIO,0),b2Vec2(visibleSize.width /PTM_RATIO,visibleSize.height /PTM_RATIO));
    groundBody->CreateFixture(&groundShapeDef);
    groundShape.Set(b2Vec2(visibleSize.width /PTM_RATIO,visibleSize.height /PTM_RATIO),b2Vec2(0,visibleSize.height /PTM_RATIO));
    groundBody->CreateFixture(&groundShapeDef);
    
    return true;
}

b2Body * HelloWorld::createCandyAt(Vec2 pt) {
     // Get the sprite from the sprite sheet
//    CCSprite *sprite = [CCSprite spriteWithSpriteFrameName:@"pineapple.png"];
//    [self addChild:sprite];
 
    // Defines the body of your candy
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2(pt.x/PTM_RATIO, pt.y/PTM_RATIO);
//    bodyDef.userData = sprite;
    bodyDef.linearDamping = 0.3f;
    b2Body *body = _world->CreateBody(&bodyDef);
 
    // Define the fixture as a polygon
    b2FixtureDef fixtureDef;
    b2PolygonShape spriteShape;
 
    b2Vec2 verts[] = {
        b2Vec2(-7.6f / PTM_RATIO, -34.4f / PTM_RATIO),
        b2Vec2(8.3f / PTM_RATIO, -34.4f / PTM_RATIO),
        b2Vec2(15.55f / PTM_RATIO, -27.15f / PTM_RATIO),
        b2Vec2(13.8f / PTM_RATIO, 23.05f / PTM_RATIO),
        b2Vec2(-3.35f / PTM_RATIO, 35.25f / PTM_RATIO),
        b2Vec2(-16.25f / PTM_RATIO, 25.55f / PTM_RATIO),
        b2Vec2(-15.55f / PTM_RATIO, -23.95f / PTM_RATIO)
    };
 
    spriteShape.Set(verts, 7);
    fixtureDef.shape = &spriteShape;
    fixtureDef.density = 30.0f;
    fixtureDef.filter.categoryBits = 0x01;
    fixtureDef.filter.maskBits = 0x01;
    body->CreateFixture(&fixtureDef);
 
//    [candies addObject:[NSValue valueWithPointer:body]];
 
    return body;
}
