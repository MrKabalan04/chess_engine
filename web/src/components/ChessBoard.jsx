import "./ChessBoard.css";
import parseFen from "./parseFen";
import { useState, useEffect, useRef } from "react";
import { uciToSan } from "./uciToSan";

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

const soundFiles = {
  move: MoveAudio,
  capture: CaptureMoveAudio,
  check: CheckMoveAudio,
  illegal: IllegalMoveAudio,
  castle: CastlingAudio,
  checkmate: CheckMateAudio,
  promotion: PromotionAudio,
};

// Low-latency sound engine: all clips are fetched and decoded once into an
// AudioContext, so playback starts instantly instead of paying the
// HTMLAudioElement startup cost on every move. The context is created on the
// first user gesture (browser autoplay policy) – not at page load.
let audioCtx = null;
const soundBuffers = {};
const fallbackEls = {};
let soundsPreloading = false;

function ensureAudioCtx() {
  if (!audioCtx) {
    try {
      audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    } catch {
      return null;
    }
  }
  if (audioCtx.state === "suspended") audioCtx.resume();
  return audioCtx;
}

function preloadSounds() {
  if (soundsPreloading || !audioCtx) return;
  soundsPreloading = true;
  Object.entries(soundFiles).forEach(([type, src]) => {
    fetch(src)
      .then(r => r.arrayBuffer())
      .then(ab => audioCtx.decodeAudioData(ab))
      .then(buf => { soundBuffers[type] = buf; })
      .catch(() => {
        if (!fallbackEls[type]) fallbackEls[type] = new Audio(src);
      });
  });
}

