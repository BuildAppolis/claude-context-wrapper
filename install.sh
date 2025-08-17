#!/bin/bash

# Claude Context Wrapper Installation Script
# Created by BuildAppolis (www.buildappolis.com)

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
INSTALL_DIR="$HOME/.local/bin"
SCRIPT_NAME="c"
GITHUB_RAW="https://raw.githubusercontent.com/BuildAppolis/claude-context-wrapper/main"

# Detect if running from local clone or via curl
if [[ -f "${BASH_SOURCE[0]}" ]] && [[ -f "$(dirname "${BASH_SOURCE[0]}")/c" ]]; then
    # Running from local clone
    SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    SOURCE_SCRIPT="$SCRIPT_DIR/c"
    INSTALL_MODE="local"
else
    # Running via curl
    INSTALL_MODE="remote"
fi

echo -e "${BLUE}╔════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   Claude Context Wrapper Installation     ║${NC}"
echo -e "${BLUE}║        by BuildAppolis.com                ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════╝${NC}"
echo ""

# Detect platform
detect_platform() {
    local platform="unknown"
    
    case "$OSTYPE" in
        linux*)
            if grep -q Microsoft /proc/version 2>/dev/null; then
                platform="wsl"
            else
                platform="linux"
            fi
            ;;
        darwin*)
            platform="macos"
            ;;
        msys*|cygwin*|win32*)
            platform="windows"
            ;;
        *)
            platform="unknown"
            ;;
    esac
    
    echo "Detected platform: $platform"
    return 0
}

