#!/bin/bash
# Headless Training Launcher for CoopGameFleep
# This script launches the packaged game in headless mode for training
# Usage: ./scripts/run-training-headless.sh [--training-build-dir "TrainingBuild"] [--map-name "P_LearningAgentsTrial"] [--log-file "training_log.log"] [--training-task-name "RunA"]

# Default values
PROJECT_PATH="$(pwd)"
TRAINING_BUILD_DIR="TrainingBuild"
MAP_NAME="P_LearningAgentsTrial1"  # Default learning map
LOG_FILE="scharacter_training.log"
EXE_NAME="CoopGameFleep"
TRAINING_TASK_NAME_ARG=""
# Obstacle configuration parameters
USE_OBSTACLES=false
MAX_OBSTACLES=8
MIN_OBSTACLE_SIZE=100.0
MAX_OBSTACLE_SIZE=300.0
OBSTACLE_MODE="Static"

# Helper to sanitize task names for filesystem and CLI usage
sanitize_task_name() {
    local name="$1"

    # Trim leading/trailing whitespace
    name="$(echo -n "$name" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
    # Replace invalid characters with underscore
    name="${name//[^A-Za-z0-9_-]/_}"
    # Trim leading/trailing underscores
    name="$(echo -n "$name" | sed -e 's/^_*//' -e 's/_*$//')"

    if [[ -z "$name" ]]; then
        echo ""
        return
    fi

    if [[ ${#name} -gt 60 ]]; then
        name="${name:0:60}"
    fi

    echo "$name"
}

generate_guid_segment() {
    if command -v uuidgen >/dev/null 2>&1; then
        uuidgen 2>/dev/null | tr '[:upper:]' '[:lower:]' | tr -d '-' | cut -c1-8
        return
    fi

    if [[ -r /proc/sys/kernel/random/uuid ]]; then
        head -c 36 /proc/sys/kernel/random/uuid | tr '[:upper:]' '[:lower:]' | tr -d '-' | cut -c1-8
        return
    fi

    if command -v openssl >/dev/null 2>&1; then
        openssl rand -hex 8 | cut -c1-8
        return
    fi

    if command -v python3 >/dev/null 2>&1; then
        python3 - <<'PY'
import sys, uuid
print(uuid.uuid4().hex[:8])
PY
        return
    fi

    if command -v python >/dev/null 2>&1; then
        python - <<'PY'
import sys, uuid
print(uuid.uuid4().hex[:8])
PY
        return
    fi

    # Fallback: use date-based entropy hashed via sha1sum if available
    if command -v sha1sum >/dev/null 2>&1; then
        date +%s%N | sha1sum | cut -c1-8
        return
    fi

    # Final fallback: use random from /dev/urandom if accessible
    if [[ -r /dev/urandom ]]; then
        od -An -tx1 -N4 /dev/urandom | tr -d ' \n'
        return
    fi

    # Absolute fallback: current timestamp in hex
    printf "%08x" "$(date +%s)"
}

generate_unique_task_name() {
    local base="$1"
    local safe_base
    safe_base="$(sanitize_task_name "$base")"
    if [[ -z "$safe_base" ]]; then
        safe_base="run"
    fi

    local guid_segment
    guid_segment="$(generate_guid_segment)"
    if [[ -z "$guid_segment" ]]; then
        guid_segment="$(date +%s)"
    fi

    local candidate="${safe_base}-${guid_segment}"
    local sanitized
    sanitized="$(sanitize_task_name "$candidate")"

    if [[ -z "$sanitized" ]]; then
        echo "$guid_segment"
    else
        echo "$sanitized"
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --training-build-dir)
            TRAINING_BUILD_DIR="$2"
            shift 2
            ;;
        --map-name)
            MAP_NAME="$2"
            shift 2
            ;;
        --log-file)
            LOG_FILE="$2"
            shift 2
            ;;
        --exe-name)
            EXE_NAME="$2"
            shift 2
            ;;
        --training-task-name)
            TRAINING_TASK_NAME_ARG="$2"
            shift 2
            ;;
        --use-obstacles)
            USE_OBSTACLES="$2"
            shift 2
            ;;
        --max-obstacles)
            MAX_OBSTACLES="$2"
            shift 2
            ;;
        --min-obstacle-size)
            MIN_OBSTACLE_SIZE="$2"
            shift 2
            ;;
        --max-obstacle-size)
            MAX_OBSTACLE_SIZE="$2"
            shift 2
            ;;
        --obstacle-mode)
            OBSTACLE_MODE="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --training-build-dir DIR    Training build directory (default: TrainingBuild)"
            echo "  --map-name MAP              Map name to load (default: P_LearningAgentsTrial1)"
            echo "  --log-file FILE             Log file name (default: scharacter_training.log)"
            echo "  --exe-name NAME             Executable name (default: CoopGameFleep)"
            echo "  --training-task-name NAME   Preferred identifier for Learning Agents task"
            echo "  --use-obstacles BOOL        Enable/disable obstacles (default: false)"
            echo "  --max-obstacles NUM         Maximum number of obstacles (default: 8)"
            echo "  --min-obstacle-size SIZE    Minimum obstacle size (default: 100.0)"
            echo "  --max-obstacle-size SIZE    Maximum obstacle size (default: 300.0)"
            echo "  --obstacle-mode MODE         Obstacle mode: Static or Dynamic (default: Static)"
            echo "  -h, --help                  Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
