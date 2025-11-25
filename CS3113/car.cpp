#include "car.h"
#include <cmath>

Car::Car(Vector2 startPos,
         Vector2 scale,
         const char* texturePath,
         CarProfile profile) {
    mPos = startPos;
    mScale = scale;
    mAngle = 0.0f;

    mSpeed = 0.0f;
    mVelocityAngle = 0.0f;
    mSteerAngle = 0.0f;

    mVel = {0.0f, 0.0f};

    mTexture = LoadTexture(texturePath);
    mProfile = profile;
}

Car::~Car() {
    UnloadTexture(mTexture);
}

void Car::updateGrip() {
    float g = 9.81f;

    float speed = mSpeed;

    mGrip.aeroFront = mProfile.frontAero * (speed * speed);
    mGrip.aeroRear  = mProfile.rearAero  * (speed * speed);

    mGrip.loadFront = mProfile.weightDistrib * mProfile.mass * g + mGrip.aeroFront;
    mGrip.loadRear  = (1.0f - mProfile.weightDistrib) * mProfile.mass * g + mGrip.aeroRear;

    float mu = mProfile.tireMu;

    mGrip.effectiveFrontGrip = mu * mGrip.loadFront;
    mGrip.effectiveRearGrip  = mu * mGrip.loadRear;
}

void Car::accelerate(float dt, Map* map) {
    float accel = (mProfile.horsepower * 1500.0f) / mProfile.mass;

    int tileID = map->getTileAtWorldPos(mPos);
    if (tileID == 0) {
        //grass penalty
        accel *= 0.5f;
    }

    float rad = mAngle * DEG2RAD;
    Vector2 forward = { std::cos(rad), std::sin(rad) };

    mVel.x += forward.x * accel * dt;
    mVel.y += forward.y * accel * dt;
}


void Car::reverse(float dt) {
    float accel = (mProfile.horsepower * 730.0f) / mProfile.mass * 0.3f;

    float rad = mAngle * DEG2RAD;
    Vector2 forward = { std::cos(rad), std::sin(rad) };

    mVel.x -= forward.x * accel * dt;
    mVel.y -= forward.y * accel * dt;
}

void Car::applyDrag(float dt) {
    float speed = mSpeed;
    if (speed <= 0.0f) return;

    float drag = mProfile.dragCoeff * speed * speed;
    float roll = mProfile.rollingCoeff * speed;

    float decel = (drag + roll) / mProfile.mass;

    float newSpeed = speed - decel * dt;
    if (newSpeed < 0) newSpeed = 0;

    if (speed > 0) {
        float scale = newSpeed / speed;
        mVel.x *= scale;
        mVel.y *= scale;
    }
}

void Car::applySteering(float dt) {
    // return steering to center when no input
    if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) {
        if (mSteerAngle > 0)        mSteerAngle -= mSteerReturnSpeed * dt;
        else if (mSteerAngle < 0)   mSteerAngle += mSteerReturnSpeed * dt;

        if (std::fabs(mSteerAngle) < 0.2f)
            mSteerAngle = 0;
    }

    if (mSpeed < 0.1f) return;

    // get car orientation
    float rad = mAngle * DEG2RAD;
    Vector2 carForward = { std::cos(rad), std::sin(rad) };

    // calculate front wheel direction
    float frontWheelAngle = rad + (mSteerAngle * DEG2RAD);
    Vector2 frontWheelDir = { std::cos(frontWheelAngle), std::sin(frontWheelAngle) };
    float steerRad = mSteerAngle * DEG2RAD;

    if (std::abs(steerRad) > 0.01f) {
        float angVel = (mSpeed * std::sin(steerRad)) / mProfile.turnRadius;
        mAngle += angVel * RAD2DEG * dt;
    }
}

