# Claude Context Wrapper (CCW)

A smart context injection tool for Claude Code that automatically includes project-specific context, timestamps, and dynamic variables in every Claude Code prompt.

**Created by [BuildAppolis](https://www.buildappolis.com)**

## 🚀 Quick Start

```bash
# Clone and install
git clone https://github.com/BuildAppolis/claude-context-wrapper.git
cd claude-context-wrapper
./install.sh

# Initialize in any project
c --init ts  # For TypeScript projects
c --init py  # For Python projects
c --init txt # For simple text context

# Use Claude with automatic context
c "write a function to process user data"
```

## 📋 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Project Context Files](#project-context-files)
- [Examples](#examples)
- [Advanced Usage](#advanced-usage)
- [Contributing](#contributing)
- [License](#license)

## ✨ Features

- **Automatic Context Injection**: Automatically includes current date/time, working directory, and git information
- **Project-Specific Context**: Load dynamic context from `.claude/context.ts`, `.claude/context.py`, or `.claude/context.txt`
- **Dynamic Execution**: Context files are executed, allowing for real-time data collection
- **Framework Detection**: Automatically detects React, Vue, Next.js, Django, Flask, FastAPI, and more
- **Git Integration**: Includes current branch, last commit hash, and uncommitted changes status
- **Multiple Language Support**: Works with TypeScript, Python, and plain text context files
- **Global Context**: Set session-wide context that persists across commands
- **Zero Config**: Works out of the box with sensible defaults

## 📋 Requirements

The only requirement is having Claude Code CLI installed and configured:

```bash
# Install Claude Code (if not already installed)
# Visit: https://claude.ai/code
```

Other optional dependencies for enhanced functionality:
- Node.js (for TypeScript context files)
- Python 3 (for Python context files)
- Git (for git integration features)

## 📦 Installation

### Automatic Installation

```bash
curl -sSL https://raw.githubusercontent.com/BuildAppolis/claude-context-wrapper/main/install.sh | bash
```

### Manual Installation

1. Clone the repository:
```bash
git clone https://github.com/BuildAppolis/claude-context-wrapper.git
cd claude-context-wrapper
```

2. Run the installation script:
```bash
chmod +x install.sh
./install.sh
```

3. Restart your terminal or source your shell configuration:
```bash
source ~/.bashrc  # or ~/.zshrc
```

## 🎯 Usage

### Basic Commands

```bash
# Use Claude with automatic context
c "create a REST API endpoint"

# Initialize context for current project
c --init ts|py|txt

# Show current context
c --show-context

# Set global context for session
c --set-global "Working on authentication feature"

# Clear global context
c --clear-global

# Show help
c --help
```

### Context File Templates

When you run `c --init [type]`, it creates a `.claude/` folder with a context file:

#### TypeScript (`context.ts`)
```typescript
// .claude/context.ts
const context = {
    timestamp: new Date().toISOString(),
    project: process.cwd().split('/').pop(),
    nodeVersion: process.version,
    environment: process.env.NODE_ENV || 'development',
    // Add your custom context here
};

console.log(Object.entries(context)
    .map(([k, v]) => `${k}: ${v}`)
    .join(', '));
```

#### Python (`context.py`)
```python
#!/usr/bin/env python3
# .claude/context.py
import os
from datetime import datetime

context = {
    'timestamp': datetime.now().isoformat(),
    'project': os.path.basename(os.getcwd()),
    'environment': os.getenv('ENV', 'development'),
    # Add your custom context here
}

print(', '.join([f"{k}: {v}" for k, v in context.items()]))
```

#### Plain Text (`context.txt`)
```text
Project: My Project
Tech Stack: TypeScript, React, Node.js
Current Focus: User authentication
Database: PostgreSQL
```

## ⚙️ Configuration

### Environment Variables

```bash
# Set in your .bashrc/.zshrc or .env file
export CLAUDE_CONTEXT="Working on v2.0 release"  # Global context
export CCW_DEBUG=true                            # Enable debug output
export CCW_DISABLE_GIT=true                      # Disable git integration
export CCW_TIMEOUT=5                              # Timeout for context execution (seconds)
```

### Project Configuration

Create a `.claude/config.json` in your project:

```json
{
  "includeGitInfo": true,
  "includeNodeModules": false,
  "customContext": {
    "team": "backend",
    "sprint": "24",
    "priority": "authentication"
  },
  "excludePaths": ["dist", "build", ".next"],
  "contextTimeout": 3
}
```

## 📁 Project Context Files

### Dynamic Context Generation

The context files are executed at runtime, allowing you to:
- Gather real-time project statistics
- Check current git status
- Count files and lines of code
- Detect installed packages
- Read environment variables
- Execute any custom logic

See the `.claude/examples/` directory for advanced examples.

## 💡 Examples

### React Project Setup

```bash
# Initialize for a React project
cd my-react-app
c --init ts

# Now use Claude with React context
c "create a custom hook for authentication"
```

### Django Project Setup

```bash
# Initialize for a Django project
cd my-django-project
c --init py

# The context will automatically detect Django
c "create a new model for user profiles with proper migrations"
```

### Setting Project-Wide Context

```bash
# In your project root
echo "Sprint: 24
Team: Backend
Priority: Performance optimization
Database: PostgreSQL with Redis cache
API Version: v2.1.0
Deployment: Kubernetes on AWS" > .claude/context.txt

# This static context will be included in every command
c "optimize the database queries in the user service"
```

## 🔧 Advanced Usage

### Combining Multiple Context Sources

The tool combines context in this order:
1. Base context (date/time, working directory)
2. Project-specific context (`.claude/context.*`)
3. Global session context (`CLAUDE_CONTEXT` environment variable)
4. Command-line context (passed directly in the prompt)

### Debugging Context

```bash
# Enable debug mode to see how context is built
export CCW_DEBUG=true
c --show-context

# Check what context file is being loaded
ls -la .claude/

# Test context file execution
node .claude/context.ts  # For TypeScript
python .claude/context.py  # For Python
```

### CI/CD Integration

```yaml
# .github/workflows/claude-assist.yml
name: Claude Code Assistance

on:
  pull_request:
    types: [opened, edited]

jobs:
  assist:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Setup Claude Context
        run: |
          mkdir -p .claude
          echo "CI: true
          PR: ${{ github.event.pull_request.number }}
          Branch: ${{ github.head_ref }}
          Target: ${{ github.base_ref }}" > .claude/context.txt
      
      - name: Analyze with Claude
        run: |
          c "review this pull request and suggest improvements"
```

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details.

### Development Setup

```bash
# Clone the repo
git clone https://github.com/BuildAppolis/claude-context-wrapper.git
cd claude-context-wrapper

# Install locally for development
./install.sh --dev

# Run tests
./test.sh
```

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 About BuildAppolis

Created and maintained by [BuildAppolis](https://www.buildappolis.com) - Building powerful developer tools for the modern web.

## 📮 Support

- **Issues**: [GitHub Issues](https://github.com/BuildAppolis/claude-context-wrapper/issues)
- **Discussions**: [GitHub Discussions](https://github.com/BuildAppolis/claude-context-wrapper/discussions)
- **Website**: [www.buildappolis.com](https://www.buildappolis.com)

## 🚦 Roadmap

- [ ] VSCode extension
- [ ] Support for more languages (Ruby, Go, Rust)
- [ ] Context templates marketplace
- [ ] Team context sharing
- [ ] Context history and versioning
- [ ] Integration with other AI coding assistants
- [ ] Web dashboard for context management

---

Made with ❤️ by [BuildAppolis](https://www.buildappolis.com) for developers using Claude Code