GRAY='\033[0;37m'
NC='\033[0m' # No Color

echo -e "${CYAN}======================================${NC}"
echo -e "${GREEN}COOPGAMEFLEEP HEADLESS TRAINING${NC}"
echo -e "${CYAN}======================================${NC}"
echo -e "${YELLOW}Project Path: $PROJECT_PATH${NC}"
echo -e "${YELLOW}Training Build Dir: $TRAINING_BUILD_DIR${NC}"
echo -e "${YELLOW}Map Name: $MAP_NAME${NC}"
echo -e "${YELLOW}Log File: $LOG_FILE${NC}"
echo -e "${YELLOW}Executable: $EXE_NAME${NC}"
REQUESTED_IDENTIFIER="$TRAINING_TASK_NAME_ARG"
SANITIZED_REQUESTED="$(sanitize_task_name "$REQUESTED_IDENTIFIER")"
TRAINING_TASK_NAME="$SANITIZED_REQUESTED"
AUTO_GENERATED_TASK_NAME=false

if [[ -n "$REQUESTED_IDENTIFIER" ]]; then
    echo -e "${YELLOW}Run Identifier (requested): $REQUESTED_IDENTIFIER${NC}"
    if [[ -n "$SANITIZED_REQUESTED" && "$SANITIZED_REQUESTED" != "$REQUESTED_IDENTIFIER" ]]; then
        echo -e "${YELLOW}Sanitized Task Name: $SANITIZED_REQUESTED${NC}"
    elif [[ -z "$SANITIZED_REQUESTED" ]]; then
        echo -e "${YELLOW}Requested task name sanitized to empty; generating unique identifier.${NC}"
    fi
fi

if [[ -z "$TRAINING_TASK_NAME" ]]; then
    TRAINING_TASK_NAME="$(generate_unique_task_name "$SANITIZED_REQUESTED")"
    AUTO_GENERATED_TASK_NAME=true
fi

if [[ "$AUTO_GENERATED_TASK_NAME" == true ]]; then
    echo -e "${YELLOW}Generated Task Name: $TRAINING_TASK_NAME${NC}"
else
    echo -e "${YELLOW}Trainer Task Name: $TRAINING_TASK_NAME${NC}"
fi

# Find the executable
BUILD_PATH="$PROJECT_PATH/$TRAINING_BUILD_DIR"
EXE_FILES=$(find "$BUILD_PATH" -name "$EXE_NAME" -type f 2>/dev/null)

if [ -z "$EXE_FILES" ]; then
    echo -e "${RED}Executable '$EXE_NAME' not found in build directory: $BUILD_PATH${NC}"
    echo -e "${RED}Please run package-training.sh first to create the training build${NC}"
    exit 1
fi

# Get the first executable found
GAME_EXECUTABLE=$(echo "$EXE_FILES" | head -n1)
EXE_DIRECTORY=$(dirname "$GAME_EXECUTABLE")

echo -e "${GREEN}Found executable: $GAME_EXECUTABLE${NC}"

# Change to executable directory for proper relative path resolution
cd "$EXE_DIRECTORY" || exit 1

echo -e "\n${CYAN}======================================${NC}"
echo -e "${YELLOW}LAUNCHING HEADLESS TRAINING${NC}"
echo -e "${CYAN}======================================${NC}"
echo -e "${YELLOW}Command Line Arguments:${NC}"
echo -e "${WHITE}  Map: $MAP_NAME${NC}"
echo -e "${WHITE}  Headless Training: Enabled (forces Training mode)${NC}"
echo -e "${WHITE}  Null RHI: Enabled (no rendering)${NC}"
echo -e "${WHITE}  No Sound: Enabled${NC}"
echo -e "${WHITE}  Logging: Enabled to $LOG_FILE${NC}"
echo -e ""
echo -e "${CYAN}Obstacle Configuration:${NC}"
echo -e "${WHITE}  Use Obstacles: $USE_OBSTACLES${NC}"
echo -e "${WHITE}  Max Obstacles: $MAX_OBSTACLES${NC}"
echo -e "${WHITE}  Min Obstacle Size: $MIN_OBSTACLE_SIZE${NC}"
echo -e "${WHITE}  Max Obstacle Size: $MAX_OBSTACLE_SIZE${NC}"
echo -e "${WHITE}  Obstacle Mode: $OBSTACLE_MODE${NC}"

