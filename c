#!/bin/bash

# Claude Context Wrapper
# Created by BuildAppolis (www.buildappolis.com)
# Version: 1.0.0

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
VERSION="1.0.0"

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

Usage: c [OPTIONS] "prompt"

OPTIONS:
    --init <type>     Initialize context file (ts, py, or txt)
    --show-context    Display current context that would be injected
    --set-global      Set global context for current session
    --clear-global    Clear global context
    --help            Show this help message
    --version         Show version information

EXAMPLES:
    c "create a REST API endpoint"
    c --init ts
    c --set-global "Working on authentication"
    c --show-context

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
    
    echo -e "${BLUE}=== Claude Context Wrapper ===${NC}"
    echo -e "${YELLOW}Current context that would be injected:${NC}"
    echo "${base_context}${project_context}${global_context}]"
    echo ""
    
    # Show which context file is being used
    if [[ -f "${CONTEXT_DIR}/context.ts" ]]; then
        echo -e "${GREEN}Using context file:${NC} ${CONTEXT_DIR}/context.ts"
    elif [[ -f "${CONTEXT_DIR}/context.py" ]]; then
        echo -e "${GREEN}Using context file:${NC} ${CONTEXT_DIR}/context.py"
    elif [[ -f "${CONTEXT_DIR}/context.txt" ]]; then
        echo -e "${GREEN}Using context file:${NC} ${CONTEXT_DIR}/context.txt"
    else
        echo -e "${YELLOW}No project context file found.${NC} Use 'c --init <type>' to create one."
    fi
    
    if [[ -n "$CLAUDE_CONTEXT" ]]; then
        echo -e "${GREEN}Global context (env):${NC} $CLAUDE_CONTEXT"
    elif [[ -f "$GLOBAL_CONTEXT_FILE" ]]; then
        echo -e "${GREEN}Global context (file):${NC} $(cat "$GLOBAL_CONTEXT_FILE")"
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

# Main logic
main() {
    # Check if claude command exists
    if ! command -v "$CLAUDE_COMMAND" &> /dev/null; then
        echo -e "${RED}Error:${NC} Claude Code CLI not found at: $CLAUDE_COMMAND"
        echo "Please install Claude Code or set CLAUDE_COMMAND environment variable"
        echo "Visit: https://claude.ai/code"
        exit 1
    fi
    
    # Parse arguments
    case "$1" in
        --help|-h)
            show_help
            exit 0
            ;;
            
        --version|-v)
            echo "Claude Context Wrapper v${VERSION}"
            echo "Created by BuildAppolis (www.buildappolis.com)"
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
            
        "")
            echo -e "${RED}Error:${NC} No prompt provided"
            echo "Usage: c \"your prompt here\""
            echo "Use 'c --help' for more information"
            exit 1
            ;;
            
        *)
            # Build full context
            local base_context=$(get_base_context)
            local project_context=$(get_project_context)
            local global_context=$(get_global_context)
            local full_context="${base_context}${project_context}${global_context}]"
            
            # Debug mode
            if [[ "$CCW_DEBUG" == "true" ]]; then
                echo -e "${YELLOW}[DEBUG] Context:${NC} $full_context" >&2
                echo -e "${YELLOW}[DEBUG] Prompt:${NC} $*" >&2
            fi
            
            # Combine context with user prompt
            local full_prompt="${full_context} $*"
            
            # Pass to claude
            $CLAUDE_COMMAND "$full_prompt"
            ;;
    esac
}

# Run main function with all arguments
main "$@"