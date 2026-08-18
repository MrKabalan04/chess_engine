import "./ChessBoard.css";
import parseFen from "./parseFen";
import { useState, useEffect, useRef } from "react";
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

import CaptureMoveAudio from "../assets/sounds/CaptureMoveAudio.mp3";
import CheckMoveAudio from "../assets/sounds/CheckMoveAudio.mp3";
import IllegalMoveAudio from "../assets/sounds/IllegalMoveAudio.mp3";
import MoveAudio from "../assets/sounds/MoveAudio.mp3";
import PromotionAudio from "../assets/sounds/PromotionAudio.mp3";
import CheckMateAudio from "../assets/sounds/CheckMateAudio.mp3";
import CastlingAudio from "../assets/sounds/CastlingAudio.mp3";

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

// Helper to get piece value for material calculation
function getPieceValue(pieceKey) {
  const type = pieceKey.split('_')[1];
  switch (type) {
    case 'pawn': return 1;
    case 'knight': return 3;
    case 'bishop': return 3;
    case 'rook': return 5;
    case 'queen': return 9;
    default: return 0;
  }
}

const soundMap = {
    move: new Audio(MoveAudio),
    capture: new Audio(CaptureMoveAudio),
    check: new Audio(CheckMoveAudio),
    illegal: new Audio(IllegalMoveAudio),
    castle: new Audio(CastlingAudio),
    checkmate: new Audio(CheckMateAudio),
    promotion: new Audio(PromotionAudio),
}; 

const INITIAL_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

