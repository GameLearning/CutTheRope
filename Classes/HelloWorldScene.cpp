#include "HelloWorldScene.h"

USING_NS_CC;
#define cc_to_b2Vec(x,y)   (b2Vec2((x)/PTM_RATIO, (y)/PTM_RATIO))

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
    
    visibleSize = Director::getInstance()->getVisibleSize();

    SpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("CutTheVerlet.plist");
    Sprite *background = Sprite::createWithSpriteFrameName("bg.png");
    background->setPosition(visibleSize.width/2, visibleSize.height/2);
    this->addChild(background ,-1);
    
    croc_ = Sprite::createWithSpriteFrameName("croc_front_mouthclosed.png");
    croc_->setAnchorPoint(Vec2(1,0));
    croc_->setPosition(Vec2(320.0, 30.0));
    this->addChild(croc_ ,1);
    
    ropeSpriteSheet = SpriteBatchNode::create("rope_texture.png",1000);
    this->addChild(ropeSpriteSheet,999);
    
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
    
    initLevel();
    this->scheduleUpdate();
    
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
    listener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    return true;
}

b2Body * HelloWorld::createCandyAt(Vec2 pt) {
     // Get the sprite from the sprite sheet
    Sprite *sprite = Sprite::createWithSpriteFrameName("pineapple.png");
    sprite->setPosition(pt);
    this->addChild(sprite);
 
    // Defines the body of your candy
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2(pt.x/PTM_RATIO, pt.y/PTM_RATIO);
    bodyDef.userData = sprite;
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
 
    candies.push_back(body);
 
    return body;
}


void HelloWorld::createRopeWith(b2Body*bodyA, b2Vec2 anchorA, b2Body*bodyB, b2Vec2 anchorB, float sag)
{
    b2RopeJointDef jd;
    jd.bodyA = bodyA;
    jd.bodyB = bodyB;
    jd.localAnchorA = anchorA;
    jd.localAnchorB = anchorB;
 
    // Max length of joint = current distance between bodies * sag
    float ropeLength = (bodyA->GetWorldPoint(anchorA) - bodyB->GetWorldPoint(anchorB)).Length() * sag;
    jd.maxLength = ropeLength;
 
    // Create joint
    b2RopeJoint *ropeJoint = (b2RopeJoint *)_world->CreateJoint(&jd);
 
    VRope *newRope = new VRope(ropeJoint, ropeSpriteSheet);
    
    ropes.push_back(newRope);
}


void HelloWorld::initLevel()
{
    Size s = Director::getInstance()->getVisibleSize();
 
    // Add the candy
    b2Body *body1 = createCandyAt(Vec2(s.width * 0.5, s.height * 0.7));
 
    // Add a bunch of ropes
    createRopeWith(groundBody ,cc_to_b2Vec(s.width * 0.15, s.height * 0.8),
                        body1 ,body1->GetLocalCenter(),
                          1.1);
    
    createRopeWith(body1 ,body1->GetLocalCenter(),
                        groundBody ,cc_to_b2Vec(s.width * 0.85, s.height * 0.8),
                          1.1);
    
    createRopeWith(body1 ,body1->GetLocalCenter(),
                        groundBody ,cc_to_b2Vec(s.width * 0.83, s.height * 0.6),
                          1.1);
}

void HelloWorld::update(float dt) {
    _world->Step(dt,10,10);
    for (b2Body* b = _world->GetBodyList(); b; b = b->GetNext())
    {
        Sprite *myActor = (Sprite*)b->GetUserData();
        if (myActor)
        {
            //Synchronize the AtlasSprites position and rotation with the corresponding body
            myActor->setPosition(Vec2( b->GetPosition().x * PTM_RATIO, b->GetPosition().y * PTM_RATIO));
            myActor->setRotation(-1 * CC_RADIANS_TO_DEGREES(b->GetAngle()));
        }
    }
    // Update all the ropes
    for (unsigned int i = 0; i < ropes.size(); i++) {
        ropes[i]->update(dt);
        ropes[i]->updateSprites();
    }
}


bool HelloWorld::checkLineIntersection(Vec2 p1,Vec2 p2, Vec2 p3, Vec2 p4)
{
    // http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
    float denominator = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
 
    // In this case the lines are parallel so you assume they don't intersect
    if (denominator == 0.0f)
        return false;
    float ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denominator;
    float ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denominator;
 
    if (ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0)
    {
        return true;
    }
 
    return false;
}

bool HelloWorld::onTouchBegan(Touch* touch, Event* unused_event){
    return true;
}

void HelloWorld::onTouchMoved(Touch* touch, Event* unused_event){
    Vec2 pt0 = touch->getPreviousLocationInView();
    Vec2 pt1 = touch->getLocationInView();
 
    // Correct Y axis coordinates to cocos2d coordinates
    pt0.y = visibleSize.height - pt0.y;
    pt1.y = visibleSize.height - pt1.y;
    
    
    for (unsigned int i = 0; i < ropes.size(); i++) {
        auto sticks = ropes[i]->getSticks();
        for (unsigned int x = 0; x < sticks.size(); x++) {
            auto stick = sticks[x];
            Vec2 pa = stick->getPointA()->getPoint();
            Vec2 pb = stick->getPointB()->getPoint();
 
            if (checkLineIntersection(pt0 ,pt1 ,pa ,pb))
            {
                // Cut the rope here
                return;
            }
        }
    }
}
