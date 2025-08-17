// Advanced TypeScript Context Example
// This shows how to create a comprehensive context file for TypeScript projects

import { execSync } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';

interface ContextData {
    [key: string]: any;
}

// Get git information
const getGitInfo = (): ContextData => {
    try {
        return {
            branch: execSync('git branch --show-current', { encoding: 'utf8' }).trim(),
            commit: execSync('git rev-parse --short HEAD', { encoding: 'utf8' }).trim(),
            uncommittedChanges: execSync('git status --porcelain', { encoding: 'utf8' }).length > 0,
            lastCommitMessage: execSync('git log -1 --pretty=%B', { encoding: 'utf8' }).trim().substring(0, 50),
            commitCount: parseInt(execSync('git rev-list --count HEAD', { encoding: 'utf8' }).trim())
        };
    } catch {
        return {};
    }
};

// Detect project type and frameworks
const detectProjectType = (): ContextData => {
    const info: ContextData = {};
    
    if (fs.existsSync('package.json')) {
        const pkg = JSON.parse(fs.readFileSync('package.json', 'utf8'));
        info.projectName = pkg.name;
        info.version = pkg.version;
        
        const deps = { ...pkg.dependencies, ...pkg.devDependencies };
        
        // Detect frameworks
        if (deps.react) info.framework = deps.next ? 'Next.js' : 'React';
        if (deps.vue) info.framework = deps.nuxt ? 'Nuxt' : 'Vue';
        if (deps.angular) info.framework = 'Angular';
        if (deps.svelte) info.framework = deps.sveltekit ? 'SvelteKit' : 'Svelte';
        
        // Detect tools
        info.hasTypeScript = !!deps.typescript;
        info.hasESLint = !!deps.eslint;
        info.hasPrettier = !!deps.prettier;
        info.testRunner = deps.jest ? 'Jest' : deps.vitest ? 'Vitest' : deps.mocha ? 'Mocha' : null;
        
        // Detect build tools
        if (deps.vite) info.buildTool = 'Vite';
        else if (deps.webpack) info.buildTool = 'Webpack';
        else if (deps.parcel) info.buildTool = 'Parcel';
        else if (deps.rollup) info.buildTool = 'Rollup';
    }
    
    return info;
};

// Get file statistics
const getProjectStats = (): ContextData => {
    try {
        const stats: ContextData = {};
        
        // Count TypeScript/JavaScript files
        const tsFiles = execSync('find . -name "*.ts" -o -name "*.tsx" 2>/dev/null | wc -l', { encoding: 'utf8' }).trim();
        const jsFiles = execSync('find . -name "*.js" -o -name "*.jsx" 2>/dev/null | wc -l', { encoding: 'utf8' }).trim();
        
        stats.typeScriptFiles = parseInt(tsFiles);
        stats.javaScriptFiles = parseInt(jsFiles);
        
        // Check for important files
        stats.hasDockerfile = fs.existsSync('Dockerfile');
        stats.hasCI = fs.existsSync('.github/workflows') || fs.existsSync('.gitlab-ci.yml');
        stats.hasTests = fs.existsSync('__tests__') || fs.existsSync('test') || fs.existsSync('tests');
        
        return stats;
    } catch {
        return {};
    }
};

// Get environment info
const getEnvironmentInfo = (): ContextData => {
    return {
        nodeVersion: process.version,
        platform: process.platform,
        arch: process.arch,
        currentDirectory: process.cwd(),
        timestamp: new Date().toISOString(),
        dayOfWeek: new Date().toLocaleDateString('en-US', { weekday: 'long' }),
        timeOfDay: new Date().getHours() < 12 ? 'morning' : new Date().getHours() < 17 ? 'afternoon' : 'evening'
    };
};

// Main context generation
const generateContext = (): string => {
    const context: ContextData = {
        ...getEnvironmentInfo(),
        ...detectProjectType(),
        ...getGitInfo(),
        ...getProjectStats()
    };
    
    // Load custom config if exists
    if (fs.existsSync('.claude/ccw.config.json')) {
        const config = JSON.parse(fs.readFileSync('.claude/ccw.config.json', 'utf8'));
        if (config.customContext) {
            Object.assign(context, config.customContext);
        }
    }
    
    // Filter out undefined values and format
    return Object.entries(context)
        .filter(([_, value]) => value !== undefined && value !== null)
        .map(([key, value]) => {
            if (typeof value === 'boolean') return `${key}: ${value ? 'yes' : 'no'}`;
            return `${key}: ${value}`;
        })
        .join(', ');
};

// Output the context
console.log(generateContext());