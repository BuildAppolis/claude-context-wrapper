#!/bin/bash

# Claude Context Wrapper
# Created by BuildAppolis (www.buildappolis.com)
# Version: 1.1.0

set -e

# Colors for output (cross-platform compatible)
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
    # Windows (Git Bash, Cygwin)
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
else
    # Unix-like systems (Linux, macOS, WSL)
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m' # No Color
fi

# Configuration
CONTEXT_DIR=".claude"
GLOBAL_CONTEXT_FILE="$HOME/.claude-global-context"
BYPASS_STATE_FILE="$HOME/.claude-bypass-state"
CONTAINER_STATE_FILE="$HOME/.claude-container-state"
VERSION="1.2.0"

# ============================================
# CUSTOMIZABLE LAUNCH COMMAND
# ============================================
# You can customize the Claude command here.
# Default: "claude"
# Examples:
#   - For a specific binary path: "/usr/local/bin/claude"
#   - For an alias: "claude-dev"
#   - For a wrapper: "npx claude"
CLAUDE_COMMAND="${CLAUDE_COMMAND:-claude}"

# You can also set this via environment variable:
# export CLAUDE_COMMAND="your-custom-command"
# ============================================

# Function to show help
show_help() {
    cat << EOF
Claude Context Wrapper v${VERSION}
Created by BuildAppolis (www.buildappolis.com)

Usage: cc [OPTIONS] [prompt]

When run without arguments, opens interactive Claude session with context.
All Claude native commands and flags are supported (--continue, --resume, etc.)

WRAPPER OPTIONS:
    --init <type>     Initialize context file (ts, py, or txt)
    --show-context    Display current context that would be injected
    --set-global      Set global context for current session
    --clear-global    Clear global context
    --bypass          Toggle dangerous bypass permissions mode
    --container       Toggle container mode (restricts to current directory)
    --help wrapper    Show this wrapper help
    --version         Show wrapper and Claude versions

EXAMPLES:
    cc                          Open interactive Claude session
    cc "create a REST API"      Run single prompt with context
    cc --continue               Continue previous session with context
    cc --resume                 Resume a conversation with context
    cc --init ts                Initialize TypeScript context
    cc --set-global "Sprint 24" Set global context
    cc --show-context           Show current context

ENVIRONMENT VARIABLES:
    CLAUDE_CONTEXT    Global context string
    CCW_DEBUG         Enable debug output (true/false)
    CCW_DISABLE_GIT   Disable git integration (true/false)
    CCW_TIMEOUT       Context execution timeout in seconds (default: 3)

For more information, visit: https://github.com/BuildAppolis/claude-context-wrapper
EOF
}

# Function to get base context
get_base_context() {
    local context=""
    
    # Add timestamp and basic info
    context="[Context: $(date '+%Y-%m-%d %H:%M:%S'), pwd: $(pwd)"
    
    # Add git info if available and not disabled
    if [[ "$CCW_DISABLE_GIT" != "true" ]] && command -v git &> /dev/null && git rev-parse --git-dir &> /dev/null 2>&1; then
        local branch=$(git branch --show-current 2>/dev/null || echo "unknown")
        local commit=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
        local changes=""
        if [[ -n $(git status --porcelain 2>/dev/null) ]]; then
            changes=", uncommitted changes"
        fi
        context="${context}, git: ${branch}@${commit}${changes}"
    fi
    
    echo "$context"
}

