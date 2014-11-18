/*
 
 MIT License.

 Copyright (c) 2012 Creeng Ltd.
 Copyright (c) 2012 Flightless Ltd.  
 Copyright (c) 2010 Clever Hamster Games.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
*/

//
//  vrope.cpp
//
//  Created by patrick on 14/10/2010.
//  Modified by Flightless www.flightless.co.nz 20/4/2012
//  Ported to cocos2d-x by Creeng www.creeng.com 12/7/2012
//

#include "vrope.h"

using namespace cocos2d;

// Flightless, init rope using a joint between two bodies
VRope::VRope(b2RopeJoint* joint, CCSpriteBatchNode* ropeBatchNode)
    : joint(joint), spriteSheet(ropeBatchNode)
{
    Vec2 pointA = Vec2(joint->GetAnchorA().x*PTM_RATIO,joint->GetAnchorA().y*PTM_RATIO);
    Vec2 pointB = Vec2(joint->GetAnchorB().x*PTM_RATIO,joint->GetAnchorB().y*PTM_RATIO);
    createRope(pointA, pointB,joint->GetMaxLength()*PTM_RATIO);
}

VRope::~VRope()
{
    for(int i=0; i<numPoints; ++i)
    {
        delete vPoints[i];
        if (i!=numPoints-1) {
            delete vSticks[i];
        }
    }
}

void VRope::update(float dt)
{
    Vec2 pointA = Vec2(joint->GetAnchorA().x*PTM_RATIO,joint->GetAnchorA().y*PTM_RATIO);
    Vec2 pointB = Vec2(joint->GetAnchorB().x*PTM_RATIO,joint->GetAnchorB().y*PTM_RATIO);
    updateWithPoints(pointA, pointB, dt);
}

void VRope::reset()
{
    Vec2 pointA = Vec2(joint->GetAnchorA().x*PTM_RATIO,joint->GetAnchorA().y*PTM_RATIO);
    Vec2 pointB = Vec2(joint->GetAnchorB().x*PTM_RATIO,joint->GetAnchorB().y*PTM_RATIO);
    resetWithPoints(pointA, pointB);
}
    
void VRope::createRope(const CCPoint& pointA, const CCPoint& pointB, float distance)
{
//    float distance = ccpDistance(pointA,pointB);
    int segmentFactor = 20; // 16; //12; //increase value to have less segments per rope, decrease to have more segments
    numPoints = (int) distance/segmentFactor;

    CCPoint diffVector = ccpSub(pointB,pointA);
    float multiplier = distance / (numPoints-1);
    antiSagHack = 0.1f; //HACK: scale down rope points to cheat sag. set to 0 to disable, max suggested value 0.1
    for(int i=0;i<numPoints;i++) {
            CCPoint tmpVector = ccpAdd(pointA, ccpMult(ccpNormalize(diffVector), multiplier*i*(1-antiSagHack)));		
    VPoint *tmpPoint = new VPoint();
    tmpPoint->setPos(tmpVector.x, tmpVector.y);
    vPoints.push_back(tmpPoint);
    }
    for(int i=0;i<numPoints-1;i++) {
    VStick* tmpStick = new VStick(vPoints[i], vPoints[i+1]);
    vSticks.push_back(tmpStick);
    }

    if(spriteSheet) {
	for(int i=0;i<numPoints-1;i++) {
            VPoint* point1 = vSticks[i]->getPointA();
            VPoint* point2 = vSticks[i]->getPointB();
            CCPoint stickVector = ccpSub(ccp(point1->x,point1->y),ccp(point2->x,point2->y));
            float stickAngle = ccpToAngle(stickVector);

            float f = spriteSheet->getTextureAtlas()->getTexture()->getPixelsHigh() / CC_CONTENT_SCALE_FACTOR();
            CCRect r = CCRectMake(0, 0, multiplier, f);
            Sprite* tmpSprite = Sprite::createWithTexture(spriteSheet->getTexture(), r);
            
            Texture2D::TexParams params = {GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT};
            tmpSprite->getTexture()->setTexParameters(&params);
            tmpSprite->setPosition(ccpMidpoint(ccp(point1->x, point1->y), ccp(point2->x, point2->y)));
            tmpSprite->setRotation(-1 * CC_RADIANS_TO_DEGREES(stickAngle));
            spriteSheet->addChild(tmpSprite);
            ropeSprites.push_back(tmpSprite);
        }
    }
}