void Car::applyFriction(float dt) {
    //  damping
    float frictionFactor = 0.998f;
    mVel.x *= frictionFactor;
    mVel.y *= frictionFactor;

    if (mSpeed < 0.1f) return;

    float rad = mAngle * DEG2RAD;
    Vector2 carForward = { std::cos(rad), std::sin(rad) };
    Vector2 carLateral = { -carForward.y, carForward.x };

    float velForward = mVel.x * carForward.x + mVel.y * carForward.y;
    float velLateral = mVel.x * carLateral.x + mVel.y * carLateral.y;

    // front wheels
    float frontWheelAngle = rad + (mSteerAngle * DEG2RAD);
    Vector2 frontDir = { std::cos(frontWheelAngle), std::sin(frontWheelAngle) };
    Vector2 frontLateral = { -frontDir.y, frontDir.x };

    float frontSlipVel = mVel.x * frontLateral.x + mVel.y * frontLateral.y;

    // rear wheels
    float rearSlipVel = velLateral;

    float frontMaxGrip = (mGrip.effectiveFrontGrip / mProfile.mass) * dt * 5.0f;
    float rearMaxGrip = (mGrip.effectiveRearGrip / mProfile.mass) * dt * 5.0f;

    const float tireStiffness = 0.15f;  // how much tires resist slip

    float frontForce = -frontSlipVel * tireStiffness;
    if (std::abs(frontForce) > frontMaxGrip) {
        frontForce = std::copysign(frontMaxGrip, frontForce);
    }

    float rearForce = -rearSlipVel * tireStiffness;
    if (std::abs(rearForce) > rearMaxGrip) {
        rearForce = std::copysign(rearMaxGrip, rearForce);
    }

    // apply forces in lateral directions
    mVel.x += frontLateral.x * frontForce * mProfile.weightDistrib;
    mVel.y += frontLateral.y * frontForce * mProfile.weightDistrib;

    mVel.x += carLateral.x * rearForce * (1.0f - mProfile.weightDistrib);
    mVel.y += carLateral.y * rearForce * (1.0f - mProfile.weightDistrib);
}

void Car::handleSpeed() {
    mSpeed = std::sqrt(mVel.x*mVel.x + mVel.y*mVel.y);

}

float Car::getForwardSpeed() const{
    float rad = mAngle * DEG2RAD;
    Vector2 carForward = { std::cos(rad), std::sin(rad)};

    return mVel.x * carForward.x + mVel.y * carForward.y;
}

void Car::handleTurn() {
    if (mSteerAngle > mMaxSteer) mSteerAngle = mMaxSteer;
    if (mSteerAngle < -mMaxSteer) mSteerAngle = -mMaxSteer;
}


void Car::brake(float dt) {
    float speed = mSpeed;
    if (speed <= 0.1f) {
        mVel.x = 0;
        mVel.y = 0;
        mSpeed = 0;
        return;
    }

    float g = 9.81f;

    float aeroFront = mProfile.frontAero * (speed * speed);
    float aeroRear  = mProfile.rearAero  * (speed * speed);

    float loadFront = mProfile.weightDistrib * mProfile.mass * g + aeroFront;
    float loadRear  = (1.0f - mProfile.weightDistrib) * mProfile.mass * g + aeroRear;

    // max brake force
    float maxBrakeForce = mProfile.tireMu * (loadFront + loadRear);

    float brakeForce = mProfile.brake;
    if (brakeForce > maxBrakeForce) {
        brakeForce = maxBrakeForce;
    }

    float decel_ms2 = brakeForce / mProfile.mass;
    const float MS_TO_GAME_UNITS = 36.0f;
    float decel_units = decel_ms2 * MS_TO_GAME_UNITS;

    float brakeReduction = decel_units * dt;

    if (brakeReduction >= speed) {
        mVel.x = 0;
        mVel.y = 0;
    } else {
        float scale = (speed - brakeReduction) / speed;
        mVel.x *= scale;
        mVel.y *= scale;
    }
}

void Car::turnleft(float dt) {
    mSteerAngle -= mSteerSpeed * dt;
}

void Car::turnright(float dt) {
    mSteerAngle += mSteerSpeed * dt;
}

void Car::update(float dt, Map *map, const std::vector<Car*> &cars) {
    // update speed for physics
    handleSpeed();
    updateGrip();
    applyGrassPenalty(map);
    applyFriction(dt);

    // reupdate for drag
    handleSpeed();
    applyDrag(dt);

    // update position and check collisions
    mPos.x += mVel.x * dt;
    checkCollisionX(cars);
    checkCollisionX(map);

    mPos.y += mVel.y * dt;
    checkCollisionY(cars);
    checkCollisionY(map);

    // update speed for steering
    handleSpeed();
    handleTurn();
    applySteering(dt);
}

void Car::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    Vector2 topCentreProbe    = { mPos.x, mPos.y - (mScale.y / 2.0f) };
    Vector2 topLeftProbe      = { mPos.x - (mScale.x / 2.0f), mPos.y - (mScale.y / 2.0f) };
    Vector2 topRightProbe     = { mPos.x + (mScale.x / 2.0f), mPos.y - (mScale.y / 2.0f) };

    Vector2 bottomCentreProbe = { mPos.x, mPos.y + (mScale.y / 2.0f) };
    Vector2 bottomLeftProbe   = { mPos.x - (mScale.x / 2.0f), mPos.y + (mScale.y / 2.0f) };
    Vector2 bottomRightProbe  = { mPos.x + (mScale.x / 2.0f), mPos.y + (mScale.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // above collision
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap))
         && mVel.y < 0.0f)
    {
        mPos.y += yOverlap * 1.01f;   // push down
        mVel.y = 0.0f;
    }

    // below collision
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap))
         && mVel.y > 0.0f)
    {
        mPos.y -= yOverlap * 1.01f;   // push up
        mVel.y = 0.0f;
    }
}

