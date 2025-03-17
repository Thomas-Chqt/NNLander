#include "raylib.h"
#include "rlgl.h"
#include <cmath>
#include <string>
#include <algorithm>

// Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float GRAVITY = 0.05f;
const float VERTICAL_THRUST_POWER = 0.1f;
const float LATERAL_THRUST_POWER = 0.08f;
const float LANDING_SAFE_SPEED = 1.5f;
const float GROUND_LEVEL = 30.0f;

// Game states
enum GameState { PLAYING, LANDED, CRASHED };

// Lander struct
struct Lander
{
    Vector2 position { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 4.0f };
    Vector2 velocity { 0.0f, 0.0f };
    float fuel = 100.0f;
    bool thrustingUp = false;
    bool thrustingLeft = false;
    bool thrustingRight = false;
    GameState state = PLAYING;

    void AnimLander()
    {
        if (state != PLAYING) return;

        // Apply gravity
        velocity.y += GRAVITY;

        // Apply vertical thrust
        if (fuel > 0)
        {
            if (thrustingUp) // Vertical thrust
            {
                velocity.y -= VERTICAL_THRUST_POWER;
                fuel -= 0.5f; // Consume fuel
            }

            if (thrustingLeft) // Lateral thrusts
            {
                velocity.x -= LATERAL_THRUST_POWER;
                fuel -= 0.3f; // Consume fuel
            }

            if (thrustingRight) // Lateral thrusts
            {
                velocity.x += LATERAL_THRUST_POWER;
                fuel -= 0.3f; // Consume fuel
            }
        }

        // Ensure fuel doesn't go negative
        if (fuel < 0) fuel = 0;

        // Update position
        position.x += velocity.x;
        position.y += velocity.y;

        // Limit lander to screen edges
        position.x = std::clamp(position.x, 0.0f, (float)SCREEN_WIDTH);

        // Check bounds for y-axis
        if (position.y < 0) position.y = 0;
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

// Landing pad struct
struct LandingPad
{
    Vector2 position {0,0};
    float width = 100.0f;

    LandingPad()
    {
        position.x = GetRandomValue(width, SCREEN_WIDTH - width);
        position.y = SCREEN_HEIGHT - GROUND_LEVEL;
    }

    void DrawLandingPad()
    {
        DrawRectangle(position.x - width/2, position.y, width, 10, GREEN);

        // Draw landing lights
        for (int i = 0; i < 5; i++)
        {
            float x = position.x - width/2 + (width/4) * i;
            DrawRectangle(x, position.y - 5, 3, 5, YELLOW);
        }
    }

    bool CheckPadLanding(Lander& lander)
    {
        // Check if lander is within landing pad bounds
        if (lander.position.y >= position.y &&
            lander.position.x >= position.x - width/2 &&
            lander.position.x <= position.x + width/2)
        {
            // Check landing speed
            const auto vx = lander.velocity.x;
            const auto vy = lander.velocity.y;
            const auto speed = sqrt(vx*vx + vy*vy);

            if (speed <= LANDING_SAFE_SPEED)
                lander.state = LANDED; // Successful landing
            else
                lander.state = CRASHED; // Crash

            return true; // Done
        }
        return false; // Continue
    }
};

// Terrain struct
struct Terrain
{
    static const int segments = 10;
    Vector2 points[segments + 1];
    float groundY = SCREEN_HEIGHT - GROUND_LEVEL;

    Terrain(LandingPad& pad)
    {
        float segmentWidth = SCREEN_WIDTH / segments;

        for (int i = 0; i <= segments; i++)
        {
            points[i].x = i * segmentWidth;

            // Find landing pad segment
            float padLeftX = pad.position.x - pad.width/2;
            float padRightX = pad.position.x + pad.width/2;

            if (points[i].x >= padLeftX - segmentWidth &&
                points[i].x <= padRightX + segmentWidth) {
                // Make flat area for landing pad
                points[i].y = pad.position.y;
            } else {
                // Very gentle height variation for terrain
                // Only small variations from the ground level
                points[i].y = groundY + GetRandomValue(-10, 10);
            }
        }

    }

    void DrawTerrain()
    {
        for (int i = 0; i < segments; ++i)
        {
            DrawLineEx(points[i], points[i + 1], 2.0f, DARKBROWN);
            // Fill terrain below
            const auto p0 = points[i];
            const auto p1 = points[i + 1];
            const auto p2 = Vector2{p0.x, SCREEN_HEIGHT};
            const auto p3 = Vector2{p1.x, SCREEN_HEIGHT};
            const auto p4 = Vector2{p0.x, SCREEN_HEIGHT};
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

int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander Simulation");
    SetTargetFPS(60);

    // Initialize game objects
    Lander lander;
    LandingPad landingPad;
    Terrain terrain(landingPad);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        if (lander.state == PLAYING)
        {
            // Handle input
            lander.thrustingUp = IsKeyDown(KEY_UP);
            lander.thrustingLeft = IsKeyDown(KEY_LEFT);
            lander.thrustingRight = IsKeyDown(KEY_RIGHT);

            // Update lander
            lander.AnimLander();

            // Check for landing
            landingPad.CheckPadLanding(lander);

            // Check for terrain collision
            terrain.CheckTerrainCollision(lander);
        }
        else
        {
            // Restart game on Space key
            if (IsKeyPressed(KEY_SPACE))
            {
                lander = Lander(); // Reset lander
                landingPad = LandingPad(); // Reset landing pad
                terrain = Terrain(landingPad); // Reset terrain
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        rlDisableBackfaceCulling(); // Don't care about triangles winding order

        // Draw terrain
        terrain.DrawTerrain();

        // Draw landing pad
        landingPad.DrawLandingPad();

        // Draw lander
        lander.DrawLander();

        // Draw UI
        DrawText(TextFormat("Fuel: %.0f%%", lander.fuel), 10, 10, 20, WHITE);
        DrawText(TextFormat("Speed: %.1f", sqrt(lander.velocity.x*lander.velocity.x + lander.velocity.y*lander.velocity.y)), 10, 40, 20, WHITE);

        // Draw game state message
        if (lander.state == LANDED)
        {
            DrawText("SUCCESSFUL LANDING!", SCREEN_WIDTH/2 - 150, 200, 30, GREEN);
            DrawText("Press SPACE to play again", SCREEN_WIDTH/2 - 150, 240, 20, WHITE);
        }
        else if (lander.state == CRASHED)
        {
            DrawText("CRASHED!", SCREEN_WIDTH/2 - 80, 200, 30, RED);
            DrawText("Press SPACE to try again", SCREEN_WIDTH/2 - 150, 240, 20, WHITE);
        }
        else
        {
            DrawText("UP: Vertical thrust, LEFT/RIGHT: Lateral thrusters", 10, SCREEN_HEIGHT - 30, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