void VRope::resetWithPoints(const CCPoint &pointA, const CCPoint& pointB)
{
	float distance = ccpDistance(pointA, pointB);
	CCPoint diffVector = ccpSub(pointB, pointA);
	float multiplier = distance / (numPoints - 1);
	for(int i=0;i<numPoints;i++) {
		CCPoint tmpVector = ccpAdd(pointA, ccpMult(ccpNormalize(diffVector), multiplier*i*(1-antiSagHack)));
        VPoint *tmpPoint = vPoints[i];
        tmpPoint->setPos(tmpVector.x, tmpVector.y);
	}
}

void VRope::updateWithPoints(const CCPoint& pointA, const CCPoint& pointB, float dt)
{
    //manually set position for first and last point of rope
    vPoints[0]->setPos(pointA.x, pointA.y);
    vPoints[numPoints -1]->setPos(pointB.x, pointB.y);
	
	//update points, apply gravity
	for(int i=1;i<numPoints-1;i++) {
        vPoints[i]->applyGravity(dt);
        vPoints[i]->update();
	}
	
	//contract sticks
	int iterations = 4;
	for(int j=0;j<iterations;j++) {
		for(int i=0;i<numPoints-1;i++) {
            vSticks[i]->contract();
		}
	}
}

void VRope::updateWithPoints(const CCPoint& pointA, const CCPoint& pointB, float gxdt, float gydt)
{
	//manually set position for first and last point of rope
    vPoints[0]->setPos(pointA.x, pointA.y);
    vPoints[numPoints-1]->setPos(pointB.x, pointB.y);
	
	//update points, apply pre-integrated gravity
	for(int i=1;i<numPoints-1;i++) {
        vPoints[i]->applyGravityxdt(gxdt, gydt);
        vPoints[i]->update();
	}
	
	//contract sticks
	int iterations = 4;
	for(int j=0;j<iterations;j++) {
		for(int i=0;i<numPoints-1;i++) {
            vSticks[i]->contract();
		}
	}
}

void VRope::debugDraw()
{
    //Depending on scenario, you might need to have different Disable/Enable of Client States
	//glDisableClientState(GL_TEXTURE_2D);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	//glDisableClientState(GL_COLOR_ARRAY);
	//set color and line width for ccDrawLine

    ccDrawColor4F(0.0f, 0.0f, 1.0f, 1.0f);
    glLineWidth(5.0f);

	for(int i=0;i<numPoints-1;i++) {
		//"debug" draw
		VPoint *pointA = vSticks[i]->getPointA();
		VPoint *pointB = vSticks[i]->getPointB();
		ccDrawPoint(ccp(pointA->x,pointA->y));
		ccDrawPoint(ccp(pointB->x,pointB->y));
		//ccDrawLine(ccp(pointA.x,pointA.y),ccp(pointB.x,pointB.y));
	}

    //restore to white and default thickness
	ccDrawColor4F(1.0f,1.0f,1.0f,1.0f);
    glLineWidth(1);
	//glEnableClientState(GL_TEXTURE_2D);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);
}