void Car::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    Vector2 leftCentreProbe   = { mPos.x - (mScale.x / 2.0f), mPos.y };
    Vector2 leftTopProbe      = { mPos.x - (mScale.x / 2.0f), mPos.y - (mScale.y / 2.0f) };
    Vector2 leftBottomProbe   = { mPos.x - (mScale.x / 2.0f), mPos.y + (mScale.y / 2.0f) };

    Vector2 rightCentreProbe  = { mPos.x + (mScale.x / 2.0f), mPos.y };
    Vector2 rightTopProbe     = { mPos.x + (mScale.x / 2.0f), mPos.y - (mScale.y / 2.0f) };
    Vector2 rightBottomProbe  = { mPos.x + (mScale.x / 2.0f), mPos.y + (mScale.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // right collision
    if ((map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightTopProbe, &xOverlap, &yOverlap)    ||
         map->isSolidTileAt(rightBottomProbe, &xOverlap, &yOverlap))
         && mVel.x > 0.0f)
    {
        mPos.x -= xOverlap * 1.01f;   // push left
        mVel.x = 0.0f;
    }

    // left collision
    if ((map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftTopProbe, &xOverlap, &yOverlap)    ||
         map->isSolidTileAt(leftBottomProbe, &xOverlap, &yOverlap))
         && mVel.x < 0.0f)
    {
        mPos.x += xOverlap * 1.01f;   // push right
        mVel.x = 0.0f;
    }
}

void Car::checkCollisionY(const std::vector<Car*> &cars)
{
    for (size_t i = 0; i < cars.size(); i++)
    {
        Car *otherCar = cars[i];

        if (isColliding(otherCar))
        {
            float yDistance = fabs(mPos.y - otherCar->mPos.y);
            float yOverlap  = fabs(yDistance - (mScale.y / 2.0f) - (otherCar->mScale.y / 2.0f));

            if (mVel.y > 0.0f)
            {
                mPos.y -= yOverlap * 1.01f;
            }
            else if (mVel.y < 0.0f)
            {
                mPos.y += yOverlap * 1.01f;
            }
        }
    }
}

void Car::checkCollisionX(const std::vector<Car*> &cars)
{
    for (size_t i = 0; i < cars.size(); i++)
    {
        Car *otherCar = cars[i];

        if (isColliding(otherCar))
        {
            float xDistance = fabs(mPos.x - otherCar->mPos.x);
            float xOverlap  = fabs(xDistance - (mScale.x / 2.0f) - (otherCar->mScale.x / 2.0f));

            if (mVel.x > 0.0f)
            {
                mPos.x -= xOverlap * 1.01f;
            }
            else if (mVel.x < 0.0f)
            {
                mPos.x += xOverlap * 1.01f;
            }
        }
    }
}

bool Car::isColliding(Car *other) const
{
    if (other == this) return false;

    float xDistance = fabs(mPos.x - other->getPosition().x) -
        ((mScale.x + other->mScale.x) / 2.0f);
    float yDistance = fabs(mPos.y - other->getPosition().y) -
        ((mScale.y + other->mScale.y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Car::checkCollision(Map *map, const std::vector<Car*> &cars)
{
    checkCollisionX(cars);
    checkCollisionX(map);

    checkCollisionY(cars);
    checkCollisionY(map);
}

void Car::applyGrassPenalty(Map *map) {
    //handle just grip portion
    if (map == nullptr) return;
    int tileID = map->getTileAtWorldPos(mPos);

    if (tileID == 0) {
        mGrip.effectiveFrontGrip *= 0.2f;
        mVel.x *= 0.999f;
        mVel.y *= 0.999f;
    }
}

void Car::render()
{
    Rectangle src = {0,0,(float)mTexture.width,(float)mTexture.height};

    Rectangle dst = {
        mPos.x,
        mPos.y,
        mScale.x,
        mScale.y
    };

    Vector2 origin = { mScale.x*0.5f, mScale.y*0.5f };

    DrawTexturePro(mTexture, src, dst, origin, mAngle, WHITE);
}
