#include "HelloWorldScene.h"
#include "libwebsockets.h"

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
    groundShape.Set(b2Vec2(-visibleSize.width/PTM_RATIO,0),b2Vec2(2*visibleSize.width /PTM_RATIO,0));
    groundBody->CreateFixture(&groundShapeDef);
    
    b2BodyDef crocBodyDef;
    crocBodyDef.position.Set((visibleSize.width - croc_->getTextureRect().size.width)/PTM_RATIO, (croc_->getPosition().y)/PTM_RATIO);
 
    crocMouth_ = _world->CreateBody(&crocBodyDef);

    b2EdgeShape crocBox;
    crocBox.Set(b2Vec2(5.0/PTM_RATIO,15.0/PTM_RATIO), b2Vec2(45.0/PTM_RATIO,15.0/PTM_RATIO));
    crocMouthBottom_ = crocMouth_->CreateFixture(&crocBox,0);
    crocMouth_->SetActive(false);
    
    initLevel();
    this->scheduleUpdate();
    openCrocMouth(0);
    
    contactListener = new MyContactListener();
    _world->SetContactListener(contactListener);
    
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
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


void HelloWorld::initLevel(float dt)
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
    
     // Add the candy
    b2Body *body2 = createCandyAt(Vec2(s.width * 0.5, s.height));
 
    // Change the linear dumping so it swings more
    body2->SetLinearDamping(0.01);
 
    // Add a bunch of ropes
    createRopeWith(groundBody, cc_to_b2Vec(s.width * 0.65, s.height + 5)
                        , body2, body2->GetLocalCenter()
                        , 1.0);
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
    
    
    
    
     // Check for collisions
    bool shouldCloseCrocMouth = false;
    std::vector<b2Body *>toDestroy;
    std::vector<MyContact>::iterator pos;
    for(pos = contactListener->_contacts.begin(); pos != contactListener->_contacts.end(); ++pos)
    {
        MyContact contact = *pos;
 
        bool hitTheFloor = false;
        b2Body *potentialCandy = nullptr;
 
        // The candy can hit the floor or the croc's mouth. Let's check
        // what it's touching.
        if (contact.fixtureA == crocMouthBottom_)
        {
            potentialCandy = contact.fixtureB->GetBody();
        }
        else if (contact.fixtureB == crocMouthBottom_)
        {
            potentialCandy = contact.fixtureA->GetBody();
        }
        else if (contact.fixtureA->GetBody() == groundBody)
        {
            potentialCandy = contact.fixtureB->GetBody();
            hitTheFloor = true;
        }
        else if (contact.fixtureB->GetBody() == groundBody)
        {
            potentialCandy = contact.fixtureA->GetBody();
            hitTheFloor = true;
        }
 
        // Check if the body was indeed one of the candies
        if (potentialCandy /*&& [candies indexOfObject:[NSValue valueWithPointer:potentialCandy]] != NSNotFound*/)
        {
            // Set it to be destroyed
            toDestroy.push_back(potentialCandy);
            if (hitTheFloor)
            {
                // If it hits the floor you'll remove all the physics of it and just simulate the pineapple sinking
                Sprite *sinkingCandy = (Sprite*)potentialCandy->GetUserData();
 
                // Sink the pineapple
                CCFiniteTimeAction *sink = MoveBy::create(3.0, Vec2(0, -sinkingCandy->getTextureRect().size.height));
                
                // Run the actions sequentially.
                sinkingCandy->runAction(CCSequence::create(sink,
                                         CallFuncN::create(this, callfuncN_selector(HelloWorld::removeCandy)),
                                         NULL));
                potentialCandy->SetUserData(NULL);
            }
            else
            {
                shouldCloseCrocMouth = true;
            }
        }
    }
    
    
    std::vector<b2Body *>::iterator pos2;
    for(pos2 = toDestroy.begin(); pos2 != toDestroy.end(); ++pos2)
    {
        b2Body *body = *pos2;
        if (body->GetUserData() != NULL)
        {
            // Remove the sprite
            CCSprite *sprite = (CCSprite *) body->GetUserData();
            removeChild(sprite, true);
            body->SetUserData(NULL);
        }
 
        // Iterate though the joins and check if any are a rope
        b2JointEdge* joints = body->GetJointList();
        while (joints)
        {
            b2Joint *joint = joints->joint;
 
            // Look in all the ropes
            for (unsigned int i = 0; i < ropes.size(); i++) {
                VRope * rope = ropes[i];
                if (rope->joint == joint)
                {
                    // This "destroys" the rope
                    rope->removeSprites();
                    ropes.erase(ropes.begin() + i);
                    break;
                }
            }
 
            joints = joints->next;
            _world->DestroyJoint(joint);
        }
 
        // Destroy the physics body
        _world->DestroyBody(body);
 
        // Removes from the candies array
        candies.erase( std::remove( std::begin(candies), std::end(candies), body ), std::end(candies) );
    }
 
    if (shouldCloseCrocMouth)
    {
        closeCrocMouth(0);
        checkLevelFinish(false);
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
                b2Body *newBodyA = this->createRopeTipBody();
                b2Body *newBodyB = this->createRopeTipBody();
 
                VRope *newRope = ropes[i]->cutRopeInStick(x, stick ,newBodyA ,newBodyB);
                ropes.push_back(newRope);
                return;
            }
        }
    }
}

