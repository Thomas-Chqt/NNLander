#include "raylib.h"
#include "rlgl.h"
#include "Simulation.h"
#include "SimulationDisplay.h"
#include <vector>

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

static void drawUI(Simulation& sim);

//==================================================================
class SimpleNeuralNet
{
    const float* mpParameters {};         // Pointer to weights (owned externally)
    const std::vector<int> mArchitecture; // Network architecture (nodes per layer)
    size_t mTotalParameters = 0;          // Total number of parameters in the network
    size_t mMaxLayerSize = 0;             // Maximum number of neurons in any layer
public:
/*
The constructor takes pointers to weights (weights and biases) and the
network architecture.
The network architecture tells how many layers there are and how many
neurons per layer.

NOTICE: the actual network that we use has more neurons and more layers.
The example below is just for illustration.

           O O O      | architecture[0] = 3 neurons (INPUT layer, the simulation state)
          /|/|\|\     |
         O O O O O    | architecture[1] = 5 neurons (HIDDEN layer, the thinking layer)
         X X X X X    |
         O O O O O    | architecture[2] = 5 neurons (HIDDEN layer, the thinking layer)
          \|/|\|/     |
           O O O      | architecture[3] = 3 neurons (OUTPUT layer, the actions to take)

  architecture = [3, 5, 5, 3]

  - Total Neurons:    16 -> (3   +   5   +   5   +   3)
    (Sum of all neurons in Input, Hidden, and Output layers)

  - Connections:      55 -> (   3*5  +  5*5  +  5*3   )
    (Weights linking each neuron in one layer to neurons in the next)

  - Biases:           13 -> (0   +   5   +   5   +   3)
    (One bias for each neuron, except for the input layer !)

  - Total Parameters: 68 -> (Connections + Biases)
*/

    SimpleNeuralNet(
        const float* pParameters,
        const std::vector<int>& architecture)
        : mpParameters(pParameters)
        , mArchitecture(architecture)
    {
        // Calculate total number of parameters needed
        mTotalParameters = 0;
        for (size_t i=1; i < mArchitecture.size(); ++i)
            // Weights between layers + biases for each neuron in current layer
            mTotalParameters += mArchitecture[i-1] * mArchitecture[i] + mArchitecture[i];

        // Find the maximum number of neurons in any layer
        mMaxLayerSize = *std::max_element(mArchitecture.begin(), mArchitecture.end());
    }

    // Feed forward function
    void FeedForward(const float* inputs, float* outputs)
    {
        // Allocate buffers on the stack to avoid touching the heap
        float* currentLayerOutputs = (float*)alloca(mMaxLayerSize * sizeof(float));
        float* nextLayerOutputs = (float*)alloca(mMaxLayerSize * sizeof(float));

        // Copy inputs (simulation states) to first layer outputs
        for (int i=0; i < mArchitecture[0]; ++i)
            currentLayerOutputs[i] = inputs[i];

        // Parameter index tracker
        int paramIdx = 0;

        // Process each layer
        for (size_t layer=1; layer < mArchitecture.size(); ++layer)
        {
            const auto prevLayerSize = mArchitecture[layer-1];
            const auto currentLayerSize = mArchitecture[layer];

            // For each neuron in the current layer
            for (int neuron=0; neuron < mArchitecture[layer]; ++neuron)
            {
                float sum = 0.0f;

                // Sum weighted inputs
                for (int prevNeuron=0; prevNeuron < prevLayerSize; ++prevNeuron)
                    sum += currentLayerOutputs[prevNeuron] * mpParameters[paramIdx++];

                // Add bias
                sum += mpParameters[paramIdx++];

                // Apply activation function (ReLU in this case)
                nextLayerOutputs[neuron] = Activate(sum);
            }

            // Swap buffers, next-layer output becomes current-layer output
            std::swap(currentLayerOutputs, nextLayerOutputs);
        }

        // Copy final layer outputs to outputs array
        for (int i = 0; i < mArchitecture.back(); ++i)
            outputs[i] = currentLayerOutputs[i];
    }

    // Get total number of parameters (weights + biases)
    size_t GetTotalParameters() const { return mTotalParameters; }

private:
    // Activation function (ReLU)
    float Activate(float x) const {
        return x > 0.0f ? x : 0.0f;
    }
};

//==================================================================
static void getNeuralNetBrainActions(
    const float* in_simState, size_t in_simStateN,
    float* out_actions, size_t out_actionsN)
{
    // TODO: Implement the neural network brain
    (void)in_simState; (void)in_simStateN;
    (void)out_actions; (void)out_actionsN;
}

//==================================================================
// Main function
//==================================================================
int main()
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander Simulation");
    SetTargetFPS(60);

    // Setup the simulation parameters
    SimParams sp;
    sp.SCREEN_WIDTH = (float)SCREEN_WIDTH;
    sp.SCREEN_HEIGHT = (float)SCREEN_HEIGHT;

    // Create the simulation object with the parameters
    Simulation sim(sp);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update if it is not crashed or landed
        if (sim.mLander.mStateIsCrashed == false &&
            sim.mLander.mStateIsLanded == false)
        {
            // Animate the simulation with the fixed brain
            sim.AnimateSim(getNeuralNetBrainActions);
        }
        else
        {
            // Restart game on Space key
            if (IsKeyPressed(KEY_SPACE))
                sim = Simulation(sp); // Reset the simulation
        }

        // Begin drawing
        BeginDrawing();

        ClearBackground(BLACK);
        // Allow any triangle to be drawn regardless of winding order
        rlDisableBackfaceCulling();

        // Draw the simulation
        DrawSim(sim);
        // Draw UI
        drawUI(sim);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

//==================================================================
static void drawUI(Simulation& sim)
{
    const int fsize = 20;
    // Draw info
    DrawText(TextFormat("Fuel: %.0f%%", sim.mLander.mFuel), 10, 10, fsize, WHITE);

    const auto speed = sim.mLander.CalcSpeed();
    const auto speedColor = sim.sp.LANDING_SAFE_SPEED < speed ? RED : GREEN;
    DrawText(TextFormat("Speed: %.1f", speed), 10, 40, fsize, speedColor);

    // Draw game state message
    if (sim.mLander.mStateIsLanded)
    {
        DrawText("SUCCESSFUL LANDING!", SCREEN_WIDTH/2 - 150, 200, fsize+10, GREEN);
        DrawText("Press SPACE to play again", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else if (sim.mLander.mStateIsCrashed)
    {
        DrawText("STATE_CRASHED!", SCREEN_WIDTH/2 - 80, 200, fsize+10, RED);
        DrawText("Press SPACE to try again", SCREEN_WIDTH/2 - 150, 240, fsize, WHITE);
    }
    else
    {
        DrawText("UP: Vertical thrust, LEFT/RIGHT: Lateral thrusters",
            SCREEN_WIDTH - 600, 10,
            fsize, WHITE);
    }
}

