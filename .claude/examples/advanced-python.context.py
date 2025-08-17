#!/usr/bin/env python3
# Advanced Python Context Example
# This shows how to create a comprehensive context file for Python projects

import os
import sys
import json
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Dict, Any

def run_command(cmd: list) -> str:
    """Run a shell command and return output."""
    try:
        return subprocess.check_output(cmd, text=True, stderr=subprocess.DEVNULL).strip()
    except:
        return ""

def get_git_info() -> Dict[str, Any]:
    """Get git repository information."""
    info = {}
    
    if Path('.git').exists():
        info['branch'] = run_command(['git', 'branch', '--show-current'])
        info['commit'] = run_command(['git', 'rev-parse', '--short', 'HEAD'])
        info['last_commit_msg'] = run_command(['git', 'log', '-1', '--pretty=%B'])[:50]
        
        status = run_command(['git', 'status', '--porcelain'])
        info['has_uncommitted'] = len(status) > 0
        
        # Count commits
        commit_count = run_command(['git', 'rev-list', '--count', 'HEAD'])
        if commit_count:
            info['commit_count'] = int(commit_count)
    
    return info

def detect_python_project() -> Dict[str, Any]:
    """Detect Python project type and configuration."""
    info = {}
    
    # Check for different Python files
    if Path('requirements.txt').exists():
        info['deps_manager'] = 'pip'
        with open('requirements.txt') as f:
            reqs = f.read().lower()
            if 'django' in reqs:
                info['framework'] = 'Django'
            elif 'flask' in reqs:
                info['framework'] = 'Flask'
            elif 'fastapi' in reqs:
                info['framework'] = 'FastAPI'
            elif 'pytest' in reqs:
                info['test_framework'] = 'pytest'
    
    if Path('Pipfile').exists():
        info['deps_manager'] = 'pipenv'
    
    if Path('pyproject.toml').exists():
        info['has_pyproject'] = True
        try:
            import toml
            with open('pyproject.toml') as f:
                pyproject = toml.load(f)
                if 'project' in pyproject:
                    info['project_name'] = pyproject['project'].get('name')
                    info['version'] = pyproject['project'].get('version')
                if 'tool' in pyproject:
                    if 'poetry' in pyproject['tool']:
                        info['deps_manager'] = 'poetry'
                    if 'black' in pyproject['tool']:
                        info['formatter'] = 'black'
                    if 'ruff' in pyproject['tool']:
                        info['linter'] = 'ruff'
        except ImportError:
            pass
    
    # Check for Django
    if Path('manage.py').exists():
        info['framework'] = 'Django'
        # Try to get Django settings
        settings_module = os.environ.get('DJANGO_SETTINGS_MODULE', '')
        if settings_module:
            info['django_settings'] = settings_module
    
    # Check for Flask/FastAPI
    for file in ['app.py', 'main.py', 'application.py']:
        if Path(file).exists():
            with open(file) as f:
                content = f.read()
                if 'from flask' in content or 'import flask' in content:
                    info['framework'] = 'Flask'
                elif 'from fastapi' in content or 'import fastapi' in content:
                    info['framework'] = 'FastAPI'
    
    return info

def get_project_stats() -> Dict[str, Any]:
    """Get project statistics."""
    stats = {}
    
    # Count Python files
    py_files = list(Path('.').rglob('*.py'))
    stats['python_files'] = len(py_files)
    
    # Count test files
    test_files = [f for f in py_files if 'test' in f.name.lower()]
    stats['test_files'] = len(test_files)
    
    # Check for important files/directories
    stats['has_dockerfile'] = Path('Dockerfile').exists()
    stats['has_tests'] = any(Path(d).exists() for d in ['tests', 'test', '__tests__'])
    stats['has_docs'] = Path('docs').exists() or Path('documentation').exists()
    stats['has_ci'] = Path('.github/workflows').exists() or Path('.gitlab-ci.yml').exists()
    
    # Check for virtual environment
    stats['has_venv'] = any(Path(d).exists() for d in ['venv', '.venv', 'env', '.env'])
    
    return stats

def get_environment_info() -> Dict[str, Any]:
    """Get environment information."""
    now = datetime.now()
    
    return {
        'timestamp': now.isoformat(),
        'day_of_week': now.strftime('%A'),
        'time_of_day': 'morning' if now.hour < 12 else 'afternoon' if now.hour < 17 else 'evening',
        'python_version': f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.patch}",
        'platform': sys.platform,
        'cwd': os.getcwd(),
        'project_name': Path.cwd().name,
        'virtualenv': os.environ.get('VIRTUAL_ENV', '').split('/')[-1] if os.environ.get('VIRTUAL_ENV') else None
    }

def load_custom_config() -> Dict[str, Any]:
    """Load custom configuration from .claude/config.json."""
    config_path = Path('.claude/config.json')
    if config_path.exists():
        try:
            with open(config_path) as f:
                config = json.load(f)
                return config.get('customContext', {})
        except:
            pass
    return {}

def main():
    """Generate and output context."""
    context = {
        **get_environment_info(),
        **detect_python_project(),
        **get_git_info(),
        **get_project_stats(),
        **load_custom_config()
    }
    
    # Filter out None values and format
    context_items = []
    for key, value in context.items():
        if value is not None:
            if isinstance(value, bool):
                context_items.append(f"{key}: {'yes' if value else 'no'}")
            else:
                context_items.append(f"{key}: {value}")
    
    print(', '.join(context_items))

if __name__ == '__main__':
    main()