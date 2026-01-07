# AI Chess Game - CEF + DirectX11 + React

An advanced desktop chess application combining native C++ with modern web technologies.

![Screenshot](screenshot.png)

## 🎮 Features

- **Native C++ Desktop App** with CEF (Chromium Embedded Framework)
- **Hardware-Accelerated Rendering** using DirectX 11
- **Modern React UI** with TypeScript and Next.js
- **AI Opponents** powered by OpenAI API with 8 unique personalities
- **Off-Screen Rendering** - CEF → Backbuffer → DirectX pipeline
- **Full Input Handling** - Mouse, keyboard, scroll support
- **Real-time Commentary** - AI provides personality-driven move analysis
- **Sound Effects** - Dynamic audio feedback
- **Move Highlighting** - Visual feedback for last moves

## 🏗️ Architecture
```
┌─────────────────────────────────────┐
│  C++ Desktop App (ChessGameCEF.exe) │
│  ├── CEF (Chromium Engine)          │
│  ├── DirectX 11 Renderer            │
│  └── Input Handler                  │
└──────────────┬──────────────────────┘
               │ loads
               ↓
┌──────────────────────────────────────┐
│  React Frontend (chess-ui)           │
│  ├── Next.js + TypeScript            │
│  ├── Chess Logic (chess.js)          │
│  └── AI Integration (OpenAI)         │
└──────────────────────────────────────┘
```

## 🚀 Getting Started

### Prerequisites
- Visual Studio 2022/2026
- CMake 3.19+
- Node.js 18+
- OpenAI API key

### Build Instructions

**1. Clone the repositories:**
```bash
git clone https://github.com/yourusername/chess-game-cef.git
git clone https://github.com/yourusername/chess-game-ui.git
```

**2. Download CEF:**
- Download from [CEF Builds](https://cef-builds.spotifycdn.com/index.html)
- Extract to `chess-game-cef/cef/`

**3. Build C++ Application:**
```bash
cd chess-game-cef
mkdir build && cd build
cmake .. -G "Visual Studio 18 2026" -A x64
cmake --build . --config Release
```

**4. Run Frontend:**
```bash
cd chess-game-ui
npm install
npm run dev
```

**5. Launch Application:**
```bash
cd chess-game-cef/build/Release
./ChessGameCEF.exe
```

## 🤖 AI Opponents

Choose from 8 unique personalities:
- **Friendly Fred** - Supportive and encouraging
- **Cocky Carl** - Overconfident trash-talker
- **Professor Pat** - Educational and analytical
- **Zen Master Zara** - Philosophical and calm
- **Mysterious Magnus** - Silent and calculating
- **Chatty Charlie** - Can't stop talking
- **Dramatic Diana** - Theatrical performances
- **Newbie Nina** - Learning and making mistakes

## 🛠️ Tech Stack

**Native Application:**
- C++17
- CEF (Chromium Embedded Framework)
- DirectX 11
- CMake

**Frontend:**
- React + Next.js
- TypeScript
- Chess.js
- OpenAI API
- Tailwind CSS

**Tools:**
- Visual Studio 2026
- Git
- npm

## 📸 Screenshots

In progress...

## 🎯 Why This Project?

This project demonstrates:
- **Multi-language proficiency** - C++, TypeScript, JavaScript
- **Native development** - DirectX, CEF, Windows API
- **Modern web tech** - React, Next.js, REST APIs
- **AI integration** - OpenAI API with creative implementations
- **Graphics programming** - Off-screen rendering, hardware acceleration
- **System architecture** - Complex integration between different technologies

## 🔗 Related Repositories

- [Frontend (React UI)](https://github.com/yourusername/chess-game-ui)
