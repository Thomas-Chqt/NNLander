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
    Vector2 position { 0.0f, 0.0f };
    Vector2 velocity { 0.0f, 0.0f };
    float fuel = 100.0f;
    bool thrustingUp = false;
    bool thrustingLeft = false;
    bool thrustingRight = false;
    GameState state = PLAYING;

    Lander(const SimParams& sp, const Vector2& pos)
        : sp(sp)
        , position(pos)
    {}

    void AnimLander()
    {
        if (state != PLAYING) return;

        // Apply gravity
        velocity.y += sp.GRAVITY;

        // Apply vertical thrust
        if (fuel > 0)
        {
            if (thrustingUp) // Vertical thrust
            {
                velocity.y -= sp.VERTICAL_THRUST_POWER;
                fuel -= 0.5f; // Consume fuel
            }

            if (thrustingLeft) // Lateral thrusts
            {
                velocity.x -= sp.LATERAL_THRUST_POWER;
                fuel -= 0.3f; // Consume fuel
            }

            if (thrustingRight) // Lateral thrusts
            {
                velocity.x += sp.LATERAL_THRUST_POWER;
                fuel -= 0.3f; // Consume fuel
            }
        }

        // Ensure fuel doesn't go negative
        if (fuel < 0) fuel = 0;

        // Update position
        position.x += velocity.x;
        position.y += velocity.y;

        // Limit lander to screen edges
        position.x = std::clamp(position.x, 0.0f, (float)sp.SCREEN_WIDTH);

        // Check bounds for y-axis
        if (position.y < 0) position.y = 0;
    }

    float CalcSpeed()
    {
        return sqrt(velocity.x*velocity.x + velocity.y*velocity.y);
    }
};

//==================================================================
// Landing pad class
//==================================================================
class LandingPad
{
    SimParams sp;
public:
    Vector2 position {0.0f, 0.0f};
    float width = 100.0f;

    LandingPad(const SimParams& sp)
        : sp(sp)
    {
        position.x = GetRandomValue(width, sp.SCREEN_WIDTH - width);
        position.y = sp.SCREEN_HEIGHT - sp.GROUND_LEVEL;
    }

    // See if it's in the pad area and if it landed or crashed
    // (sets lander state appropriately)
    bool CheckPadLanding(Lander& lander)
    {
        const auto landerX = lander.position.x;
        const auto landerY = lander.position.y;
        // Check if lander is within landing pad bounds
        if (landerY >= position.y &&
            landerX >= position.x - width/2 &&
            landerX <= position.x + width/2)
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
    SimParams sp;
public:
    static const int SEGMENTS_N = 10;
    Vector2 points[SEGMENTS_N + 1];

    float groundY = 0;

    Terrain(const SimParams& sp, LandingPad& pad)
        : sp(sp)
    {
        groundY = sp.SCREEN_HEIGHT - sp.GROUND_LEVEL;

        float segmentWidth = sp.SCREEN_WIDTH / SEGMENTS_N;

        for (int i=0; i <= SEGMENTS_N; ++i)
        {
            points[i].x = i * segmentWidth;

            // Find landing pad segment
            float padLeftX = pad.position.x - pad.width/2;
            float padRightX = pad.position.x + pad.width/2;

            const auto isLandingPadArea =
                points[i].x >= padLeftX - segmentWidth &&
                points[i].x <= padRightX + segmentWidth;

            if (isLandingPadArea)
            {
                // Make flat area for landing pad
                points[i].y = pad.position.y;
            }
            else
            {
                // Very gentle height variation for terrain
                // Only small variations from the ground level
                points[i].y = groundY + GetRandomValue(-10, 10);
            }
        }
    }

    // See if crashed on the terrain
    // (sets lander state appropriately)
    bool CheckTerrainCollision(Lander& lander)
    {
        if (lander.state != PLAYING)
            return false;

        if (lander.position.y >= groundY)
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
    Lander lander;
    LandingPad landingPad;
    Terrain terrain;

    Simulation(const SimParams& sp)
        : sp(sp)
        , lander(sp, Vector2{sp.SCREEN_WIDTH / 2.0f, sp.SCREEN_HEIGHT / 4.0f})
        , landingPad(sp)
        , terrain(sp, landingPad)
    {
    }

    void AnimateSim()
    {
        // Only animate if the lander is active
        if (lander.state != PLAYING)
            return;

        // Update lander
        lander.AnimLander();
        // Check for landing
        landingPad.CheckPadLanding(lander);
        // Check for terrain collision
        terrain.CheckTerrainCollision(lander);
    }
};

