#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <limits>

#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"
#include "SimpleNeuralNet.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static const float RESTART_DELAY = 2.0f;
static float restartTimer = 0.0f;

// Forward declarations
class TrainingTask;

static void drawUI(Simulation& sim, TrainingTask& trainingTask);

//==================================================================
// Network configuration
//==================================================================
static const std::vector<int> NETWORK_ARCHITECTURE = {
    SIM_BRAINSTATE_N,             // Input layer: simulation state variables
    (int)(SIM_BRAINSTATE_N*1.25), // Hidden layer
    (int)(SIM_BRAINSTATE_N*1.25), // Hidden layer
    SIM_BRAINACTION_N             // Output layer: actions (up, left, right)
};

//==================================================================
// TrainingTask class - handles neural network training
//==================================================================
class TrainingTask
{
private:
    // Training parameters
    static const int MAX_TRAINING_ITERATIONS = 50000;
    int mCurrentTrainingIteration = 0;
    float mBestLoss = std::numeric_limits<float>::max();
    std::vector<float> mBestNetworkParameters;
    std::vector<int> mNetworkArchitecture;
    SimParams mSimParams;

public:
    TrainingTask(const SimParams& sp, const std::vector<int>& architecture)
        : mNetworkArchitecture(architecture)
        , mSimParams(sp)
    {
    }

    // Run a single training iteration
    void RunIteration()
    {
        // Create a temporary neural network with random parameters
        SimpleNeuralNet tempNet(mNetworkArchitecture);
        const size_t paramCount = tempNet.GetTotalParameters();

        // Generate random parameters if we don't have best parameters yet
        if (mBestNetworkParameters.empty())
            mBestNetworkParameters.resize(paramCount); // Initializes to 0

        // Generate random parameters for this iteration
        const auto currentParams = GenerateRandomParameters(paramCount, mCurrentTrainingIteration);

        // Create a neural network with the random parameters
        SimpleNeuralNet net(mNetworkArchitecture);

        // Create a simulation for training
        uint32_t seed = 1134; // Initial random seed
        Simulation trainingSim(mSimParams, seed);

        // Lambda to handle actions using our neural network
        auto getActions = [&](const float* states, size_t, float* actions, size_t)
        {
            net.FeedForward(currentParams.data(), states, 0, actions, 0);
        };

        // Run the simulation until it ends
        double elapsedTimeS = 0.0;
        const double timeStepS = 1.0 / 60.0; // Assuming 60 FPS

        while (!trainingSim.mLander.mStateIsLanded &&
               !trainingSim.mLander.mStateIsCrashed &&
               elapsedTimeS < 30.0) // Maximum 30 seconds simulation time
        {
            trainingSim.AnimateSim(getActions);
            elapsedTimeS += timeStepS;
        }

        // Calculate loss for this run
        float currentLoss = CalculateLoss(trainingSim, elapsedTimeS);

        // If this network is better than our current best, save it
        if (currentLoss < mBestLoss)
        {
            mBestLoss = currentLoss;
            mBestNetworkParameters = currentParams;
        }

        // Increment the training iteration counter
        ++mCurrentTrainingIteration;
    }

    // Calculate loss for a simulation run
    float CalculateLoss(const Simulation& sim, double elapsedTimeS)
    {
        double loss = 0;

        // Calculate distance to pad center
        const auto landerPos = sim.mLander.mPos;
        const auto padPos = sim.mLandingPad.mPos;
        const auto distanceToPad =
            std::sqrt(std::pow(landerPos.x - padPos.x, 2) +
                      std::pow(landerPos.y - padPos.y, 2));

        loss += distanceToPad; // Penalize distance to pad
        loss *= (1 + elapsedTimeS); // Penalize time

        if (sim.mLander.mStateIsLanded)
            loss /= 10.0; // Bonus for successful landing

        if (sim.mLander.mStateIsCrashed)
            loss *= 10.0; // Penalty for crashing

        return (float)loss;
    }

    // Generate random parameters for neural network
    std::vector<float> GenerateRandomParameters(size_t paramCount, uint32_t seed = 0)
    {
        // Use a random number generator to create random parameters
        // between -1.0 and 1.0
        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        // Generate random parameters
        std::vector<float> params(paramCount);
        for (size_t i = 0; i < paramCount; ++i)
            params[i] = dist(rng);

        return params;
    }

    const auto& GetBestNetworkParameters() const { return mBestNetworkParameters; }

