#ifndef SIMULATION_H
#define SIMULATION_H

#include "raylib.h"
#include <cmath>
#include <string>
#include <algorithm>

//==================================================================
// Simulation parameters
//==================================================================
class SimParams
{
public:
    float SCREEN_WIDTH = 800;
    float SCREEN_HEIGHT = 600;
    float GRAVITY = 0.05f;
    float VERTICAL_THRUST_POWER = 0.1f;
    float LATERAL_THRUST_POWER = 0.08f;
    float LANDING_SAFE_SPEED = 1.5f;
    float GROUND_LEVEL = 30.0f;
};

//==================================================================
// Game states
//==================================================================
enum GameState { PLAYING, LANDED, CRASHED };

//==================================================================
// Lander class
//==================================================================
class Lander
{
    SimParams sp;
public:
    Vector2 mPos { 0.0f, 0.0f };
    Vector2 mVel { 0.0f, 0.0f };
    float mFuel = 100.0f;
    bool mIsThrustUpActive = false;
    bool mIsThrustLeftActive = false;
    bool mIsThrustRightActive = false;
    GameState state = PLAYING;

    Lander(const SimParams& sp, const Vector2& pos)
        : sp(sp)
        , mPos(pos)
    {}

    void AnimLander()
    {
        if (state != PLAYING) return;

        // Apply gravity
        mVel.y += sp.GRAVITY;

        // Apply vertical thrust
        if (mFuel > 0)
        {
            if (mIsThrustUpActive) // Vertical thrust
            {
                mVel.y -= sp.VERTICAL_THRUST_POWER;
                mFuel -= 0.5f; // Consume fuel
            }

            if (mIsThrustLeftActive) // Lateral thrusts
            {
                mVel.x -= sp.LATERAL_THRUST_POWER;
                mFuel -= 0.3f; // Consume fuel
            }

            if (mIsThrustRightActive) // Lateral thrusts
            {
                mVel.x += sp.LATERAL_THRUST_POWER;
                mFuel -= 0.3f; // Consume fuel
            }
        }

        // Ensure fuel doesn't go negative
        if (mFuel < 0) mFuel = 0;

        // Update position
        mPos.x += mVel.x;
        mPos.y += mVel.y;

        // Limit lander to screen edges
        mPos.x = std::clamp(mPos.x, 0.0f, (float)sp.SCREEN_WIDTH);

        // Check bounds for y-axis
        if (mPos.y < 0) mPos.y = 0;
    }

    float CalcSpeed()
    {
        return sqrt(mVel.x*mVel.x + mVel.y*mVel.y);
    }
};

//==================================================================
// Landing pad class
//==================================================================
class LandingPad
{
    SimParams sp;
public:
    Vector2 mPos {0.0f, 0.0f};
    float mPadWidth = 100.0f;

    LandingPad(const SimParams& sp)
        : sp(sp)
    {
        mPos.x = GetRandomValue(mPadWidth, sp.SCREEN_WIDTH - mPadWidth);
        mPos.y = sp.SCREEN_HEIGHT - sp.GROUND_LEVEL;
    }

    // See if it's in the pad area and if it landed or crashed
    // (sets lander state appropriately)
    bool CheckPadLanding(Lander& lander)
    {
        const auto landerX = lander.mPos.x;
        const auto landerY = lander.mPos.y;
        // Check if lander is within landing pad bounds
        if (landerY >= mPos.y &&
            landerX >= mPos.x - mPadWidth/2 &&
            landerX <= mPos.x + mPadWidth/2)
        {
            // Check landing speed
            if (lander.CalcSpeed() <= sp.LANDING_SAFE_SPEED)
                lander.state = LANDED; // Successful landing
            else
                lander.state = CRASHED; // Crash

            return true; // Done
        }
        return false; // Continue
    }
};

//==================================================================
// Terrain class
//==================================================================
class Terrain
{
public:
    SimParams sp;
public:
    static const size_t SEGMENTS_N = 10;
    Vector2 mPoints[SEGMENTS_N + 1];

    float mGroundY = 0;

    Terrain(const SimParams& sp, LandingPad& pad)
        : sp(sp)
    {
        mGroundY = sp.SCREEN_HEIGHT - sp.GROUND_LEVEL;

        float segmentWidth = sp.SCREEN_WIDTH / SEGMENTS_N;

        for (size_t i=0; i <= SEGMENTS_N; ++i)
        {
            mPoints[i].x = i * segmentWidth;

            // Find landing pad segment
            float padLeftX = pad.mPos.x - pad.mPadWidth/2;
            float padRightX = pad.mPos.x + pad.mPadWidth/2;

            const auto isLandingPadArea =
                mPoints[i].x >= padLeftX - segmentWidth &&
                mPoints[i].x <= padRightX + segmentWidth;

            if (isLandingPadArea)
            {
                // Make flat area for landing pad
                mPoints[i].y = pad.mPos.y;
            }
            else
            {
                // Very gentle height variation for terrain
                // Only small variations from the ground level
                mPoints[i].y = mGroundY + GetRandomValue(-10, 10);
            }
        }
    }

    // See if crashed on the terrain
    // (sets lander state appropriately)
    bool CheckTerrainCollision(Lander& lander)
    {
        if (lander.state != PLAYING)
            return false;

        if (lander.mPos.y >= mGroundY)
        {
            lander.state = CRASHED;
            return true;
        }

        return false;
    }
};

//==================================================================
// Simulation class
//==================================================================
class Simulation
{
public:
    SimParams sp;
    Lander mLander;
    LandingPad landingPad;
    Terrain terrain;

    Simulation(const SimParams& sp)
        : sp(sp)
        , mLander(sp, Vector2{sp.SCREEN_WIDTH / 2.0f, sp.SCREEN_HEIGHT / 4.0f})
        , landingPad(sp)
        , terrain(sp, landingPad)
    {
    }

    void AnimateSim()
    {
        // Only animate if the lander is active
        if (mLander.state != PLAYING)
            return;

        // Update lander
        mLander.AnimLander();
        // Check for landing
        landingPad.CheckPadLanding(mLander);
        // Check for terrain collision
        terrain.CheckTerrainCollision(mLander);
    }
};

#endif