# Check if claude is installed
check_claude() {
    local claude_cmd="${CLAUDE_COMMAND:-claude}"
    echo -n "Checking for Claude Code CLI... "
    
    if command -v "$claude_cmd" &> /dev/null; then
        echo -e "${GREEN}✓${NC} (found: $claude_cmd)"
        return 0
    else
        echo -e "${RED}✗${NC}"
        echo ""
        echo -e "${YELLOW}⚠ Claude Code CLI not found!${NC}"
        echo "Please install Claude Code first:"
        echo "  Visit: https://claude.ai/code"
        echo ""
        echo "You can also set a custom command via CLAUDE_COMMAND environment variable"
        echo ""
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo ""
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

# Create install directory
create_install_dir() {
    echo -n "Creating installation directory... "
    mkdir -p "$INSTALL_DIR"
    echo -e "${GREEN}✓${NC}"
}

# Install the wrapper script
install_wrapper() {
    echo -n "Installing wrapper script... "
    
    if [[ "$INSTALL_MODE" == "local" ]]; then
        # Copy from local file
        cp "$SOURCE_SCRIPT" "$INSTALL_DIR/$SCRIPT_NAME"
    else
        # Download from GitHub
        if command -v curl &> /dev/null; then
            curl -sSL "$GITHUB_RAW/c" -o "$INSTALL_DIR/$SCRIPT_NAME"
        elif command -v wget &> /dev/null; then
            wget -q "$GITHUB_RAW/c" -O "$INSTALL_DIR/$SCRIPT_NAME"
        else
            echo -e "${RED}✗${NC}"
            echo "Error: Neither curl nor wget found. Please install one of them."
            exit 1
        fi
    fi
    
    chmod +x "$INSTALL_DIR/$SCRIPT_NAME"
    echo -e "${GREEN}✓${NC}"
}

# Update shell configuration
update_shell_config() {
    local shell_config=""
    local shell_name=""
    
    # Detect shell
    if [[ "$SHELL" == *"zsh"* ]]; then
        shell_config="$HOME/.zshrc"
        shell_name="zsh"
    elif [[ "$SHELL" == *"bash"* ]]; then
        shell_config="$HOME/.bashrc"
        shell_name="bash"
    else
        echo -e "${YELLOW}⚠ Unknown shell. Please add $INSTALL_DIR to your PATH manually.${NC}"
        return
    fi
    
    echo -n "Updating $shell_name configuration... "
    
    # Check if PATH already contains the install directory
    if ! grep -q "$INSTALL_DIR" "$shell_config" 2>/dev/null; then
        echo "" >> "$shell_config"
        echo "# Claude Context Wrapper - Added by install.sh" >> "$shell_config"
        echo "export PATH=\"\$PATH:$INSTALL_DIR\"" >> "$shell_config"
        echo -e "${GREEN}✓${NC}"
        echo ""
        echo -e "${YELLOW}Note: Please restart your terminal or run:${NC}"
        echo "  source $shell_config"
    else
        echo -e "${GREEN}✓${NC} (already configured)"
    fi
}

# Install examples
install_examples() {
    echo -n "Installing example context files... "
    
    local examples_dir="$HOME/.claude-context-examples"
    mkdir -p "$examples_dir"
    
    if [[ "$INSTALL_MODE" == "local" ]]; then
        # Copy examples if they exist locally
        if [[ -d "$SCRIPT_DIR/.claude/examples" ]]; then
            cp -r "$SCRIPT_DIR/.claude/examples/"* "$examples_dir/" 2>/dev/null || true
        fi
    else
        # Download examples from GitHub
        if command -v curl &> /dev/null; then
            curl -sSL "$GITHUB_RAW/.claude/examples/advanced-typescript.context.ts" -o "$examples_dir/advanced-ts.context.ts" 2>/dev/null || true
            curl -sSL "$GITHUB_RAW/.claude/examples/advanced-python.context.py" -o "$examples_dir/advanced-py.context.py" 2>/dev/null || true
        elif command -v wget &> /dev/null; then
            wget -q "$GITHUB_RAW/.claude/examples/advanced-typescript.context.ts" -O "$examples_dir/advanced-ts.context.ts" 2>/dev/null || true
            wget -q "$GITHUB_RAW/.claude/examples/advanced-python.context.py" -O "$examples_dir/advanced-py.context.py" 2>/dev/null || true
        fi
    fi
    
    # Create basic examples if not present
    if [[ ! -f "$examples_dir/advanced-ts.context.ts" ]]; then
        cat > "$examples_dir/advanced-ts.context.ts" << 'EOF'
// Advanced TypeScript Context Example
import { execSync } from 'child_process';
import * as fs from 'fs';

const getGitInfo = () => {
    try {
        return {
            branch: execSync('git branch --show-current', { encoding: 'utf8' }).trim(),
            commit: execSync('git rev-parse --short HEAD', { encoding: 'utf8' }).trim(),
        };
    } catch {
        return {};
    }
};

const context = {
    ...getGitInfo(),
    timestamp: new Date().toISOString(),
    project: process.cwd().split('/').pop(),
    nodeVersion: process.version,
};

console.log(Object.entries(context)
    .map(([k, v]) => `${k}: ${v}`)
    .join(', '));
EOF
    fi
    
    if [[ ! -f "$examples_dir/advanced-py.context.py" ]]; then
        cat > "$examples_dir/advanced-py.context.py" << 'EOF'
#!/usr/bin/env python3
# Advanced Python Context Example

import os
import subprocess
from datetime import datetime
from pathlib import Path

def get_git_info():
    try:
        branch = subprocess.check_output(['git', 'branch', '--show-current'], text=True).strip()
        commit = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], text=True).strip()
        return {'branch': branch, 'commit': commit}
    except:
        return {}

context = {
    **get_git_info(),
    'timestamp': datetime.now().isoformat(),
    'project': Path.cwd().name,
    'python_version': '.'.join(map(str, sys.version_info[:3])),
}

print(', '.join([f"{k}: {v}" for k, v in context.items()]))
EOF
    fi
    
    echo -e "${GREEN}✓${NC}"
    echo "  Examples installed to: $examples_dir"
}

# Verify installation
verify_installation() {
    echo ""
    echo "Verifying installation..."
    
    # Check if script is accessible
    if [[ -f "$INSTALL_DIR/$SCRIPT_NAME" ]]; then
        echo -e "  ${GREEN}✓${NC} Wrapper script installed"
    else
        echo -e "  ${RED}✗${NC} Wrapper script not found"
        exit 1
    fi
    
    # Try to run the help command
    if "$INSTALL_DIR/$SCRIPT_NAME" --version &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} Wrapper script is executable"
    else
        echo -e "  ${YELLOW}⚠${NC} Wrapper script may not be in PATH yet"
    fi
}

