import "./ChessBoard.css";
import parseFen from "./parseFen";
import { useState, useEffect } from "react";
import { uciToSan } from "./uciToSan";
import MoveHistory from "./MoveHistory/MoveHistory.jsx";

import WhitePawn from "../assets/pieces/WhitePawn.svg";
import WhiteRook from "../assets/pieces/WhiteRook.svg";
import WhiteKnight from "../assets/pieces/WhiteKnight.svg";
import WhiteBishop from "../assets/pieces/WhiteBishop.svg";
import WhiteQueen from "../assets/pieces/WhiteQueen.svg";
import WhiteKing from "../assets/pieces/WhiteKing.svg";

import BlackPawn from "../assets/pieces/BlackPawn.svg";
import BlackRook from "../assets/pieces/BlackRook.svg";
import BlackKnight from "../assets/pieces/BlackKnight.svg";
import BlackBishop from "../assets/pieces/BlackBishop.svg";
import BlackQueen from "../assets/pieces/BlackQueen.svg";
import BlackKing from "../assets/pieces/BlackKing.svg";

const pieces = {
  white_pawn: WhitePawn,
  white_rook: WhiteRook,
  white_knight: WhiteKnight,
  white_bishop: WhiteBishop,
  white_queen: WhiteQueen,
  white_king: WhiteKing,
  black_pawn: BlackPawn,
  black_rook: BlackRook,
  black_knight: BlackKnight,
  black_bishop: BlackBishop,
  black_queen: BlackQueen,
  black_king: BlackKing,
};

