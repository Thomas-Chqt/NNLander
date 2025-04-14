#include <array>
#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"
#include "TrainingTaskRandom.h"
#include "DrawUI.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static const float RESTART_DELAY = 2.0f;

// Number of training epochs to run
static const int MAX_TRAINING_EPOCHS = 100000;

//==================================================================
// Network configuration
//==================================================================
static constexpr std::array<int, 3> NETWORK_ARCHITECTURE = {
    SIM_BRAINSTATE_N,              // Input layer: simulation state variables
    (int)((double)SIM_BRAINSTATE_N*1.25), // Hidden layer
    SIM_BRAINACTION_N             // Output layer: actions (up, left, right)
};

using TrainingTask = TrainingTaskRandom<float, NETWORK_ARCHITECTURE>;

// Forward declarations
static void drawUI(Simulation& sim, TrainingTask& trainingTask);

//==================================================================
// Main function
//==================================================================
int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NNLander03 - Random Training");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    uint32_t seed = 1135; // Initial random seed
    Simulation sim(sp, seed);

    // Create the training task
    TrainingTask trainingTask(sp, MAX_TRAINING_EPOCHS);

    // No separate testNet needed, we'll use the one inside trainingTask

    float restartTimer = 0.0f;

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

        // Auto-restart after landing or crashing
        if (sim.mLander.mStateIsLanded || sim.mLander.mStateIsCrashed)
        {
            restartTimer += GetFrameTime();
            if (restartTimer >= RESTART_DELAY || IsKeyPressed(KEY_SPACE))
            {
                // Reset the simulation, keep the same seed
                sim = Simulation(sp, seed);
                restartTimer = 0.0f;
            }
        }
        else
        {
            // Animate the simulation using the best network from the training task
            sim.AnimateSim([&](const TrainingTask::NeuralNet::Inputs& states, TrainingTask::NeuralNet::Outputs& actions)
            {
                // states -> bestNet -> actions
                trainingTask.GetBestNetwork().FeedForward(states, actions);
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

    CloseWindow();
    return 0;
}

//==================================================================
static void drawUI(Simulation& sim, TrainingTask& trainingTask)
{
    // Draw neural network visualization
    //if (!sim.mLander.mStateIsLanded && !sim.mLander.mStateIsCrashed)
    {
        // Use the actual best network from the training task for visualization
        const auto& bestNet = trainingTask.GetBestNetwork();
        // Pass the flattened parameters to the drawing function
        DrawNeuralNetwork(bestNet);
    }

    const int fsize = 20;

    DrawUIBase(sim, fsize, "ai");

    // Draw training information
    DrawUITrainingStatus(trainingTask.IsTrainingComplete(), fsize);

    DrawText(TextFormat("Epoch: %i/%i",
                       (int)trainingTask.GetCurrentEpoch(),
                       (int)trainingTask.GetMaxEpochs()),
            SCREEN_WIDTH - 300, 40, fsize, WHITE);

    const double bestScore = trainingTask.GetBestScore();
    DrawText(TextFormat("Best Score: %.2f", bestScore),
            SCREEN_WIDTH - 300, 70, fsize, bestScore > 500.0f ? GREEN : ORANGE);
}
