#ifndef CAR_H
#define CAR_H

#include "map.h"

struct CarProfile {
    float horsepower;       
    float mass;             
    float dragCoeff;        
    float rollingCoeff;     
    float tireMu;           
    float weightDistrib;    
    float turnRadius;       
    float frontAero;
    float rearAero;
    float brake;
};

struct GripInfo {
    float loadFront;
    float loadRear;
    float aeroFront;
    float aeroRear;
    float effectiveFrontGrip;
    float effectiveRearGrip;
};


class Car {
private:
    Vector2 mPos;
    float mAngle;
    float mSpeed;
    float mVelocityAngle; 
    float mSteerAngle;      
    float mMaxSteer = 20.0f;  
    float mSteerSpeed = 20.0f; 
    float mSteerReturnSpeed = 20.0f; 

    Texture2D mTexture;
    Vector2 mScale;  
    Vector2 mVel = {0.0f, 0.0f};     

    CarProfile mProfile;
    GripInfo mGrip;

    void applyDrag(float dt);
    void applyDraft();
    void applySteering(float dt);
    void applyFriction(float dt);
    void handleSpeed();
    void handleTurn();

    void checkCollisionX(Map *map);
    void checkCollisionY(Map *map);
    void checkCollisionX(const std::vector<Car*> &cars);
    void checkCollisionY(const std::vector<Car*> &cars);
    void checkCollision(Map *map, const std::vector<Car*> &cars);
    bool isColliding(Car *other) const;
    void applyGrassPenalty(Map *map);

public:
    Car(Vector2 startPos,
        Vector2 scale,                
        const char* texturePath,
        CarProfile profile);
    ~Car();

    void updateGrip();
    
    void accelerate(float dt, Map* map);
    void brake(float dt);
    void reverse(float dt);
    void turnleft(float dt);
    void turnright(float dt);
    void update(float dt, Map *map, const std::vector<Car*> &cars);
    void render();
    void displayCollider();

    Vector2 getPosition() const { return mPos; }
    float getAngle() const { return mAngle; }
    float getSpeed() const { return mSpeed; }
    float getForwardSpeed() const;
    float getFrontGrip() const { return mGrip.effectiveFrontGrip; }
    float getSteerAngle() const { return mSteerAngle; }
    Vector2 getVelocity() const { return mVel; }
    float getWeight() const { return mProfile.mass; }

    void setAngle(float angle) { mAngle = angle; }
    void setSteerAngle(float angle) { mSteerAngle = angle; }

    
};

#endif
