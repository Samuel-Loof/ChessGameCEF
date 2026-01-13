# AI Chess Application (C++ / CEF / DirectX 11 / React)

A Windows desktop application combining native C++, hardware-accelerated rendering, and a modern React-based UI.  
The project explores off-screen browser rendering, native input handling, and integration between a DirectX rendering pipeline and a web-based frontend.

## Overview

The application embeds a Chromium-based UI using CEF (Chromium Embedded Framework) and renders it off-screen into a DirectX 11 backbuffer.  
Game logic and AI interaction are handled in the frontend, while rendering, input, and lifecycle management are controlled by the native C++ host.

## Key Features

### Native Desktop Application
- C++17 Windows application using CEF
- Explicit control over application lifecycle and rendering loop
- Custom input handling (mouse, keyboard, scroll)

### Rendering Pipeline
- Off-screen CEF rendering
- Direct transfer of rendered frames into a DirectX 11 backbuffer
- Hardware-accelerated rendering path
- Frame synchronization between native and browser layers

### Web-Based UI
- React + Next.js frontend written in TypeScript
- Chess logic implemented with `chess.js`
- UI rendered entirely inside the embedded browser

### AI Integration
- AI-driven opponent logic via OpenAI API
- Multiple configurable behavior profiles
- Real-time move analysis and commentary

## Architecture

```text
┌─────────────────────────────────────┐
│ Native Host Application (C++)       │
│ ├── CEF (Off-Screen Rendering)      │
│ ├── DirectX 11 Renderer             │
│ ├── Input Handling                  │
│ └── Application Lifecycle           │
└──────────────┬──────────────────────┘
               │ renders & communicates
               ↓
┌─────────────────────────────────────┐
│ Embedded Web UI (React / Next.js)   │
│ ├── Game Logic                      │
│ ├── UI Rendering                    │
│ └── AI API Integration              │
└─────────────────────────────────────┘
Technology Stack
Native Application
C++17

Chromium Embedded Framework (CEF)

DirectX 11

Windows API

CMake

Frontend
React

Next.js

TypeScript

Chess.js

OpenAI API

Tooling
Visual Studio 2026

Git

npm

Build & Run
Prerequisites
Visual Studio 2022 or later

CMake 3.19+

Node.js 18+

OpenAI API key

Build Steps
Native application

bash
Copy code
cd chess-game-cef
mkdir build && cd build
cmake .. -G "Visual Studio 18 2026" -A x64
cmake --build . --config Release
Frontend

bash
Copy code
cd chess-game-ui
npm install
npm run dev
Run

bash
Copy code
./ChessGameCEF.exe
Design Considerations
Separation of concerns between native rendering and UI logic

Off-screen rendering to allow full control over the graphics pipeline

Clear boundary between system-level responsibilities (C++) and application logic (React)

Type-safe frontend with predictable state management

Related Repositories
Frontend UI (React / Next.js):
https://github.com/yourusername/chess-game-ui

Backend (C# / ASP.NET Core Web API):
https://github.com/Samuel-Loof/ChessBackend
