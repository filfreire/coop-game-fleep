#!/bin/bash
# Setup pre-commit hooks for coop-game-fleep repository

set -e

echo "🔧 Setting up pre-commit hooks for coop-game-fleep..."

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is required but not installed. Please install Python 3."
    exit 1
fi

# Check if pip is installed
if ! command -v pip3 &> /dev/null; then
    echo "❌ pip3 is required but not installed. Please install pip3."
    exit 1
fi

# Install pre-commit
echo "📦 Installing pre-commit..."
if command -v pipx &> /dev/null; then
    echo "Using pipx to install pre-commit..."
    pipx install pre-commit
elif pip3 install --user pre-commit 2>/dev/null; then
    echo "Installed pre-commit with --user flag"
else
    echo "Trying to install with --break-system-packages flag..."
    pip3 install --break-system-packages pre-commit
fi

# Install the pre-commit hooks
echo "🪝 Installing pre-commit hooks..."
pre-commit install

# Run pre-commit on all files to ensure everything is working
echo "🧹 Running pre-commit on all files (this may take a while on first run)..."
pre-commit run --all-files || {
    echo "⚠️  Some pre-commit checks failed. This is normal for the first run."
    echo "💡 You can fix the issues and run 'pre-commit run --all-files' again."
}

echo ""
echo "✅ Pre-commit setup complete!"
echo ""
echo "📋 Available commands:"
echo "  • Run on all files:     pre-commit run --all-files"
echo "  • Run on staged files:  pre-commit run"
echo "  • Update hooks:         pre-commit autoupdate"
echo "  • Uninstall hooks:      pre-commit uninstall"
echo ""
echo "🎯 Pre-commit will now run automatically on every commit!"