function ChessBoard() {
  const [board, setBoard] = useState(
    parseFen(INITIAL_FEN)
  );
  const [playerSide, setPlayerSide] = useState("white");
  const [gameStarted, setGameStarted] = useState(false);
  const [selectedSquare, setSelectedSquare] = useState(null);
  const [legalMoves, setLegalMoves] = useState([]);
  const [gameStatus, setGameStatus] = useState("playing");
  const [pendingPromotion, setPendingPromotion] = useState(null);
  const [kingInCheck, setKingInCheck] = useState(false);
  const [kingSquare, setKingSquare] = useState(null);
  const [moveHistory, setMoveHistory] = useState([]);
  const [capturedWhite, setCapturedWhite] = useState([]);
  const [capturedBlack, setCapturedBlack] = useState([]);
  // Stack of every ply played: { side: 'white'|'black', san, captured }
  const [moveStack, setMoveStack] = useState([]);
  // Last played move: [[fromRow, fromCol], [toRow, toCol]] in board coords
  const [lastMove, setLastMove] = useState(null);
  const engineTimerRef = useRef(null);

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

  // Fixed move history – no duplicates, proper pairing
  function addMoveToHistory(side, san) {
    setMoveHistory(prev => {
      const newHistory = [...prev];
      if (side === "white") {
        // If last entry already has a white move, push a new move number
        if (newHistory.length === 0 || newHistory[newHistory.length - 1].white !== null) {
          newHistory.push({ white: san, black: null });
        } else {
          // Last entry has white:null, black:? (shouldn't happen, but fill white)
          newHistory[newHistory.length - 1].white = san;
        }
      } else {
        if (newHistory.length === 0) {
          newHistory.push({ white: null, black: san });
        } else {
          const last = newHistory[newHistory.length - 1];
          if (last.black === null) {
            last.black = san;
          } else {
            newHistory.push({ white: null, black: san });
          }
        }
      }
      return newHistory;
    });
  }

  function pushPly(side, san, captured) {
    setMoveStack(prev => [...prev, { side, san, captured: captured || null }]);
  }

  // Rebuild the white/black paired history from the flat ply stack
  function rebuildHistoryFromStack(stack) {
    const history = [];
    for (let i = 0; i < stack.length; ) {
      const entry = { white: null, black: null };
      if (stack[i].side === "white") entry.white = stack[i].san;
      else entry.black = stack[i].san;
      if (i + 1 < stack.length && stack[i + 1].side === "black") {
        entry.black = stack[i + 1].san;
        i += 2;
      } else {
        i += 1;
      }
      history.push(entry);
    }
    return history;
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
      playSound("check");
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

  // Let the engine play its move. boardBefore is the position right before
  // the engine moves (used only to build SAN).
  function engineReply(boardBefore) {
    if (!window.luna) return;
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
      const engineFromRow = 7 - Math.floor(fromSq / 8);
      const engineFromCol = fromSq % 8;
      const engineToRow = 7 - Math.floor(toSq / 8);
      const engineToCol = toSq % 8;
      const engineColor = window.luna.getSideToMove() === 0 ? "white" : "black";
      const currentBoard = parseFen(window.luna.getFen());
      const engineCaptured = currentBoard[engineToRow][engineToCol];
      let engineCapturedKey = null;
      if (engineCaptured) {
        engineCapturedKey = `${engineCaptured.color}_${engineCaptured.type}`;
        if (engineColor === "white") setCapturedWhite(prev => [...prev, engineCapturedKey]);
        else setCapturedBlack(prev => [...prev, engineCapturedKey]);
      }
      const sanEngine = uciToSan(boardBefore || board, engineFromRow, engineFromCol, engineToRow, engineToCol, promotion);
      addMoveToHistory(engineColor, sanEngine);
      pushPly(engineColor, sanEngine, engineCapturedKey);
      const newFen2 = window.luna.makeMove(fromSq, toSq, promotion ? promotion[0] : "");
      setBoard(parseFen(newFen2));
      setLastMove([[engineFromRow, engineFromCol], [engineToRow, engineToCol]]);
      playSound(promotion ? "promotion" : engineCaptured ? "capture" : "move");
      updateCheckStatus();
    }
    const status = window.luna.getGameStatus();
    setGameStatus(status);
    if (status !== "playing") playSound("checkmate");
    engineTimerRef.current = null;
  }

  // Wipe all game state back to the starting position (no engine move)
  function resetBoard() {
    if (!window.luna) return;
    if (engineTimerRef.current) {
      clearTimeout(engineTimerRef.current);
      engineTimerRef.current = null;
    }
    window.luna.initEngine();
    setBoard(parseFen(INITIAL_FEN));
    setSelectedSquare(null);
    setLegalMoves([]);
    setGameStatus("playing");
    setPendingPromotion(null);
    setMoveHistory([]);
    setMoveStack([]);
    setCapturedWhite([]);
    setCapturedBlack([]);
    setKingInCheck(false);
    setKingSquare(null);
    setLastMove(null);
    updateCheckStatus();
  }

  // Start a fresh game as the given side. If the human plays black,
  // the engine (white) makes the opening move.
  function playAs(side) {
    setPlayerSide(side);
    resetBoard();
    setGameStarted(true);
    if (side === "black") {
      engineTimerRef.current = setTimeout(() => engineReply(parseFen(INITIAL_FEN)), 100);
    }
  }

  // New Game returns to side selection – nothing starts until a side is picked
  function resetGame() {
    resetBoard();
    setGameStarted(false);
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
    pushPly(pendingPromotion.color === "white" ? "white" : "black", san, pendingPromotion.captured);
    setLastMove([[fromRow, fromCol], [toRow, toCol]]);

    // Engine response after promotion
    engineTimerRef.current = setTimeout(() => engineReply(board), 100);
  }



function playSound(type) {
    const sound = soundMap[type];
    if (sound) {
        sound.currentTime = 0;
        sound.play();
    }
}

function handleUndo() {
    if (!window.luna) return;
    if (moveStack.length === 0) return;

    // User is picking a promotion piece – cancel the overlay and undo any
    // captured piece that was recorded when the promoting move was clicked
    if (pendingPromotion) {
      if (pendingPromotion.captured) {
        if (pendingPromotion.color === "white") {
          setCapturedWhite(prev => prev.slice(0, -1));
        } else {
          setCapturedBlack(prev => prev.slice(0, -1));
        }
      }
      setPendingPromotion(null);
      setSelectedSquare(null);
      setLegalMoves([]);
      return;
    }

    // Cancel any engine reply that hasn't fired yet
    if (engineTimerRef.current) {
      clearTimeout(engineTimerRef.current);
      engineTimerRef.current = null;
    }

    const stack = [...moveStack];
    const last = stack[stack.length - 1];
    // The engine plays the opposite side. If the engine just replied,
    // take back both plies, otherwise just the player's ply.
    const engineColor = playerSide === "white" ? "black" : "white";
    const pliesToRemove = last.side === engineColor ? Math.min(2, stack.length) : 1;

    for (let i = 0; i < pliesToRemove; i++) {
      window.luna.undoMove();
    }
    const removed = stack.splice(stack.length - pliesToRemove, pliesToRemove);

    // Restore captured pieces (captured lists are append-only)
    const w = [...capturedWhite];
    const b = [...capturedBlack];
    removed.forEach(ply => {
      if (!ply.captured) return;
      if (ply.side === "white") w.pop();
      else b.pop();
    });

    setMoveStack(stack);
    setCapturedWhite(w);
    setCapturedBlack(b);
    setMoveHistory(rebuildHistoryFromStack(stack));
    setBoard(parseFen(window.luna.getFen()));
    setSelectedSquare(null);
    setLegalMoves([]);
    setGameStatus(window.luna.getGameStatus());
    updateCheckStatus();

    // If the undo landed back on the engine's turn (e.g. player is black and
    // took back the only opening move), let the engine move again.
    const humanSide = playerSide === "white" ? 0 : 1;
    if (window.luna.getSideToMove() !== humanSide) {
      const boardBeforeEngine = parseFen(window.luna.getFen());
      engineTimerRef.current = setTimeout(() => engineReply(boardBeforeEngine), 100);
    }
  }

  function handleClick(displayRow, displayCol) {
    if (gameStatus !== "playing") return;

    // The board is rendered flipped for black – map display -> board coords
    const row = boardFlipped ? 7 - displayRow : displayRow;
    const col = boardFlipped ? 7 - displayCol : displayCol;

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
      const capturedPiece = board[row][col];
      let capturedKey = null;

      // Record capture
      if (capturedPiece) {
        capturedKey = `${capturedPiece.color}_${capturedPiece.type}`;
        if (movingPiece.color === 'white') {
          setCapturedWhite(prev => [...prev, capturedKey]);
        } else {
          setCapturedBlack(prev => [...prev, capturedKey]);
        }
      }

      // Check for pawn promotion
      if (movingPiece.type === "pawn" && (row === 0 || row === 7)) {
        setPendingPromotion({ from, to, color: movingPiece.color, captured: capturedKey });
        setSelectedSquare(null);
        setLegalMoves([]);
        return;
      }

      // Normal move – add SAN before changing board
      const san = uciToSan(board, fromRowSel, fromColSel, row, col, null);
      addMoveToHistory(movingPiece.color === "white" ? "white" : "black", san);
      pushPly(movingPiece.color === "white" ? "white" : "black", san, capturedKey);

      const newFen = window.luna.makeMove(from, to, "");
      setBoard(parseFen(newFen));
      setLastMove([[fromRowSel, fromColSel], [row, col]]);
      const isCastle = movingPiece.type === "king" && Math.abs(col - fromColSel) === 2;
      console.log("about to play sound");
      playSound(isCastle ? "castle" : capturedPiece ? "capture" : "move");
      setSelectedSquare(null);
      setLegalMoves([]);
      updateCheckStatus();

      // Engine response
      engineTimerRef.current = setTimeout(() => engineReply(board), 100);
    } else {
      playSound("illegal");
      setSelectedSquare(null);
      setLegalMoves([]);
    }
  }

  // Calculate material advantage
  const whiteMaterial = capturedWhite.reduce((sum, key) => sum + getPieceValue(key), 0);
  const blackMaterial = capturedBlack.reduce((sum, key) => sum + getPieceValue(key), 0);
  const materialAdvantage = whiteMaterial - blackMaterial;
  const advantageText = materialAdvantage > 0 ? `+${materialAdvantage}` : materialAdvantage < 0 ? `${materialAdvantage}` : '';

  // Board is flipped when the player sits on the black side
  const boardFlipped = playerSide === "black";

  return (
    <div className="chess-dashboard">
      {/* Left: Chess board */}
      <div className="board-container">
        <div className="board-wrapper">
          <div className="board">
          {Array.from({ length: 8 }, (_, dr) =>
            Array.from({ length: 8 }, (_, dc) => {
              // Render in display order; when flipped (black player), the
              // player's pieces sit at the bottom.
              const r = boardFlipped ? 7 - dr : dr;
              const c = boardFlipped ? 7 - dc : dc;
              const sq = board[r][c];
              return (
              <div
                key={`${dr}-${dc}`}
                className={`square ${(dr + dc) % 2 === 0 ? "light" : "dark"} ${
                  lastMove && (
                    (lastMove[0][0] === r && lastMove[0][1] === c) ||
                    (lastMove[1][0] === r && lastMove[1][1] === c)
                  ) ? "last-move" : ""
                } ${
                  selectedSquare && selectedSquare[0] === r && selectedSquare[1] === c ? "selected" : ""
                } ${
                  legalMoves.some((m) => m[0] === r && m[1] === c) ? "legal-move" : ""
                } ${
                  kingInCheck && kingSquare && kingSquare[0] === r && kingSquare[1] === c ? "in-check" : ""
                }`}
                onClick={() => handleClick(dr, dc)}
              >
                {dc === 0 && (
                  <span className="coord coord-rank">
                    {boardFlipped ? dr + 1 : 8 - dr}
                  </span>
                )}
                {dr === 7 && (
                  <span className="coord coord-file">
                    {String.fromCharCode(boardFlipped ? 104 - dc : 97 + dc)}
                  </span>
                )}
                {sq && (
                  <img
                    src={pieces[`${sq.color}_${sq.type}`]}
                    className="piece"
                    draggable="false"
                    alt=""
                  />
                )}
              </div>
              );
            })
          )}
          </div>

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
      </div>

      {/* Right: Dashboard */}
      <div className="dashboard-panel">
        {/* Top bar: status + new game */}
        <div className="game-status-bar">
          <div className="game-status-message">
            {gameStatus === "playing"
              ? (kingInCheck ? "CHECK!" : "Playing")
              : gameStatus === "checkmate"
              ? "Checkmate!"
              : gameStatus === "stalemate"
              ? "Stalemate!"
              : "Draw!"}
          </div>
          <button className="new-game-btn" onClick={resetGame}>New Game</button>
        </div>

        {/* Captured pieces section – with icons */}
        <div className="captured-section">
          <div className="captured-row">
            <span className="captured-label">White</span>
            <div className="captured-pieces">
              {capturedWhite.map((key, idx) => (
                <img key={idx} src={pieces[key]} className="captured-piece" alt="" />
              ))}
            </div>
            <span className="material-score">{whiteMaterial > 0 ? `+${whiteMaterial}` : ''}</span>
          </div>
          <div className="captured-row">
            <span className="captured-label">Black</span>
            <div className="captured-pieces">
              {capturedBlack.map((key, idx) => (
                <img key={idx} src={pieces[key]} className="captured-piece" alt="" />
              ))}
            </div>
            <span className="material-score">{blackMaterial > 0 ? `+${blackMaterial}` : ''}</span>
          </div>
          {advantageText && <div className="advantage-indicator">{advantageText}</div>}
        </div>

        {/* Game actions – choose side before starting, undo during play */}
        <div className="game-actions">
          {!gameStarted && moveStack.length === 0 && (
            <div className="side-picker" title="Play as">
              <button
                className={`side-btn ${playerSide === "white" ? "active" : ""}`}
                onClick={() => playAs("white")}
              >
                White
              </button>
              <button
                className={`side-btn ${playerSide === "black" ? "active" : ""}`}
                onClick={() => playAs("black")}
              >
                Black
              </button>
            </div>
          )}
          <button
            className="control-btn undo-btn"
            onClick={handleUndo}
            disabled={moveStack.length === 0}
            title="Undo move"
          >
            <span className="control-btn-icon" aria-hidden="true">
              <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
                <path d="M3 7v6h6" />
                <path d="M21 17a9 9 0 0 0-15-6.7L3 13" />
              </svg>
            </span>
            <span>Undo</span>
          </button>
        </div>

        {/* Move history – clean table */}
        <div className="move-history">
          <h3>Move History</h3>
          <div className="move-table">
            <table>
              <thead>
                <tr>
                  <th>#</th>
                  <th>White</th>
                  <th>Black</th>
                </tr>
              </thead>
              <tbody>
                {moveHistory.map((move, idx) => (
                  <tr
                    key={idx}
                    className={idx === moveHistory.length - 1 ? "last-move-row" : ""}
                  >
                    <td>{idx + 1}</td>
                    <td>{move.white || '—'}</td>
                    <td>{move.black || '—'}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  );
}

export default ChessBoard;