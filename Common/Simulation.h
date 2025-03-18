#ifndef SIMULATION_H
#define SIMULATION_H

#include "raylib.h"
#include <cmath>
#include <string>
#include <array>
#include <algorithm>
#include <functional>

//==================================================================
// General simulation parameters (screen size, gravity, etc.)
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

// Indices of states in the simulation state array
enum SimBrainState
{
    SIM_BRAINSTATE_LANDER_X = 0,
    SIM_BRAINSTATE_LANDER_Y,
    SIM_BRAINSTATE_LANDER_VX,
    SIM_BRAINSTATE_LANDER_VY,
    SIM_BRAINSTATE_LANDER_FUEL,
    SIM_BRAINSTATE_LANDER_STATE_LANDED,
    SIM_BRAINSTATE_LANDER_STATE_CRASHED,
    SIM_BRAINSTATE_PAD_X,
    SIM_BRAINSTATE_PAD_Y,
    SIM_BRAINSTATE_PAD_WIDTH,
    SIM_BRAINSTATE_N
};

// Indices of actions from the brain
enum SimBrainAction
{
    SIM_BRAINACTION_UP = 0,
    SIM_BRAINACTION_LEFT,
    SIM_BRAINACTION_RIGHT,
    SIM_BRAINACTION_N
};

//==================================================================
// Lander class
//==================================================================
class Lander
{
    SimParams sp;
public:
    // These are the controls to apply to the lander
    // They may come from the user or from an artificial brain
    bool    mControl_UpThrust = false;
    bool    mControl_LeftThrust = false;
    bool    mControl_RightThrust = false;

    // These are the state variables of the lander
    Vector2 mPos {0.0f, 0.0f};
    Vector2 mVel {0.0f, 0.0f};
    float   mFuel = 100.0f;
    bool    mStateIsLanded = false;
    bool    mStateIsCrashed = false;

    Lander(const SimParams& sp, const Vector2& pos)
        : sp(sp)
        , mPos(pos)
    {}

    void AnimLander()
    {
        // Do not animate if lander is crashed or landed
        if (mStateIsCrashed || mStateIsLanded)
            return;

        // Apply gravity
        mVel.y += sp.GRAVITY;

        // Apply vertical thrust
        if (mFuel > 0)
        {
            if (mControl_UpThrust) // Vertical thrust
            {
                mVel.y -= sp.VERTICAL_THRUST_POWER;
                mFuel -= 0.5f; // Consume fuel
            }

            if (mControl_LeftThrust) // Lateral thrusts
            {
                mVel.x -= sp.LATERAL_THRUST_POWER;
                mFuel -= 0.3f; // Consume fuel
            }

            if (mControl_RightThrust) // Lateral thrusts
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
    float   mPadWidth = 100.0f;

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
                lander.mStateIsLanded = true; // Landed
            else
                lander.mStateIsCrashed = true; // Crashed

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
        if (lander.mStateIsCrashed || lander.mStateIsLanded)
            return false;

        if (lander.mPos.y >= mGroundY)
        {
            lander.mStateIsCrashed = true;
            return true;
        }

        return false;
    }
};

//==================================================================
// Simulation class
//==================================================================
using GetBrainActionsFnT =
    std::function<void(const float*, size_t, float*, size_t)>;

class Simulation
{
public:
    SimParams   sp;
    Lander      mLander;
    LandingPad  mLandingPad;
    Terrain     mTerrain;

    Simulation(const SimParams& sp)
        : sp(sp)
        , mLander(sp, Vector2{sp.SCREEN_WIDTH / 2.0f, sp.SCREEN_HEIGHT / 4.0f})
        , mLandingPad(sp)
        , mTerrain(sp, mLandingPad)
    {
    }

    void AnimateSim(const GetBrainActionsFnT& getBrainActions)
    {
        // Skip the simulation if lander is not active
        if (mLander.mStateIsCrashed || mLander.mStateIsLanded)
            return;

        // 1. Convert the simulation variables to a simple/flat array for the brain input
        std::array<float, SIM_BRAINSTATE_N> simState {};
        simState[SIM_BRAINSTATE_LANDER_X] = mLander.mPos.x;
        simState[SIM_BRAINSTATE_LANDER_Y] = mLander.mPos.y;
        simState[SIM_BRAINSTATE_LANDER_VX] = mLander.mVel.x;
        simState[SIM_BRAINSTATE_LANDER_VY] = mLander.mVel.y;
        simState[SIM_BRAINSTATE_LANDER_FUEL] = mLander.mFuel;
        simState[SIM_BRAINSTATE_LANDER_STATE_LANDED] = mLander.mStateIsLanded;
        simState[SIM_BRAINSTATE_LANDER_STATE_CRASHED] = mLander.mStateIsCrashed;
        simState[SIM_BRAINSTATE_PAD_X] = mLandingPad.mPos.x;
        simState[SIM_BRAINSTATE_PAD_Y] = mLandingPad.mPos.y;
        simState[SIM_BRAINSTATE_PAD_WIDTH] = mLandingPad.mPadWidth;

        // 2. Get the brain actions
        std::array<float, SIM_BRAINACTION_N> actions {};
        getBrainActions(
            simState.data(), std::size(simState),
            actions.data(), std::size(actions)
        );

        // 3. Convert the brain actions to the simulation variables
        mLander.mControl_UpThrust = actions[SIM_BRAINACTION_UP] > 0.5f;
        mLander.mControl_LeftThrust = actions[SIM_BRAINACTION_LEFT] > 0.5f;
        mLander.mControl_RightThrust = actions[SIM_BRAINACTION_RIGHT] > 0.5f;

        mLander.AnimLander(); // Update lander
        mLandingPad.CheckPadLanding(mLander); // Check for landing
        mTerrain.CheckTerrainCollision(mLander); // Check for terrain collision
    }
};

#endif