# Function to get project context
get_project_context() {
    local context=""
    local timeout="${CCW_TIMEOUT:-3}"
    
    # Check for TypeScript context
    if [[ -f "${CONTEXT_DIR}/context.ts" ]]; then
        if command -v node &> /dev/null; then
            local ts_context=$(timeout ${timeout}s node "${CONTEXT_DIR}/context.ts" 2>/dev/null || echo "")
            if [[ -n "$ts_context" ]]; then
                context="${context}, ${ts_context}"
            fi
        fi
    # Check for Python context
    elif [[ -f "${CONTEXT_DIR}/context.py" ]]; then
        if command -v python3 &> /dev/null; then
            local py_context=$(timeout ${timeout}s python3 "${CONTEXT_DIR}/context.py" 2>/dev/null || echo "")
            if [[ -n "$py_context" ]]; then
                context="${context}, ${py_context}"
            fi
        elif command -v python &> /dev/null; then
            local py_context=$(timeout ${timeout}s python "${CONTEXT_DIR}/context.py" 2>/dev/null || echo "")
            if [[ -n "$py_context" ]]; then
                context="${context}, ${py_context}"
            fi
        fi
    # Check for text context
    elif [[ -f "${CONTEXT_DIR}/context.txt" ]]; then
        local txt_context=$(cat "${CONTEXT_DIR}/context.txt" 2>/dev/null | tr '\n' ', ' | sed 's/, $//')
        if [[ -n "$txt_context" ]]; then
            context="${context}, ${txt_context}"
        fi
    fi
    
    echo "$context"
}

# Function to get global context
get_global_context() {
    local context=""
    
    # Check environment variable first
    if [[ -n "$CLAUDE_CONTEXT" ]]; then
        context=", global: $CLAUDE_CONTEXT"
    # Check global context file
    elif [[ -f "$GLOBAL_CONTEXT_FILE" ]]; then
        local file_context=$(cat "$GLOBAL_CONTEXT_FILE" 2>/dev/null)
        if [[ -n "$file_context" ]]; then
            context=", global: $file_context"
        fi
    fi
    
    echo "$context"
}

# Function to initialize context file
init_context() {
    local type="$1"
    
    # Create .claude directory if it doesn't exist
    mkdir -p "$CONTEXT_DIR"
    
    case "$type" in
        ts|typescript)
            cat > "${CONTEXT_DIR}/context.ts" << 'EOF'
// Claude Context - TypeScript
// This file is executed to provide dynamic context to Claude Code

const context = {
    timestamp: new Date().toISOString(),
    project: process.cwd().split('/').pop(),
    nodeVersion: process.version,
    environment: process.env.NODE_ENV || 'development',
    // Add your custom context here
};

// Output context as comma-separated key-value pairs
console.log(Object.entries(context)
    .map(([k, v]) => `${k}: ${v}`)
    .join(', '));
EOF
            echo -e "${GREEN}✓${NC} Created ${CONTEXT_DIR}/context.ts"
            echo "Edit this file to customize your TypeScript context"
            ;;
            
        py|python)
            cat > "${CONTEXT_DIR}/context.py" << 'EOF'
#!/usr/bin/env python3
# Claude Context - Python
# This file is executed to provide dynamic context to Claude Code

import os
from datetime import datetime

context = {
    'timestamp': datetime.now().isoformat(),
    'project': os.path.basename(os.getcwd()),
    'environment': os.getenv('ENV', 'development'),
    # Add your custom context here
}

# Output context as comma-separated key-value pairs
print(', '.join([f"{k}: {v}" for k, v in context.items()]))
EOF
            chmod +x "${CONTEXT_DIR}/context.py"
            echo -e "${GREEN}✓${NC} Created ${CONTEXT_DIR}/context.py"
            echo "Edit this file to customize your Python context"
            ;;
            
        txt|text)
            cat > "${CONTEXT_DIR}/context.txt" << 'EOF'
Project: My Project
Tech Stack: [Add your tech stack]
Current Focus: [Add current focus]
Notes: [Add any relevant notes]
EOF
            echo -e "${GREEN}✓${NC} Created ${CONTEXT_DIR}/context.txt"
            echo "Edit this file to customize your text context"
            ;;
            
        *)
            echo -e "${RED}Error:${NC} Invalid type. Use 'ts', 'py', or 'txt'"
            exit 1
            ;;
    esac
}

