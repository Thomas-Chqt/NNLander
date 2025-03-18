#include "raylib.h"
#include "rlgl.h"
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

    void DrawLander()
    {
        // Draw lander
        Color landerColor = WHITE;
        if (state == LANDED) landerColor = GREEN;
        if (state == CRASHED) landerColor = RED;

        const auto drawX = position.x;
        const auto drawY = position.y - 20;

        // Main body
        DrawRectangle(drawX - 15, drawY - 15, 30, 30, landerColor);

        // Landing legs
        DrawLine(drawX - 15, drawY + 15, drawX - 25, drawY + 25, landerColor);
        DrawLine(drawX + 15, drawY + 15, drawX + 25, drawY + 25, landerColor);

        // Do not draw flame triangles if lander is not playing or out of fuel
        if (state != PLAYING || fuel <= 0) return;

        // Bottom thruster (UP key)
        if (thrustingUp)
        {
            DrawTriangle(
                {drawX - 8, drawY + 15},
                {drawX + 8, drawY + 15},
                {drawX, drawY + 25 + GetRandomValue(0, 5)},
                ORANGE
            );
        }

        // Right thruster (LEFT key)
        if (thrustingLeft)
        {
            DrawTriangle(
                {drawX + 15, drawY - 8},
                {drawX + 15, drawY + 8},
                {drawX + 25 + GetRandomValue(0, 5), drawY},
                ORANGE
            );
        }

        // Left thruster (RIGHT key)
        if (thrustingRight)
        {
            DrawTriangle(
                {drawX - 15, drawY - 8},
                {drawX - 15, drawY + 8},
                {drawX - 25 - GetRandomValue(0, 5), drawY},
                ORANGE
            );
        }
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

    bool CheckPadLanding(Lander& lander)
    {
        // Check if lander is within landing pad bounds
        if (lander.position.y >= position.y &&
            lander.position.x >= position.x - width/2 &&
            lander.position.x <= position.x + width/2)
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

    void DrawLandingPad()
    {
        DrawRectangle(position.x - width/2, position.y, width, 10, GREEN);

        // Draw landing lights
        for (int i = 0; i < 5; ++i)
        {
            float x = position.x - width/2 + (width/4) * i;
            DrawRectangle(x, position.y - 5, 3, 5, YELLOW);
        }
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

    void DrawTerrain()
    {
        for (int i = 0; i < SEGMENTS_N; ++i)
        {
            DrawLineEx(points[i], points[i + 1], 2.0f, DARKBROWN);
            // Fill terrain below
            const auto p0 = points[i];
            const auto p1 = points[i + 1];
            const auto p2 = Vector2{p0.x, sp.SCREEN_HEIGHT};
            const auto p3 = Vector2{p1.x, sp.SCREEN_HEIGHT};
            //const auto p4 = Vector2{p0.x, sp.SCREEN_HEIGHT};
            DrawTriangle(p0, p1, p2, BROWN);
            DrawTriangle(p1, p3, p2, BROWN);
        }
    }

    bool CheckTerrainCollision(Lander& lander)
    {
        if (lander.state != PLAYING) return false;

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

    void DrawSim()
    {
        // Draw terrain
        terrain.DrawTerrain();

        // Draw landing pad
        landingPad.DrawLandingPad();

        // Draw lander
        lander.DrawLander();
    }
};
