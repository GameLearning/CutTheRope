/* 
 * File:   MyContactListener.h
 * Author: thedreamer
 *
 * Created on November 6, 2014, 4:03 PM
 */

#ifndef MYCONTACTLISTENER_H
#define	MYCONTACTLISTENER_H

#include "Box2D/Box2D.h"
//#import <vector>
//#import <algorithm>
 
struct MyContact {
    b2Fixture *fixtureA;
    b2Fixture *fixtureB;
    bool operator==(const MyContact& other) const
    {
        return (fixtureA == other.fixtureA) && (fixtureB == other.fixtureB);
    }
};
 
class MyContactListener : public b2ContactListener {
 
public:
    std::vector<MyContact>_contacts;
 
    MyContactListener();
    ~MyContactListener();
 
    virtual void BeginContact(b2Contact* contact);
    virtual void EndContact(b2Contact* contact);
    virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);    
    virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
 
};

#endif	/* MYCONTACTLISTENER_H */

