# Changelog

All notable changes to Claude Context Wrapper will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.1] - 2025-08-17

### Fixed
- Fixed `--help` command to show wrapper help by default instead of Claude's help
- Added `--help claude` option to show Claude's original help
- Improved installation script to show version being installed
- Fixed installation script to allow interaction after installation completes

### Changed
- Help command now prioritizes wrapper help over Claude help for better discoverability

## [1.2.0] - 2025-08-17

### Added
- Full pass-through support for all Claude flags (--continue, --resume, -r, -c, etc.)
- Smart detection to automatically detect Claude commands vs prompts
- Interactive sessions support when running `cc` without arguments
- Complete command compatibility with config, mcp, update, and all other Claude commands

### Changed
- Wrapper is now a complete drop-in replacement for Claude CLI
- Context is automatically injected into every interaction

## [1.1.0] - Previous Version

### Added
- Initial context wrapper functionality
- Support for TypeScript, Python, and text context files
- Git integration features
- Framework detection
- Global context support
- Bypass permissions mode
- Container mode for restricted access