if [[ -n "$TRAINING_TASK_NAME" ]]; then
    TENSORBOARD_HINT="Intermediate/LearningAgents/${TRAINING_TASK_NAME}*/TensorBoard/runs"
    SNAPSHOTS_HINT="Intermediate/LearningAgents/${TRAINING_TASK_NAME}*/NeuralNetworks"
else
    TENSORBOARD_HINT="Intermediate/LearningAgents/TensorBoard/runs"
    SNAPSHOTS_HINT="Intermediate/LearningAgents/Training*"
fi

# Build command line arguments for headless training
GAME_ARGS=(
    "$MAP_NAME"                    # Load the training map
    "-headless-training"           # Custom flag to identify headless training mode
    "-nullrhi"                     # Disable rendering for headless mode
    "-nosound"                     # Disable sound
    "-log"                         # Enable logging to console
    "-log=$LOG_FILE"               # Log to specific file
    "-unattended"                  # Run without user interaction
    "-nothreading"                 # Some training setups work better without threading
    "-NoVerifyGC"                  # Skip garbage collection verification for performance
    "-NoLoadStartupPackages"       # Skip loading startup packages for faster boot
    "-FORCELOGFLUSH"               # Force log flushing for real-time monitoring
    "-ini:Engine:[Core.Log]:LogPython=Verbose"  # Enable Python logging for Learning Agents
    "-UseObstacles=$USE_OBSTACLES"  # Enable/disable obstacles
    "-MaxObstacles=$MAX_OBSTACLES"  # Maximum number of obstacles
    "-MinObstacleSize=$MIN_OBSTACLE_SIZE"  # Minimum obstacle size
    "-MaxObstacleSize=$MAX_OBSTACLE_SIZE"  # Maximum obstacle size
    "-ObstacleMode=$OBSTACLE_MODE"  # Obstacle mode (Static/Dynamic)
)

if [[ -n "$TRAINING_TASK_NAME" ]]; then
    GAME_ARGS+=("-TrainingTaskName=$TRAINING_TASK_NAME")
fi

# Debug: Show all game arguments
echo -e "\n${CYAN}Game Arguments:${NC}"
for arg in "${GAME_ARGS[@]}"; do
    echo -e "${WHITE}  $arg${NC}"
done

echo -e "\n${GREEN}Starting headless training...${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop training${NC}"
echo -e "${CYAN}Monitor progress in: $LOG_FILE${NC}"
echo -e "${CYAN}TensorBoard logs hint: $TENSORBOARD_HINT${NC}"

echo -e "\n${GRAY}Executing command:${NC}"
echo -e "${GRAY}$EXE_NAME ${GAME_ARGS[*]}${NC}"

# Function to handle cleanup on exit
cleanup() {
    echo -e "\n${YELLOW}Stopping training...${NC}"
    if [ ! -z "$TRAINING_PID" ]; then
        kill "$TRAINING_PID" 2>/dev/null
        wait "$TRAINING_PID" 2>/dev/null
    fi
    echo -e "${CYAN}Training stopped.${NC}"
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Start the training process
echo -e "\n${GREEN}Starting training process...${NC}"
"$GAME_EXECUTABLE" "${GAME_ARGS[@]}" &
TRAINING_PID=$!

echo -e "${GREEN}Training process started with PID: $TRAINING_PID${NC}"
echo -e "${CYAN}You can monitor the log file in another terminal with:${NC}"
echo -e "${WHITE}  tail -f '$LOG_FILE'${NC}"

# Wait for the process to complete
echo -e "\n${YELLOW}Waiting for training to complete...${NC}"
echo -e "${CYAN}Training will run indefinitely (Press Ctrl+C to stop)${NC}"

# Wait for the process to exit
wait "$TRAINING_PID"
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo -e "\n${GREEN}Training completed successfully!${NC}"
elif [ $EXIT_CODE -eq -1 ]; then
    echo -e "\n${YELLOW}Training terminated due to timeout${NC}"
else
    echo -e "\n${YELLOW}Training completed with exit code: $EXIT_CODE${NC}"
fi

echo -e "\n${CYAN}======================================${NC}"
echo -e "${YELLOW}TRAINING SESSION ENDED${NC}"
echo -e "${CYAN}======================================${NC}"
echo -e "${WHITE}Check the following for results:${NC}"
echo -e "${CYAN}  - Log file: $EXE_DIRECTORY/$LOG_FILE${NC}"
if [[ -n "$TRAINING_TASK_NAME" ]]; then
    echo -e "${CYAN}  - TensorBoard logs: $PROJECT_PATH/$TENSORBOARD_HINT${NC}"
    echo -e "${CYAN}  - Neural network snapshots: $PROJECT_PATH/$SNAPSHOTS_HINT${NC}"
else
    echo -e "${CYAN}  - TensorBoard logs: $PROJECT_PATH/Intermediate/LearningAgents/TensorBoard/runs${NC}"
    echo -e "${CYAN}  - Neural network snapshots in project Intermediate directory${NC}"
fi

echo -e "\n${GREEN}To view TensorBoard, run: ./scripts/run-tensorboard.sh${NC}"

# Training session completed
