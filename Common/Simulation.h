#ifndef SIMULATION_H
#define SIMULATION_H

#include <cmath>
#include <array>
#include <algorithm>

#include <Eigen/Dense>
#include "Eigen/src/Core/Matrix.h"
#include "raylib.h"
#include "Utils.h" // For the random number generation

//==================================================================
inline float calcMagnitude(const Vector2& vec)
{
    float m = vec.x*vec.x + vec.y*vec.y;
    return m ? std::sqrt(m) : 0.0f;
}

inline float mapTo01(float value, float inMin, float inMax)
{
    return std::clamp((value - inMin) / (inMax - inMin), 0.0f, 1.0f);
}

//==================================================================
// General simulation parameters (screen size, gravity, etc.)
//==================================================================
class SimParams
{
public:
    float SCREEN_WIDTH = 800;
    float SCREEN_HEIGHT = 600;
    float GRAVITY = -0.05f;
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

    static constexpr float MAX_FUEL = 100.0f;

    // These are the state variables of the lander
    Vector2 mPos {0.0f, 0.0f};
    Vector2 mVel {0.0f, 0.0f};
    float   mFuel = MAX_FUEL;
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
                mVel.y += sp.VERTICAL_THRUST_POWER;
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
        mPos.x = std::clamp(mPos.x, -sp.SCREEN_WIDTH*0.6f, sp.SCREEN_WIDTH*0.6f);

        // Limit lander to the top of the area
        if (mPos.y > sp.SCREEN_HEIGHT) mPos.y = sp.SCREEN_HEIGHT;
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

    LandingPad(const SimParams& sp, uint64_t& seed)
        : sp(sp)
    {
        // Random position for the landing pad
        auto hsw = sp.SCREEN_WIDTH*0.5f;
        auto hpw = mPadWidth*0.5f;
        RandomGenerator rng(seed);
        mPos.x = rng.RandRange(-hsw + hpw, hsw - hpw);
        mPos.y = sp.GROUND_LEVEL;
        // Update seed for chaining with other generators
        seed = rng.NextU64();
    }

    // See if it's in the pad area and if it landed or crashed
    // (sets lander state appropriately)
    bool CheckPadLanding(Lander& lander)
    {
        const auto landerX = lander.mPos.x;
        const auto landerY = lander.mPos.y;
        // Check if lander is within landing pad bounds
        if (landerY <= mPos.y &&
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

    Terrain(const SimParams& sp, LandingPad& pad, uint64_t& seed)
        : sp(sp)
    {
        mGroundY = sp.GROUND_LEVEL;

        float segmentWidth = sp.SCREEN_WIDTH / SEGMENTS_N;
        RandomGenerator rng(seed);

        for (size_t i=0; i <= SEGMENTS_N; ++i)
        {
            mPoints[i].x = i * segmentWidth - sp.SCREEN_WIDTH*0.5f;

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
                mPoints[i].y = mGroundY + rng.RandRange(-10, 10);
            }
        }

        // Update seed for chaining with other generators
        seed = rng.NextU64();
    }

    // See if crashed on the terrain
    // (sets lander state appropriately)
    bool CheckTerrainCollision(Lander& lander)
    {
        if (lander.mStateIsCrashed || lander.mStateIsLanded)
            return false;

        if (lander.mPos.y <= mGroundY)
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
template<typename T, typename S, int stateCount, int actionCount>
concept GetBrainActionsFn = requires(T t, const Eigen::Vector<S, stateCount>& state, Eigen::Vector<S, actionCount>& actions)
{
    { t(state, actions) } -> std::same_as<void>;
};

class Simulation
{
public:
    SimParams   sp;
    Lander      mLander;
    LandingPad  mLandingPad;
    Terrain     mTerrain;
    static constexpr double mTimeStepS = 1.0 / 60.0;
    double      mElapsedTimeS = 0;

    static constexpr float MAX_TIME_S = 100.0f;

    float mMaxDistanceToPad = 0.0f;

    // Constructor
    Simulation(const SimParams& sp, uint64_t seed)
        : sp(sp)
        , mLander(sp, Vector2{0.0f, sp.SCREEN_HEIGHT * 0.75f})
        , mLandingPad(sp, seed)
        , mTerrain(sp, mLandingPad, seed)
    {
        auto w = sp.SCREEN_WIDTH;
        auto h = sp.SCREEN_HEIGHT;
        mMaxDistanceToPad = calcMagnitude({w, h});
    }

    // Execute one simulation step
    void AnimateSim(const GetBrainActionsFn<float, SIM_BRAINSTATE_N, SIM_BRAINACTION_N> auto& getBrainActions)
    {
        // Skip the simulation if lander is not active
        if (mLander.mStateIsCrashed || mLander.mStateIsLanded)
            return;

        mElapsedTimeS += mTimeStepS;

        // 1. Convert the simulation variables to a eigen vector for the brain input
        Eigen::Vector<float, SIM_BRAINSTATE_N> simState;
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
        Eigen::Vector<float, SIM_BRAINACTION_N> actions;
        getBrainActions(simState.data(), actions.data());

        // 3. Convert the brain actions to the simulation variables
        mLander.mControl_UpThrust = actions[SIM_BRAINACTION_UP] > 0.5f;
        mLander.mControl_LeftThrust = actions[SIM_BRAINACTION_LEFT] > 0.5f;
        mLander.mControl_RightThrust = actions[SIM_BRAINACTION_RIGHT] > 0.5f;

        mLander.AnimLander(); // Update lander
        mLandingPad.CheckPadLanding(mLander); // Check for landing
        mTerrain.CheckTerrainCollision(mLander); // Check for terrain collision
    }

    // Get the elapsed time in seconds
    double GetElapsedTimeS() const { return mElapsedTimeS; }

    // Check if the simulation is complete
    bool IsSimulationComplete() const
    {
        return mLander.mStateIsLanded || mLander.mStateIsCrashed;
    }

    // Calculate the score for the simulation
    double CalculateScore() const
    {
        double score = 1;

        // Calculate distance to pad center
        const auto landerPos = mLander.mPos;
        const auto padPos = mLandingPad.mPos;
        const auto distanceToPad = calcMagnitude({padPos.x - landerPos.x,
                                                  padPos.y - landerPos.y});

        score += 1 * (1 - mapTo01(distanceToPad, 0.0f, mMaxDistanceToPad)); // Penalize distance to pad
        //score -= 0.1 * mapTo01(mElapsedTimeS, 0.0f, MAX_TIME_S); // Penalize time
        //score += 0.1 * mapTo01(mLander.mFuel, 0.0f, Lander::MAX_FUEL); // Bonus for fuel
        auto speed = calcMagnitude(mLander.mVel);
        score += 0.1 * (1 - mapTo01(speed, 0.0f, sp.LANDING_SAFE_SPEED)); // Bonus for soft landing

        if (mLander.mStateIsLanded)
            score += 1;

        if (mLander.mStateIsCrashed)
            score -= 1;

        return score * 10; // Scale for readability
    }
};

#endif