# Function to show current context
show_context() {
    local base_context=$(get_base_context)
    local project_context=$(get_project_context)
    local global_context=$(get_global_context)
    
    # Check bypass mode
    local bypass_context=""
    if [[ -f "$BYPASS_STATE_FILE" ]]; then
        bypass_context=", BYPASS_PERMISSIONS: ENABLED"
    fi
    
    # Check container mode
    local container_context=""
    if [[ -f "$CONTAINER_STATE_FILE" ]]; then
        container_context=", CONTAINER: $(cat "$CONTAINER_STATE_FILE")"
    fi
    
    echo -e "${BLUE}=== Claude Context Wrapper ===${NC}"
    echo -e "${YELLOW}Current context that would be injected:${NC}"
    echo "${base_context}${project_context}${global_context}${bypass_context}${container_context}]"
    echo ""
    
    # Show which context file is being used
    if [[ -f "${CONTEXT_DIR}/context.ts" ]]; then
        echo -e "${GREEN}Using context file:${NC} ${CONTEXT_DIR}/context.ts"
    elif [[ -f "${CONTEXT_DIR}/context.py" ]]; then
        echo -e "${GREEN}Using context file:${NC} ${CONTEXT_DIR}/context.py"
    elif [[ -f "${CONTEXT_DIR}/context.txt" ]]; then
        echo -e "${GREEN}Using context file:${NC} ${CONTEXT_DIR}/context.txt"
    else
        echo -e "${YELLOW}No project context file found.${NC} Use 'cc --init <type>' to create one."
    fi
    
    if [[ -n "$CLAUDE_CONTEXT" ]]; then
        echo -e "${GREEN}Global context (env):${NC} $CLAUDE_CONTEXT"
    elif [[ -f "$GLOBAL_CONTEXT_FILE" ]]; then
        echo -e "${GREEN}Global context (file):${NC} $(cat "$GLOBAL_CONTEXT_FILE")"
    fi
    
    # Show bypass mode status
    if [[ -f "$BYPASS_STATE_FILE" ]]; then
        echo -e "${RED}⚠ BYPASS MODE:${NC} ENABLED - Safety checks bypassed!"
    else
        echo -e "${GREEN}Permissions:${NC} Normal mode"
    fi
    
    # Show container mode status
    if [[ -f "$CONTAINER_STATE_FILE" ]]; then
        echo -e "${BLUE}📦 CONTAINER:${NC} Restricted to $(cat "$CONTAINER_STATE_FILE")"
    fi
}

# Function to set global context
set_global_context() {
    local context="$1"
    echo "$context" > "$GLOBAL_CONTEXT_FILE"
    echo -e "${GREEN}✓${NC} Global context set: $context"
}

# Function to clear global context
clear_global_context() {
    rm -f "$GLOBAL_CONTEXT_FILE"
    unset CLAUDE_CONTEXT
    echo -e "${GREEN}✓${NC} Global context cleared"
}

# Function to toggle container mode
toggle_container_mode() {
    if [[ -f "$CONTAINER_STATE_FILE" ]]; then
        # Container mode is currently ON, turn it OFF
        rm -f "$CONTAINER_STATE_FILE"
        echo -e "${GREEN}✓${NC} Container mode DISABLED"
        echo -e "${YELLOW}Claude can now access files outside current directory${NC}"
    else
        # Container mode is currently OFF, turn it ON
        # Store container config as JSON
        local container_config="{\"root\":\"$(pwd)\",\"allowed\":[]}"
        
        # Check for allowed directories in .claude/ccw.config.json
        if [[ -f "${CONTEXT_DIR}/ccw.config.json" ]]; then
            # Use tr to remove newlines for easier parsing
            local allowed_dirs=$(cat "${CONTEXT_DIR}/ccw.config.json" 2>/dev/null | tr '\n' ' ' | grep -o '"allowedDirectories"[[:space:]]*:[[:space:]]*\[[^]]*\]' | sed 's/.*\[/[/' | sed 's/\]/]/' || echo "[]")
            if [[ "$allowed_dirs" != "[]" ]]; then
                container_config="{\"root\":\"$(pwd)\",\"allowed\":$allowed_dirs}"
                echo -e "${BLUE}Additional allowed directories from config:${NC}"
                echo "$allowed_dirs" | sed 's/[][]//g' | tr ',' '\n' | sed 's/"//g' | while read dir; do
                    [[ -n "$dir" ]] && echo "  - $dir"
                done
            fi
        fi
        
        echo "$container_config" > "$CONTAINER_STATE_FILE"
        echo -e "${GREEN}✓${NC} Container mode ENABLED"
        echo -e "${BLUE}Container root:${NC} $(pwd)"
        echo -e "${YELLOW}Claude is restricted to this directory and subdirectories${NC}"
        echo -e "${GREEN}Global context:${NC} Still accessible (read-only)"
        
        if [[ -f "$BYPASS_STATE_FILE" ]]; then
            echo ""
            echo -e "${GREEN}Safer bypass mode active:${NC}"
            echo "- File modifications without permission: YES"
            echo "- Restricted to container: YES"
            echo "- Global context access: YES"
        fi
    fi
    
    # Show current state
    if [[ -f "$CONTAINER_STATE_FILE" ]]; then
        local config=$(cat "$CONTAINER_STATE_FILE")
        local root=$(echo "$config" | grep -o '"root":"[^"]*"' | cut -d'"' -f4)
        echo "Current state: CONTAINER ON (root: $root)"
    else
        echo "Current state: No container restrictions"
    fi
}