b2Body * HelloWorld::createRopeTipBody()
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.linearDamping = 0.5f;
    b2Body *body = _world->CreateBody(&bodyDef);
 
    b2FixtureDef circleDef;
    b2CircleShape circle;
    circle.m_radius = 1.0/PTM_RATIO;
    circleDef.shape = &circle;
    circleDef.density = 10.0f;
 
    circleDef.filter.maskBits = 0;
    body->CreateFixture(&circleDef);
 
    return body;
}

void HelloWorld::openCrocMouth(float dt) {
    croc_->setDisplayFrame(SpriteFrameCache::sharedSpriteFrameCache()->getSpriteFrameByName("croc_front_mouthopen.png"));
    croc_->setZOrder(1);
    crocMouth_->SetActive(true);

    float interval = 3.0 + 2.0 * rand_0_1();
    this->scheduleOnce(schedule_selector(HelloWorld::closeCrocMouth), interval);
}

void HelloWorld::closeCrocMouth(float dt) {
    croc_->setDisplayFrame(SpriteFrameCache::sharedSpriteFrameCache()->getSpriteFrameByName("croc_front_mouthclosed.png"));
    croc_->setZOrder(-1);
    crocMouth_->SetActive(false);

    float interval = 3.0 + 2.0 * rand_0_1();
    this->scheduleOnce(schedule_selector(HelloWorld::openCrocMouth), interval);
}

void HelloWorld::removeCandy(Node* node){
    removeChild(node, true);
    checkLevelFinish(true);
}

void HelloWorld::checkLevelFinish(bool forceFinish){
    if (candies.size() == 0 || forceFinish)
    {
        // Destroy everything
        finishedLevel();
        this->scheduleOnce(schedule_selector(HelloWorld::initLevel), 2);
    }
}

void HelloWorld::finishedLevel(){
    std::set<b2Body *>toDestroy;
    for (unsigned int i = 0; i < ropes.size(); i++) {
        VRope* rope = ropes[i];
        rope->removeSprites();
        if (rope->joint->GetBodyA() != groundBody)
            toDestroy.insert(rope->joint->GetBodyA());
        if (rope->joint->GetBodyB() != groundBody)
            toDestroy.insert(rope->joint->GetBodyB());
        
        _world->DestroyJoint(rope->joint);
    }
    ropes.clear();
    
    std::set<b2Body *>::iterator pos;
    for(pos = toDestroy.begin(); pos != toDestroy.end(); ++pos)
    {
        b2Body *body = *pos;
        if (body->GetUserData() != NULL)
        {
            // Remove the sprite
            CCSprite *sprite = (CCSprite *) body->GetUserData();
            removeChild(sprite, true);
            body->SetUserData(NULL);
        }
        _world->DestroyBody(body);
    }
    candies.clear();
}