function ChessBoard() {
  const [board, setBoard] = useState(
    parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
  );
  const [selectedSquare, setSelectedSquare] = useState(null);
  const [legalMoves, setLegalMoves] = useState([]);
  const [gameStatus, setGameStatus] = useState("playing");
  const [pendingPromotion, setPendingPromotion] = useState(null);
  const [kingInCheck, setKingInCheck] = useState(false);
  const [kingSquare, setKingSquare] = useState(null);
  const [moveHistory, setMoveHistory] = useState([]);

  useEffect(() => {
    const interval = setInterval(() => {
      if (window.LunaEngine) {
        window.LunaEngine().then(engine => {
          window.luna = engine;
          window.luna.initEngine();
          console.log("Luna engine ready");
          updateCheckStatus();
        });
        clearInterval(interval);
      }
    }, 100);
  }, []);

  function addMoveToHistory(side, san) {
    setMoveHistory(prev => {
      const last = prev[prev.length - 1];
      if (side === "white") {
        if (last && !last.white) {
          last.white = san;
          return [...prev.slice(0, -1), last];
        }
        return [...prev, { white: san, black: null }];
      } else {
        if (last && !last.black) {
          last.black = san;
          return [...prev.slice(0, -1), last];
        }
        return [...prev, { white: null, black: san }];
      }
    });
  }

  function updateCheckStatus() {
    if (!window.luna) return;
    const inCheck = window.luna.isInCheck();
    if (inCheck) {
      const ks = window.luna.getKingSquare();
      const row = 7 - Math.floor(ks / 8);
      const col = ks % 8;
      setKingInCheck(true);
      setKingSquare([row, col]);
    } else {
      setKingInCheck(false);
      setKingSquare(null);
    }
  }

  function algebraicToSquare(str) {
    const file = str.charCodeAt(0) - 'a'.charCodeAt(0);
    const rank = parseInt(str[1]) - 1;
    return rank * 8 + file;
  }

  function resetGame() {
    window.luna.initEngine();
    setBoard(parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));
    setSelectedSquare(null);
    setLegalMoves([]);
    setGameStatus("playing");
    setPendingPromotion(null);
    setMoveHistory([]);
    updateCheckStatus();
  }

  function handlePromotion(type) {
    if (!pendingPromotion) return;
    const promoChar = type === "knight" ? "n" : type[0];
    const newFen = window.luna.makeMove(pendingPromotion.from, pendingPromotion.to, promoChar);
    setBoard(parseFen(newFen));
    setPendingPromotion(null);
    setSelectedSquare(null);
    setLegalMoves([]);
    updateCheckStatus();

    // Add promotion move to history (human side)
    const fromRow = 7 - Math.floor(pendingPromotion.from / 8);
    const fromCol = pendingPromotion.from % 8;
    const toRow = 7 - Math.floor(pendingPromotion.to / 8);
    const toCol = pendingPromotion.to % 8;
    const san = uciToSan(board, fromRow, fromCol, toRow, toCol, type);
    addMoveToHistory(pendingPromotion.color === "white" ? "white" : "black", san);

    // Engine response
    setTimeout(() => {
      if (window.luna) {
        const bestMove = window.luna.getBestMove();
        if (bestMove && bestMove.length >= 4) {
          const fromSq = algebraicToSquare(bestMove.slice(0, 2));
          const toSq = algebraicToSquare(bestMove.slice(2, 4));
          let promotion = null;
          if (bestMove.length > 4) {
            const promoLetter = bestMove[4];
            if (promoLetter === 'q') promotion = 'queen';
            else if (promoLetter === 'r') promotion = 'rook';
            else if (promoLetter === 'b') promotion = 'bishop';
            else if (promoLetter === 'n') promotion = 'knight';
          }
          // Get board BEFORE engine move (current board)
          const fromRowEngine = 7 - Math.floor(fromSq / 8);
          const fromColEngine = fromSq % 8;
          const toRowEngine = 7 - Math.floor(toSq / 8);
          const toColEngine = toSq % 8;
          const sanEngine = uciToSan(board, fromRowEngine, fromColEngine, toRowEngine, toColEngine, promotion);
          addMoveToHistory("black", sanEngine);
          const newFen2 = window.luna.makeMove(fromSq, toSq, promotion ? promotion[0] : "");
          setBoard(parseFen(newFen2));
          updateCheckStatus();
        }
        const status = window.luna.getGameStatus();
        setGameStatus(status);
      }
    }, 100);
  }

  function handleClick(row, col) {
    // If game is over, don't allow any moves
    if (gameStatus !== "playing") return;

    const piece = board[row][col];

    if (!piece && !selectedSquare) return;

    if (piece && !selectedSquare) {
      setSelectedSquare([row, col]);
      const square = (7 - row) * 8 + col;
      const movesStr = window.luna?.getLegalMoves(square);
      if (movesStr && movesStr !== "") {
        const moves = movesStr.split(",").map(s => {
          const sq = parseInt(s);
          return [7 - Math.floor(sq / 8), sq % 8];
        });
        setLegalMoves(moves);
      } else {
        setLegalMoves([]);
      }
      return;
    }

    const isLegalMove = legalMoves.some(m => m[0] === row && m[1] === col);
    if (isLegalMove) {
      const fromRowSel = selectedSquare[0];
      const fromColSel = selectedSquare[1];
      const from = (7 - fromRowSel) * 8 + fromColSel;
      const to = (7 - row) * 8 + col;
      const movingPiece = board[fromRowSel][fromColSel];

      // Check for pawn promotion
      if (movingPiece.type === "pawn" && (row === 0 || row === 7)) {
        setPendingPromotion({ from, to, color: movingPiece.color });
        setSelectedSquare(null);
        setLegalMoves([]);
        return;
      }

      // Normal move – add SAN before changing board
      const san = uciToSan(board, fromRowSel, fromColSel, row, col, null);
      addMoveToHistory(movingPiece.color === "white" ? "white" : "black", san);

      const newFen = window.luna.makeMove(from, to, "");
      setBoard(parseFen(newFen));
      setSelectedSquare(null);
      setLegalMoves([]);
      updateCheckStatus();

      // Engine response
      setTimeout(() => {
        if (window.luna) {
          const bestMove = window.luna.getBestMove();
          if (bestMove && bestMove.length >= 4) {
            const fromSq = algebraicToSquare(bestMove.slice(0, 2));
            const toSq = algebraicToSquare(bestMove.slice(2, 4));
            let promotion = null;
            if (bestMove.length > 4) {
              const promoLetter = bestMove[4];
              if (promoLetter === 'q') promotion = 'queen';
              else if (promoLetter === 'r') promotion = 'rook';
              else if (promoLetter === 'b') promotion = 'bishop';
              else if (promoLetter === 'n') promotion = 'knight';
            }
            const fromRowEngine = 7 - Math.floor(fromSq / 8);
            const fromColEngine = fromSq % 8;
            const toRowEngine = 7 - Math.floor(toSq / 8);
            const toColEngine = toSq % 8;
            const sanEngine = uciToSan(board, fromRowEngine, fromColEngine, toRowEngine, toColEngine, promotion);
            addMoveToHistory("black", sanEngine);
            const newFen2 = window.luna.makeMove(fromSq, toSq, promotion ? promotion[0] : "");
            setBoard(parseFen(newFen2));
            updateCheckStatus();
          }
          const status = window.luna.getGameStatus();
          setGameStatus(status);
        }
      }, 100);
    } else {
      setSelectedSquare(null);
      setLegalMoves([]);
    }
  }

  // Helper to get game over message
  function getGameOverMessage() {
    if (gameStatus === "checkmate") return "Checkmate!";
    if (gameStatus === "stalemate") return "Stalemate!";
    if (gameStatus === "draw") return "Draw!";
    return null;
  }

  const gameOverMessage = getGameOverMessage();

  return (
  <div className="chess-dashboard">
    {/* Left side: Board + overlays */}
    <div className="board-container">
      <div className="board">
        {board.map((row, r) =>
          row.map((sq, c) => (
            <div
              key={`${r}-${c}`}
              className={`square ${(r + c) % 2 === 0 ? "light" : "dark"} ${
                selectedSquare && selectedSquare[0] === r && selectedSquare[1] === c ? "selected" : ""
              } ${
                legalMoves.some((m) => m[0] === r && m[1] === c) ? "legal-move" : ""
              } ${
                kingInCheck && kingSquare && kingSquare[0] === r && kingSquare[1] === c ? "in-check" : ""
              }`}
              onClick={() => handleClick(r, c)}
            >
              {sq && (
                <img
                  src={pieces[`${sq.color}_${sq.type}`]}
                  className="piece"
                  draggable="false"
                  alt=""
                />
              )}
            </div>
          ))
        )}
      </div>

      {/* Promotion overlay (centered on board) */}
      {pendingPromotion && (
        <div className="promotion-overlay">
          <div className="promotion-box">
            <p>Choose a piece</p>
            <div className="promotion-pieces">
              {["queen", "rook", "bishop", "knight"].map(type => (
                <img
                  key={type}
                  src={pieces[`${pendingPromotion.color}_${type}`]}
                  onClick={() => handlePromotion(type)}
                  className="promotion-piece"
                  alt={type}
                />
              ))}
            </div>
          </div>
        </div>
      )}
    </div>

    {/* Right side: Dashboard */}
    <div className="dashboard-panel">
      {/* Game status bar */}
      <div className="game-status-bar">
        <span className="game-status-message">
          {gameStatus === "playing"
            ? (kingInCheck ? "CHECK!" : "Playing")
            : gameStatus === "checkmate"
            ? "Checkmate!"
            : gameStatus === "stalemate"
            ? "Stalemate!"
            : "Draw!"}
        </span>
        <button className="new-game-btn" onClick={resetGame}>
          New Game
        </button>
      </div>

      {/* Move history */}
      <MoveHistory moves={moveHistory} />
    </div>
  </div>
  );
}

export default ChessBoard;