void VRope::updateSprites()
{
	if(spriteSheet) {
		for(int i=0;i<numPoints-1;i++) {
			VPoint *point1 = vSticks[i]->getPointA();
			VPoint *point2 = vSticks[i]->getPointB();
			CCPoint point1_ = ccp(point1->x,point1->y);
			CCPoint point2_ = ccp(point2->x,point2->y);
			float stickAngle = ccpToAngle(ccpSub(point1_,point2_));
			CCSprite *tmpSprite = ropeSprites[i];
			tmpSprite->setPosition(ccpMidpoint(point1_,point2_));
			tmpSprite->setRotation(-CC_RADIANS_TO_DEGREES(stickAngle));
		}
	}
}

void VRope::removeSprites()
{
    for(int i=0;i<numPoints-1;i++) {
		CCSprite *tmpSprite = ropeSprites[i];
        spriteSheet->removeChild(tmpSprite, true);
	}
    ropeSprites.clear();
}

std::vector<VStick*> VRope::getSticks() {
    return vSticks;
}
typedef struct {
    unsigned int location;
    unsigned int length;
} NSRange;
VRope* VRope::cutRopeInStick(int nPoint, VStick* stick, b2Body* newBodyA, b2Body* newBodyB) {
    auto range = NSRange{nPoint, numPoints-nPoint-1};
    
    std::vector< VStick *> newRopeSticks( vSticks.begin() + range.location, vSticks.begin() + range.location + range.length );
    vSticks.erase(vSticks.begin() + range.location, vSticks.begin() + range.location + range.length);
    
    std::vector<Sprite*> newRopeSprites(ropeSprites.begin() + range.location, ropeSprites.begin() + range.location + range.length);
    ropeSprites.erase(ropeSprites.begin() + range.location, ropeSprites.begin() + range.location + range.length);
    
    range.length += 1;

    std::vector<VPoint*> newRopePoints(vPoints.begin() + range.location, vPoints.begin() + range.location + range.length);
    vPoints.erase(vPoints.begin() + range.location, vPoints.begin() + range.location + range.length);

    VPoint *pointOfBreak = newRopePoints.at(0);
    VPoint *newPoint = new VPoint();
    newPoint->setPos(pointOfBreak->x, pointOfBreak->y);
    vPoints.push_back(newPoint);

    VStick *lastStick = vSticks.back();
    lastStick->setPointB(newPoint);
 
    float cutRatio = (float)nPoint / (numPoints - 1);
    numPoints = nPoint + 1;

    b2Vec2 newBodiesPosition  = b2Vec2(pointOfBreak->x / PTM_RATIO, pointOfBreak->y / PTM_RATIO);

    b2World *world = newBodyA->GetWorld();

    b2RopeJointDef jd;
    jd.bodyA = joint->GetBodyA();
    jd.bodyB = newBodyB;
    jd.localAnchorA = joint->GetLocalAnchorA();
    jd.localAnchorB = b2Vec2(0, 0);
    jd.maxLength = joint->GetMaxLength() * cutRatio;
    newBodyB->SetTransform(newBodiesPosition, 0.0);
    b2RopeJoint *newJoint1 = (b2RopeJoint *)world->CreateJoint(&jd); //create joint

    jd.bodyA = newBodyA;
    jd.bodyB = joint->GetBodyB();
    jd.localAnchorA = b2Vec2(0, 0);
    jd.localAnchorB = joint->GetLocalAnchorB();
    jd.maxLength = joint->GetMaxLength() * (1 - cutRatio);
    newBodyA->SetTransform(newBodiesPosition, 0.0);
    b2RopeJoint *newJoint2 = (b2RopeJoint *)world->CreateJoint(&jd);
    
    world->DestroyJoint(joint);
    joint = newJoint1;

    VRope* newRope = new VRope(newJoint2,spriteSheet,newRopePoints,newRopeSticks,newRopeSprites);
    return newRope;
}

VRope::VRope(b2RopeJoint* joint, CCSpriteBatchNode* batchNode,std::vector<VPoint*> points,
        std::vector< VStick *> sticks, std::vector<Sprite*> sprites )
        : joint(joint), spriteSheet(batchNode), vPoints(points), vSticks(sticks), ropeSprites(sprites) {}