#pragma once 

class IntelliVibeData
{
public:
    unsigned char AAMissileFired; // how many AA missiles fired.
    unsigned char AGMissileFired; // how many maveric/rockets fired
    unsigned char BombDropped; // how many bombs dropped
    unsigned char FlareDropped; // how many flares dropped
    unsigned char ChaffDropped; // how many chaff dropped
    unsigned char BulletsFired; // how many bullets shot
    int CollisionCounter; // Collisions
    bool IsFiringGun; // gun is firing
    bool IsEndFlight; // Ending the flight from 3d
    bool IsEjecting; // we've ejected
    bool In3D; // In 3D?
    bool IsPaused; // sim paused?
    bool IsFrozen; // sim frozen?
    bool IsOverG; // are G limits being exceeded?
    bool IsOnGround; // are we on the ground
    bool IsExitGame; // Did we exit Falcon?
    float Gforce; // what gforce we are feeling
    float eyex; // where the eye is in relationship to the plane
    float eyey; // where the eye is in relationship to the plane
    float eyez; // where the eye is in relationship to the plane
    int lastdamage; // 1 to 8 depending on quadrant. 
    float damageforce; // how big the hit was.
    int whendamage;
};