if (typeof window !== "undefined") {
  const kick = () => {
    ensureAudioCtx();
    preloadSounds();
    window.removeEventListener("pointerdown", kick);
    window.removeEventListener("keydown", kick);
  };
  window.addEventListener("pointerdown", kick);
  window.addEventListener("keydown", kick);
}

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
  const engineBusyRef = useRef(false);
  // Mirrors playerSide for code running in timeouts/closures, which would
  // otherwise capture a stale value from the render that scheduled them.
  const playerSideRef = useRef("white");
  const moveListRef = useRef(null);
  const [copied, setCopied] = useState(false);

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

  // Keep the latest move in view as the game progresses
  useEffect(() => {
    if (moveListRef.current) {
      moveListRef.current.scrollTop = moveListRef.current.scrollHeight;
    }
  }, [moveHistory]);

  // Build a PGN from the paired move history
  function buildPgn() {
    let result = "*";
    if (gameStatus === "checkmate") {
      const last = moveStack[moveStack.length - 1];
      result = last && last.side === "white" ? "1-0" : "0-1";
    } else if (gameStatus === "stalemate" || gameStatus === "draw") {
      result = "1/2-1/2";
    }
    let pgn = `[Event "Luna vs Player"]\n[Site "Web"]\n[Result "${result}"]\n\n`;
    let line = "";
    moveHistory.forEach((m, i) => {
      let txt = `${i + 1}.`;
      if (m.white) txt += ` ${m.white}`;
      if (m.black) txt += ` ${m.black}`;
      line += txt + "  ";
      if (line.length > 60) {
        pgn += line.trimEnd() + "\n";
        line = "";
      }
    });
    pgn += (line.trimEnd() + " " + result).trim() + "\n";
    return pgn;
  }

  async function copyPgn() {
    if (moveHistory.length === 0) return;
    const pgn = buildPgn();
    try {
      await navigator.clipboard.writeText(pgn);
    } catch (err) {
      const ta = document.createElement("textarea");
      ta.value = pgn;
      ta.style.position = "fixed";
      ta.style.opacity = "0";
      document.body.appendChild(ta);
      ta.select();
      document.execCommand("copy");
      document.body.removeChild(ta);
    }
    setCopied(true);
    setTimeout(() => setCopied(false), 1600);
  }

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
    setMoveStack(prev => {
      // Guard against duplicate recording (e.g. an engine reply firing
      // twice): an identical consecutive ply is always a true duplicate.
      const last = prev[prev.length - 1];
      if (last && last.side === side && last.san === san) return prev;
      return [...prev, { side, san, captured: captured || null }];
    });
  }

  // Schedule exactly one engine reply; if a reply is already pending (or
  // in flight), ignore the new request so plies are never recorded twice.
  function scheduleEngineReply(boardBefore, tries = 0) {
    if (engineTimerRef.current || engineBusyRef.current) return;
    engineTimerRef.current = setTimeout(() => {
      if (!window.luna) {
        // Engine still loading – retry instead of silently dropping the move
        engineTimerRef.current = null;
        if (tries < 100) scheduleEngineReply(boardBefore, tries + 1);
        return;
      }
      engineReply(boardBefore);
    }, 100);
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
    if (engineBusyRef.current) return;   // already producing a reply
    engineBusyRef.current = true;
    try {
      const engineSide = playerSideRef.current === "white" ? 1 : 0;
      if (window.luna.getSideToMove() !== engineSide) return; // not our turn
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
        playSound(promotion ? "promotion" : engineCaptured ? "capture" : "move");
        const newFen2 = window.luna.makeMove(fromSq, toSq, promotion ? promotion[0] : "");
        setBoard(parseFen(newFen2));
        setLastMove([[engineFromRow, engineFromCol], [engineToRow, engineToCol]]);
        updateCheckStatus();
      }
      const status = window.luna.getGameStatus();
      setGameStatus(status);
      if (status !== "playing") playSound("checkmate");
    } finally {
      engineTimerRef.current = null;
      engineBusyRef.current = false;
    }
  }

  // Wipe all game state back to the starting position (no engine move)
  function resetBoard() {
    if (!window.luna) return;
    if (engineTimerRef.current) {
      clearTimeout(engineTimerRef.current);
      engineTimerRef.current = null;
    }
    engineBusyRef.current = false;
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
    playerSideRef.current = side;   // update ref first so scheduled replies see the new side
    setPlayerSide(side);
    resetBoard();
    setGameStarted(true);
    if (side === "black") {
      scheduleEngineReply(parseFen(INITIAL_FEN));
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
    scheduleEngineReply(board);
  }



function playSound(type) {
  ensureAudioCtx();
  const buf = soundBuffers[type];
  if (buf && audioCtx && audioCtx.state === "running") {
    const node = audioCtx.createBufferSource();
    node.buffer = buf;
    node.connect(audioCtx.destination);
    node.start(0);
    return;
  }
  const el = fallbackEls[type] || (fallbackEls[type] = new Audio(soundFiles[type]));
  el.currentTime = 0;
  el.play().catch(() => {});
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
    engineBusyRef.current = false;

    const stack = [...moveStack];
    const last = stack[stack.length - 1];
    // The engine plays the opposite side. If the engine just replied,
    // take back both plies, otherwise just the player's ply.
    const engineColor = playerSideRef.current === "white" ? "black" : "white";
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
    const humanSide = playerSideRef.current === "white" ? 0 : 1;
    if (window.luna.getSideToMove() !== humanSide) {
      const boardBeforeEngine = parseFen(window.luna.getFen());
      scheduleEngineReply(boardBeforeEngine);
    }
  }

  function handleClick(displayRow, displayCol) {
    if (gameStatus !== "playing") return;
    if (engineTimerRef.current || engineBusyRef.current) return; // engine is moving

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

      // Play immediately – before the synchronous engine work – so the
      // sound lands exactly on the click instead of trailing it.
      const isCastle = movingPiece.type === "king" && Math.abs(col - fromColSel) === 2;
      playSound(isCastle ? "castle" : capturedPiece ? "capture" : "move");

      const newFen = window.luna.makeMove(from, to, "");
      setBoard(parseFen(newFen));
      setLastMove([[fromRowSel, fromColSel], [row, col]]);
      setSelectedSquare(null);
      setLegalMoves([]);
      updateCheckStatus();

      // Engine response
      scheduleEngineReply(board);
    } else {
      playSound("illegal");
      setSelectedSquare(null);
      setLegalMoves([]);
    }
  }

  // Calculate material advantage
  const whiteMaterial = capturedWhite.reduce((sum, key) => sum + getPieceValue(key), 0);
  const blackMaterial = capturedBlack.reduce((sum, key) => sum + getPieceValue(key), 0);

  // Board is flipped when the player sits on the black side
  const boardFlipped = playerSide === "black";

  // ---- Dashboard model -------------------------------------------------
  // capturedWhite/capturedBlack are appended when a WHITE/BLACK piece makes
  // a capture, i.e. they are that side's TROPHIES. Badge goes on whichever
  // side has captured more value.
  const engineColor = playerSide === "white" ? "black" : "white";
  const trophies = { white: capturedWhite, black: capturedBlack };
  const takenValue = { white: whiteMaterial, black: blackMaterial };
  const materialDiff = takenValue.white - takenValue.black;
  const badgeFor = (color) =>
    color === "white"
      ? materialDiff > 0 ? `+${materialDiff}` : ""
      : materialDiff < 0 ? `+${-materialDiff}` : "";

  const yourTurn =
    gameStatus === "playing" &&
    ((moveStack.length % 2 === 0) === (playerSide === "white"));
  const gameNotStarted = !gameStarted && moveStack.length === 0;
  const statusText = gameNotStarted
    ? "Choose your side"
    : gameStatus === "checkmate"
    ? "Checkmate"
    : gameStatus === "stalemate"
    ? "Stalemate"
    : gameStatus === "draw"
    ? "Draw"
    : kingInCheck
    ? "Check!"
    : yourTurn
    ? "Your move"
    : "Luna is thinking";

  // Render a SAN move with its piece icon in front (e.g. [N]f3, [R]xe8+,
  // [K]e1 for castling). Pawn moves get the pawn glyph.
  function renderSan(san, color) {
    if (!san || san === "—") return san;
    const pieceType =
      san.startsWith("O-O") ? "king"
      : { N: "knight", B: "bishop", R: "rook", Q: "queen", K: "king" }[san[0]];
    return (
      <>
        <img className="san-piece" src={pieces[`${color}_${pieceType || "pawn"}`]} alt="" />
        {pieceType ? san.slice(1) : san}
      </>
    );
  }

  function PlayerCard({ color, name, className = "" }) {
    return (
      <div className={`player-card ${className}`}>
        <img className="player-icon" src={pieces[`${color}_king`]} alt="" />
        <div className="player-meta">
          <span className="player-name">
            {name}
            <span className={`player-side ${color}`}>
              {color === "white" ? "White" : "Black"}
            </span>
          </span>
          <div className="player-trophies">
            {trophies[color].map((key, i) => (
              <img key={i} src={pieces[key]} alt="" />
            ))}
            {badgeFor(color) && (
              <span className="trophies-badge">{badgeFor(color)}</span>
            )}
          </div>
        </div>
      </div>
    );
  }

  // Shared action buttons – rendered under the board on mobile and at the
  // bottom of the panel on desktop.
  function gameActions() {
    return gameNotStarted ? (
      <>
        <button className="side-option" onClick={() => playAs("white")}>
          <img src={WhiteKing} alt="" />
          <span>Play White</span>
        </button>
        <button className="side-option" onClick={() => playAs("black")}>
          <img src={BlackKing} alt="" />
          <span>Play Black</span>
        </button>
      </>
    ) : (
      <>
        <button className="ghost-btn" onClick={handleUndo} disabled={moveStack.length === 0}>
          Undo
        </button>
        <button className="ghost-btn accent" onClick={resetGame}>
          New Game
        </button>
      </>
    );
  }

  return (
    <div className="chess-dashboard">
      {/* Mobile: engine card sits on top of the board */}
      <PlayerCard color={engineColor} name="Luna" className="player-strip mobile-only" />

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

      {/* Mobile: your card sits under the board, right above the actions */}
      <PlayerCard color={playerSide} name="You" className="player-strip mobile-only" />

      {/* Mobile-only action row: sits right under the board */}
      <div className="board-actions">{gameActions()}</div>

      {/* Right: Dashboard – quiet player-card ledger */}
      <div className="dashboard-panel">
        <PlayerCard color={engineColor} name="Luna" className="desktop-only" />

        <div className="history-block">
          <div className="history-top">
            <span
              className={`status-dot ${
                gameStatus !== "playing" ? "ended" : kingInCheck ? "check" : ""
              }`}
            />
            <span className="status-text">{statusText}</span>
            <button
              className="ghost-btn small"
              onClick={copyPgn}
              disabled={moveHistory.length === 0}
              title="Copy the game moves (PGN) to your clipboard"
            >
              {copied ? "Copied" : "PGN"}
            </button>
          </div>
          <div className="move-ledger" ref={moveListRef}>
            {moveHistory.map((m, i) => (
              <div
                key={i}
                className={`ledger-row ${i === moveHistory.length - 1 ? "current" : ""}`}
              >
                <span className="ledger-num">{i + 1}</span>
                <span className="ledger-san">{renderSan(m.white, "white")}</span>
                <span className="ledger-san">{renderSan(m.black, "black")}</span>
              </div>
            ))}
          </div>
        </div>

        <PlayerCard color={playerSide} name="You" className="desktop-only" />

        <div className="panel-actions">{gameActions()}</div>
      </div>
    </div>
  );
}

export default ChessBoard;