# Function to toggle bypass permissions
toggle_bypass_permissions() {
    if [[ -f "$BYPASS_STATE_FILE" ]]; then
        # Bypass is currently ON, turn it OFF
        rm -f "$BYPASS_STATE_FILE"
        echo -e "${GREEN}✓${NC} Bypass permissions mode DISABLED"
        echo -e "${YELLOW}Normal file permissions restored${NC}"
    else
        # Bypass is currently OFF, turn it ON
        echo "enabled" > "$BYPASS_STATE_FILE"
        echo -e "${YELLOW}⚠ WARNING: Bypass permissions mode ENABLED${NC}"
        echo -e "${RED}This will use --dangerously-skip-permissions flag${NC}"
        echo -e "${RED}Claude can modify ANY files without asking!${NC}"
        echo ""
        echo -e "${YELLOW}For safer operation, consider using --container mode${NC}"
    fi
    
    # Show current state
    if [[ -f "$BYPASS_STATE_FILE" ]]; then
        echo "Current state: BYPASS ON (--dangerously-skip-permissions)"
    else
        echo "Current state: Normal mode (file modifications require permission)"
    fi
}

# Function to build context
build_context() {
    local base_context=$(get_base_context)
    local project_context=$(get_project_context)
    local global_context=$(get_global_context)
    
    # Add bypass mode to context if enabled
    local bypass_context=""
    if [[ -f "$BYPASS_STATE_FILE" ]]; then
        bypass_context=", BYPASS_PERMISSIONS: ENABLED"
    fi
    
    # Add container mode to context if enabled
    local container_context=""
    if [[ -f "$CONTAINER_STATE_FILE" ]]; then
        local container_config=$(cat "$CONTAINER_STATE_FILE")
        local container_root=$(echo "$container_config" | grep -o '"root":"[^"]*"' | cut -d'"' -f4)
        container_context=", CONTAINER: $container_root"
        
        # Check if current directory is within container or allowed directories
        local is_allowed=false
        
        # Check if we're in the container root
        if [[ "$(pwd)" == "$container_root"* ]]; then
            is_allowed=true
        else
            # Check allowed directories
            local allowed_dirs=$(echo "$container_config" | grep -o '"allowed":\[[^]]*\]' | sed 's/.*\[//' | sed 's/\]//' | tr ',' '\n' | sed 's/"//g' | sed 's/[[:space:]]//g')
            while IFS= read -r allowed_dir; do
                if [[ -n "$allowed_dir" ]]; then
                    # Expand path if it starts with ~
                    allowed_dir="${allowed_dir/#\~/$HOME}"
                    if [[ "$(pwd)" == "$allowed_dir"* ]]; then
                        is_allowed=true
                        container_context="$container_context (allowed: $allowed_dir)"
                        break
                    fi
                fi
            done <<< "$allowed_dirs"
        fi
        
        if [[ "$is_allowed" != "true" ]]; then
            echo -e "${RED}Error:${NC} Current directory is outside container and allowed directories!"
            echo -e "Container root: $container_root"
            echo -e "Current directory: $(pwd)"
            echo -e "${YELLOW}Please cd to container directory or disable container mode${NC}"
            exit 1
        fi
    fi
    
    echo "${base_context}${project_context}${global_context}${bypass_context}${container_context}]"
}