# Initialize project context
initialize_project() {
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}Would you like to initialize Claude context in the current directory?${NC}"
    echo "Current directory: $(pwd)"
    echo ""
    echo "Options:"
    echo "  1) TypeScript project (.ts)"
    echo "  2) Python project (.py)"
    echo "  3) Text-based context (.txt)"
    echo "  4) Skip for now"
    echo ""
    read -p "Enter your choice (1-4): " -n 1 -r
    echo ""
    
    case "$REPLY" in
        1)
            echo -e "${GREEN}Initializing TypeScript context...${NC}"
            "$INSTALL_DIR/$SCRIPT_NAME" --init ts
            echo -e "${GREEN}✓ Context initialized in $(pwd)/.claude/${NC}"
            ;;
        2)
            echo -e "${GREEN}Initializing Python context...${NC}"
            "$INSTALL_DIR/$SCRIPT_NAME" --init py
            echo -e "${GREEN}✓ Context initialized in $(pwd)/.claude/${NC}"
            ;;
        3)
            echo -e "${GREEN}Initializing text context...${NC}"
            "$INSTALL_DIR/$SCRIPT_NAME" --init txt
            echo -e "${GREEN}✓ Context initialized in $(pwd)/.claude/${NC}"
            ;;
        4|*)
            echo -e "${YELLOW}Skipping initialization.${NC}"
            echo "You can initialize context later with: c --init <type>"
            ;;
    esac
}

# Show completion message
show_completion() {
    echo ""
    echo -e "${GREEN}════════════════════════════════════════════${NC}"
    echo -e "${GREEN}  Installation Complete! 🎉${NC}"
    echo -e "${GREEN}════════════════════════════════════════════${NC}"
    echo ""
    echo "Commands Available (after restart):"
    echo "  c --help         Show help"
    echo "  c --init <type>  Initialize context (ts/py/txt)"
    echo "  c --show-context Show current context"
    echo "  c --bypass       Toggle bypass permissions mode"
    echo "  c --container    Toggle container mode"
    echo ""
    echo "Documentation: https://github.com/BuildAppolis/claude-context-wrapper"
    echo "Created by: BuildAppolis (www.buildappolis.com)"
}

# Development mode
if [[ "$1" == "--dev" ]]; then
    if [[ "$INSTALL_MODE" == "local" ]]; then
        echo -e "${YELLOW}Running in development mode...${NC}"
        INSTALL_DIR="$SCRIPT_DIR"
        echo "Using local directory for testing"
    else
        echo -e "${RED}Error: Development mode requires local installation${NC}"
        exit 1
    fi
fi

# Main installation flow
main() {
    detect_platform
    check_claude
    create_install_dir
    install_wrapper
    update_shell_config
    install_examples
    verify_installation
    show_completion
    
    # Ask about initialization only if not in dev mode or --no-init flag
    local skip_init=false
    for arg in "$@"; do
        if [[ "$arg" == "--no-init" ]] || [[ "$arg" == "--dev" ]]; then
            skip_init=true
            break
        fi
    done
    
    if [[ "$skip_init" == "false" ]]; then
        initialize_project
    fi
    
    # Final reminder about PATH
    echo ""
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}⚠  IMPORTANT: Restart your terminal!${NC}"
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "The 'c' command won't work until you:"
    echo -e "  ${GREEN}1.${NC} Close this terminal window/tab"
    echo -e "  ${GREEN}2.${NC} Open a new terminal"
    echo ""
    echo "Or run this command to reload your shell:"
    echo -e "  ${BLUE}source ~/.bashrc${NC}  (or ~/.zshrc for Zsh)"
    echo ""
}

# Handle errors
trap 'echo -e "\n${RED}Installation failed!${NC}"' ERR

# Run installation with all arguments
main "$@"