# SCharacter Learning Agents Implementation

This document describes the learning agents implementation for the SCharacter in the CoopGameFleep project, allowing characters to learn how to move to target locations using Unreal Engine 5.6's Learning Agents system.

## Components Overview

### 1. SCharacterManager
**Location**: `Source/CoopGameFleep/Learning/SCharacterManager.h/cpp`

The main orchestrator that manages the entire learning process. It:
- Initializes all learning components (interactor, policy, critic, training environment)
- Manages different run modes (Training, Inference, ReInitialize)
- Handles the main learning loop in the Tick function

### 2. SCharacterInteractor
**Location**: `Source/CoopGameFleep/Learning/SCharacterInteractor.h/cpp`

Handles the interface between the learning agent and the game world:

**Observations**:
- Character location and velocity
- Character forward direction
- Target location
- Direction from character to target
- Distance to target

**Actions**:
- Movement input (forward/backward, left/right)
- Rotation input (yaw/turn)

### 3. SCharacterTrainingEnvironment
**Location**: `Source/CoopGameFleep/Learning/SCharacterTrainingEnvironment.h/cpp`

Manages the training environment, including:

**Rewards**:
- Large positive reward for reaching the target
- Distance-based reward (closer = better)
- Movement towards target reward
- Time step penalty to encourage efficiency

**Episode Management**:
- Resets characters and targets to random positions
- Detects episode completion (target reached, out of bounds, timeout, death)

### 4. STargetActor
**Location**: `Source/CoopGameFleep/Learning/STargetActor.h/cpp`

A simple target actor that characters learn to reach:
- Visual sphere representation
- Configurable reach distance
- Random position reset functionality

### 5. SCharacterManagerComponent
**Location**: `Source/CoopGameFleep/Learning/SCharacterManagerComponent.h/cpp`

A component wrapper around ULearningAgentsManager for easy integration.

## Setup Instructions

### 1. Create Neural Network Assets

Before using the learning system, you need to create Neural Network Data Assets in the Unreal Editor:

1. In the Content Browser, right-click → Create → Learning Agents → Neural Network
2. Create the following neural networks:
   - **Encoder Neural Network**: Input processing
   - **Policy Neural Network**: Action selection
   - **Decoder Neural Network**: Action output processing  
   - **Critic Neural Network**: Value estimation for training

### 2. Place Actors in Level

1. **SCharacter**: Place one or more SCharacter instances in your level
2. **STargetActor**: Place a target actor in your level
3. **SCharacterManager**: Place the manager actor in your level

### 3. Configure SCharacterManager

In the SCharacterManager's details panel:

**Manager Settings**:
- **Run Mode**: 
  - **Training**: For editor testing (loads existing neural networks)
  - **Inference**: For using trained models
  - **ReInitialize**: For fresh training (automatically used in headless mode)
- **Random Seed**: Set a seed for reproducible results

**Neural Networks**:
- Assign all four neural network assets created in step 1
- **Target Actor**: Reference to the STargetActor in your level

**Learning Settings**:
- Configure PolicySettings, CriticSettings, TrainingSettings as needed
- Adjust TrainingGameSettings for episode length, etc.

### 4. Configure Training Environment

In the SCharacterTrainingEnvironment (accessible through the manager):

**Rewards**:
- **Reach Target Reward**: Large positive reward (default: 100.0)
- **Distance Reward Scale**: Scale for distance-based rewards (default: 0.1)
- **Movement Towards Target Reward**: Small positive reward for progress (default: 0.5)
- **Time Step Penalty**: Small negative reward per step (default: -0.01)

**Environment**:
- **Reset Center**: Center point for random resets
- **Reset Bounds**: Bounds for random positioning
- **Min Distance Between Character And Target**: Minimum separation on reset

### 5. Input Handling During Learning

The system automatically handles input conflicts:
- **Player Input Control**: SCharacter has a `bPlayerInputEnabled` flag
- **During Learning**: Player input is automatically disabled when AI takes control
- **Manual Control**: You can toggle `bPlayerInputEnabled` to switch between player and AI control

## Usage

### Training Mode

1. Set **Run Mode** to **Training** in the SCharacterManager
2. Ensure all neural networks are properly assigned
3. Configure training settings (TrainingSettings, TrainingGameSettings)
4. Play the level
5. Characters will begin learning to move towards the target through PPO training
6. Monitor progress through log output and observe improving behavior over time
7. Training will run continuously - characters get better at reaching targets over episodes

### Inference Mode

1. Set **Run Mode** to **Inference** in the SCharacterManager
2. Ensure trained neural networks are loaded
3. Play the level
4. Characters will use learned behavior to move towards targets

## Learning Process

The system implements Proximal Policy Optimization (PPO) with the following learning loop:

1. **Observation**: Character observes environment state
2. **Action**: Policy network selects movement actions
3. **Environment Step**: Character performs actions
4. **Reward**: Training environment calculates rewards
5. **Episode Reset**: When target reached or episode ends
6. **Learning**: PPO updates policy and critic networks

## Customization

### Adding New Observations

To add new observations to the SCharacterInteractor:

1. Add observation specification in `SpecifyAgentObservation_Implementation`
2. Add observation gathering in `GatherAgentObservation_Implementation`

### Modifying Rewards

To modify the reward structure:

1. Edit `GatherAgentReward_Implementation` in SCharacterTrainingEnvironment
2. Add new reward components and configurable parameters

### Changing Actions

To modify available actions:

1. Update action specification in `SpecifyAgentAction_Implementation`
2. Update action execution in `PerformAgentAction_Implementation`

## Troubleshooting

### Common Issues

1. **Neural networks not set**: Ensure all four neural network assets are assigned
2. **No target actor**: Make sure TargetActor is properly referenced
3. **Characters not moving**: Check that the character controllers are properly set up
4. **Training not progressing**: Verify reward signals are appropriate and environment resets correctly

### Debug Output

The system provides extensive logging:
- Agent initialization
- Episode resets with positions and distances
- Target reached notifications
- Reward calculations

Check the Output Log in Unreal Editor for detailed information.

## Training Implementation

The system now includes full PPO (Proximal Policy Optimization) training capabilities:

- **Shared Memory Communicator**: Enables communication with external training processes
- **PPO Trainer**: Handles the complete training loop with policy and critic updates
- **Training Modes**: 
  - **Training**: Loads existing neural networks for continued training
  - **Inference**: Uses trained models for testing
  - **ReInitialize**: Fresh neural network initialization (used automatically in headless training)
- **Real-time Learning**: Characters learn while the game is running

### Training Process

1. **Policy Network**: Learns to map observations to actions
2. **Critic Network**: Learns to estimate value of states for better training
3. **Experience Collection**: Gathers state-action-reward sequences
4. **Batch Learning**: Updates networks periodically based on collected experience
5. **Exploration vs Exploitation**: Balances trying new actions vs using learned behavior

## Future Enhancements

This implementation provides a foundation that can be extended with:

1. **Multi-Agent Learning**: Support for multiple characters learning simultaneously
2. **Complex Behaviors**: More sophisticated movement patterns and obstacle avoidance
3. **Dynamic Targets**: Moving targets and multiple objectives
4. **Hierarchical Tasks**: Breaking down complex navigation into sub-tasks
5. **Curriculum Learning**: Gradually increasing task difficulty

## Dependencies

- Unreal Engine 5.6
- Learning Agents Plugin (Experimental)
- Required modules: Learning, LearningAgents, LearningTraining, LearningAgentsTraining 