    // Getters for training status
    int GetCurrentIteration() const { return mCurrentTrainingIteration; }
    int GetMaxIterations() const { return MAX_TRAINING_ITERATIONS; }
    float GetBestLoss() const { return mBestLoss; }
    bool IsTrainingComplete() const { return mCurrentTrainingIteration >= MAX_TRAINING_ITERATIONS; }
};

// Global TrainingTask instance
static TrainingTask* gpTrainingTask = nullptr;

//==================================================================
// Main function
//==================================================================
int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander - Neural Network Training Demo");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    uint32_t seed = 1134; // Initial random seed
    Simulation sim(sp, seed);

    // Create the training task
    TrainingTask trainingTask(sp, NETWORK_ARCHITECTURE);
    gpTrainingTask = &trainingTask;  // Set global pointer to access from callback

    // Neural net object used for testing (will use the best params as they come
    // from the ongoing training)
    SimpleNeuralNet testNet(NETWORK_ARCHITECTURE);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Run training iterations in the background
        if (!trainingTask.IsTrainingComplete())
        {
            // Run a small batch, so we don't block the main thread
            for (int i=0; i < 25; ++i)
                trainingTask.RunIteration();
        }

        float deltaTime = GetFrameTime();

        // Auto-restart after landing or crashing
        if (sim.mLander.mStateIsLanded || sim.mLander.mStateIsCrashed)
        {
            restartTimer += deltaTime;
            if (restartTimer >= RESTART_DELAY || IsKeyPressed(KEY_SPACE))
            {
                // Reset the simulation, keep the same seed
                sim = Simulation(sp, seed);
                restartTimer = 0.0f;
            }
        }
        else
        {
            // Animate the simulation with the neural network brain
            const auto& bestParams = trainingTask.GetBestNetworkParameters();
            sim.AnimateSim([&](const float* states, size_t, float* actions, size_t)
            {
                testNet.FeedForward(bestParams.data(), states, 0, actions, 0);
            });
        }

        // Begin drawing
        BeginDrawing();

        ClearBackground(BLACK);
        // Allow any triangle to be drawn regardless of winding order
        rlDisableBackfaceCulling();

        // Draw the simulation
        DrawSim(sim);
        // Draw UI
        drawUI(sim, trainingTask);

        EndDrawing();
    }

    // Clear the global pointer
    gpTrainingTask = nullptr;

    CloseWindow();
    return 0;
}

//==================================================================
static void drawUI(Simulation& sim, TrainingTask& trainingTask)
{
    const int fsize = 20;
    // Draw info
    DrawText(TextFormat("Fuel: %.0f%%", sim.mLander.mFuel), 10, 10, fsize, WHITE);

    const auto speed = sim.mLander.CalcSpeed();
    const auto speedColor = sim.sp.LANDING_SAFE_SPEED < speed ? RED : GREEN;
    DrawText(TextFormat("Speed: %.1f", speed), 10, 40, fsize, speedColor);

    // Draw training information
    const char* trainingStatus = trainingTask.IsTrainingComplete() ?
                                "TRAINING COMPLETE" : "TRAINING...";
    DrawText(trainingStatus, SCREEN_WIDTH - 300, 10, fsize, YELLOW);

    DrawText(TextFormat("Epoch: %d/%d",
                       trainingTask.GetCurrentIteration(),
                       trainingTask.GetMaxIterations()),
            SCREEN_WIDTH - 300, 40, fsize, WHITE);

    const float bestLoss = trainingTask.GetBestLoss();
    DrawText(TextFormat("Best Loss: %.2f", bestLoss),
            SCREEN_WIDTH - 300, 70, fsize, bestLoss < 100.0f ? GREEN : ORANGE);

    // Draw game state message
    if (sim.mLander.mStateIsLanded)
    {
        DrawText("SUCCESSFUL LANDING!", SCREEN_WIDTH/2 - 150, 200, fsize+10, GREEN);
        DrawText("Wait for restart or press SPACE", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else if (sim.mLander.mStateIsCrashed)
    {
        DrawText("CRASHED!", SCREEN_WIDTH/2 - 80, 200, fsize+10, RED);
        DrawText("Wait for restart or press SPACE", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else
    {
        // Display that we're watching the AI play
        DrawText("AI CONTROLLING LANDER",
            SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT - 40,
            fsize, RAYWHITE);
    }
}