# Main logic
main() {
    # Check if claude command exists
    if ! command -v "$CLAUDE_COMMAND" &> /dev/null; then
        echo -e "${RED}Error:${NC} Claude Code CLI not found at: $CLAUDE_COMMAND"
        echo "Please install Claude Code or set CLAUDE_COMMAND environment variable"
        echo "Visit: https://claude.ai/code"
        exit 1
    fi
    
    # Handle our wrapper-specific commands first
    case "$1" in
        --help)
            # Check if this is for our wrapper
            if [[ "$2" == "wrapper" ]]; then
                show_help
                exit 0
            fi
            # Otherwise pass through to Claude
            ;;
            
        --version)
            # Show both versions
            echo "Claude Context Wrapper v${VERSION}"
            echo "Created by BuildAppolis (www.buildappolis.com)"
            echo ""
            $CLAUDE_COMMAND --version
            exit 0
            ;;
            
        --init)
            if [[ -z "$2" ]]; then
                echo -e "${RED}Error:${NC} Please specify type: ts, py, or txt"
                exit 1
            fi
            init_context "$2"
            exit 0
            ;;
            
        --show-context)
            show_context
            exit 0
            ;;
            
        --set-global)
            if [[ -z "$2" ]]; then
                echo -e "${RED}Error:${NC} Please provide context string"
                exit 1
            fi
            set_global_context "$2"
            exit 0
            ;;
            
        --clear-global)
            clear_global_context
            exit 0
            ;;
            
        --bypass)
            toggle_bypass_permissions
            exit 0
            ;;
            
        --container)
            toggle_container_mode
            exit 0
            ;;
    esac
    
    # Build context for all Claude operations
    local full_context=$(build_context)
    
    # Debug mode
    if [[ "$CCW_DEBUG" == "true" ]]; then
        echo -e "${YELLOW}[DEBUG] Context:${NC} $full_context" >&2
        echo -e "${YELLOW}[DEBUG] Arguments:${NC} $*" >&2
    fi
    
    # Build Claude command with context
    local claude_args=()
    
    # Add bypass flag if enabled
    if [[ -f "$BYPASS_STATE_FILE" ]]; then
        claude_args+=("--dangerously-skip-permissions")
    fi
    
    # Check if we have arguments or not
    if [[ $# -eq 0 ]]; then
        # No arguments - start interactive session
        echo -e "${BLUE}Starting Claude with context...${NC}"
        echo -e "${YELLOW}Context:${NC} ${full_context}"
        echo ""
        
        claude_args+=("--append-system-prompt" "$full_context")
    else
        # We have arguments - check what they are
        local is_prompt=true
        
        # Check if first arg is a Claude flag or command
        if [[ "$1" =~ ^- ]] || [[ "$1" =~ ^(config|mcp|migrate-installer|setup-token|doctor|update|install)$ ]]; then
            is_prompt=false
        fi
        
        if [[ "$is_prompt" == "false" ]]; then
            # It's a Claude command/flag - inject context via system prompt
            claude_args+=("--append-system-prompt" "$full_context")
            claude_args+=("$@")
            
            # Show context for interactive commands
            if [[ "$1" == "-c" ]] || [[ "$1" == "--continue" ]] || [[ "$1" == "-r" ]] || [[ "$1" == "--resume" ]]; then
                echo -e "${BLUE}Continuing/Resuming Claude with context...${NC}"
                echo -e "${YELLOW}Context:${NC} ${full_context}"
                echo ""
            fi
        else
            # It's a prompt - prepend context to it
            local full_prompt="${full_context} $*"
            claude_args+=("$full_prompt")
        fi
    fi
    
    # Execute Claude with built arguments
    $CLAUDE_COMMAND "${claude_args[@]}"
}

# Run main function with all arguments
main "$@"