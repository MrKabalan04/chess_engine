# ♟️ Chess Engine (C++ Bitboard Implementation) — *Luna*

A modular chess engine written in C++ using bitboards (`uint64_t`) for board representation.

The project is built step-by-step with a focus on correctness, clean architecture, and long-term scalability into a fully playable chess engine.

It has now evolved from a terminal-based engine into a browser-ready project with a React frontend and future WebAssembly integration.

---

## 🎯 Project Goal

The goal of this project is to build a complete chess engine from scratch, including:

- Efficient board representation using bitboards
- Full legal move generation and validation
- Game rules (check, checkmate, castling, en passant, promotion)
- AI decision-making using minimax / alpha-beta pruning
- Position evaluation system
- Web-based integration using WebAssembly

The long-term goal is to allow users to play against the engine directly in the browser.

---

## 🌐 Current Status

The project has reached a new stage:

### ✔️ Engine (C++)
- Bitboard-based board representation
- Standard chess starting position initialization
- Separate bitboards for each piece type and color
- Combined occupancy tracking (white / black / full board)
- Debug visualization in console
- Legal move generation implemented
- Core engine logic is stable

### ✔️ Web Interface (React)
- Chessboard UI built using React
- 8x8 dynamic rendering system
- Theme-ready board styling
- Component-based architecture prepared for engine integration

---

## 🧱 Architecture Overview

The project is structured into three main layers:

### **1. Engine Layer (C++)**
- Bitboard representation
- Move generation & validation
- Search (minimax / alpha-beta pruning)
- Evaluation system

### **2. Web Layer (React)**
- Chessboard rendering
- User interaction layer
- Game state visualization
- Future integration with engine output

### **3. Bridge Layer (WebAssembly - Planned)**
- Compile C++ engine to WASM
- Connect engine logic to browser
- Enable real-time gameplay in the UI

---

## 🚀 Current Direction

The project is now focused on:

- Connecting the C++ engine to the React frontend
- Integrating WebAssembly for browser execution
- Turning the engine into an interactive playable experience
- Improving UI/UX for real gameplay interaction

---

## ⚙️ Tech Stack

- C++ (core engine)
- Bitboards (`uint64_t`)
- React (frontend UI)
- JavaScript (UI logic layer)
- WebAssembly (planned engine integration)

---

## 📌 Notes

This project started as a chess engine learning exercise and is now evolving into a full-stack chess application.

Current focus is shifting from:
> "making the engine work"

to:
> "making the engine playable in the browser"

Still building. Still learning.