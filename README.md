# ♟️ Chess Engine (C++ Bitboard Implementation)

A modular chess engine written in C++ using **bitboards (uint64_t)** for board representation.  
This project is being built step-by-step with a focus on clean architecture, correctness, and future scalability toward a full chess engine and web-based application.

---

## 🎯 Project Goal

The goal of this project is to build a complete chess engine from scratch, including:

- Efficient board representation using bitboards
- Move generation and legal move validation
- AI decision-making using minimax / alpha-beta pruning
- Position evaluation system
- Future integration into a web-based chess application using WebAssembly

This project is primarily a learning and portfolio project aimed at improving systems design and algorithmic thinking for real-world software development roles.

---

## ⚙️ Current Features

- Bitboard-based board representation
- Initialization of standard chess starting position
- Separate bitboards for each piece type and color
- Combined occupancy tracking (white, black, and full board)
- Debug board visualization using console output

---

## 🧱 Architecture Overview

The project is being designed with modular separation:

- **Board Module**
  - Stores game state using bitboards
  - Handles piece placement and updates

- **Move System (Planned)**
  - Move representation (from → to, special moves)
  - Move generation for all pieces

- **Game Controller (Planned)**
  - Turn management (white/black)
  - Game flow control

- **Engine (Planned)**
  - Minimax / alpha-beta search
  - Position evaluation function

- **Web Layer (Future)**
  - JavaScript UI for chessboard rendering
  - WebAssembly integration of C++ engine
  - Browser-based gameplay and deployment

---

## 🚀 Planned Features

- Legal move generation for all pieces
- Check / checkmate detection
- Castling, en passant, promotion
- Alpha-beta pruning search algorithm
- Position evaluation heuristics
- Opening improvements
- WebAssembly compilation for browser execution
- JavaScript-based UI with interactive chessboard
- Deployment using Netlify

---

## 🧠 Tech Stack

- C++
- Bitboards (`uint64_t`)
- Console-based debugging tools
- Future: WebAssembly + JavaScript frontend
- Future deployment: Netlify

---

## 📌 Notes

- This project is currently in early development (board representation phase).
- Focus is on building a strong foundation before implementing engine logic.
- Debugging tools are used to validate board state correctness.

---

## 🧪 How to Run

Compile using a standard C++ compiler:

```bash
g++ main.cpp -o chess
./chess