// utils/uciToSan.js

const pieceSymbol = {
  pawn: '',
  knight: 'N',
  bishop: 'B',
  rook: 'R',
  queen: 'Q',
  king: 'K'
};

// Helper: deep copy board
function copyBoard(board) {
  return board.map(row => row.map(cell => cell ? { ...cell } : null));
}

// Helper: get opponent color
function getOpponent(color) {
  return color === 'white' ? 'black' : 'white';
}

// Helper: find king position
function findKing(board, color) {
  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      const piece = board[r][c];
      if (piece && piece.type === 'king' && piece.color === color) {
        return [r, c];
      }
    }
  }
  return null;
}

// Check if a square is attacked by a specific side (simple ray casting)
function isSquareAttacked(board, row, col, attackerColor) {
  // Knight attacks
  const knightOffsets = [[-2,-1],[-2,1],[-1,-2],[-1,2],[1,-2],[1,2],[2,-1],[2,1]];
  for (const [dr, dc] of knightOffsets) {
    const r = row + dr, c = col + dc;
    if (r >= 0 && r < 8 && c >= 0 && c < 8) {
      const piece = board[r][c];
      if (piece && piece.color === attackerColor && piece.type === 'knight') return true;
    }
  }

  // King attacks (adjacent)
  for (let dr = -1; dr <= 1; dr++) {
    for (let dc = -1; dc <= 1; dc++) {
      if (dr === 0 && dc === 0) continue;
      const r = row + dr, c = col + dc;
      if (r >= 0 && r < 8 && c >= 0 && c < 8) {
        const piece = board[r][c];
        if (piece && piece.color === attackerColor && piece.type === 'king') return true;
      }
    }
  }

  // Pawn attacks
  const pawnDirs = attackerColor === 'white' ? [[-1,-1],[-1,1]] : [[1,-1],[1,1]];
  for (const [dr, dc] of pawnDirs) {
    const r = row + dr, c = col + dc;
    if (r >= 0 && r < 8 && c >= 0 && c < 8) {
      const piece = board[r][c];
      if (piece && piece.color === attackerColor && piece.type === 'pawn') return true;
    }
  }

  // Rook/Queen (horizontal & vertical)
  const rookDirs = [[0,1],[0,-1],[1,0],[-1,0]];
  for (const [dr, dc] of rookDirs) {
    let r = row + dr, c = col + dc;
    while (r >= 0 && r < 8 && c >= 0 && c < 8) {
      const piece = board[r][c];
      if (piece) {
        if (piece.color === attackerColor && (piece.type === 'rook' || piece.type === 'queen')) return true;
        break;
      }
      r += dr;
      c += dc;
    }
  }

  // Bishop/Queen (diagonals)
  const bishopDirs = [[1,1],[1,-1],[-1,1],[-1,-1]];
  for (const [dr, dc] of bishopDirs) {
    let r = row + dr, c = col + dc;
    while (r >= 0 && r < 8 && c >= 0 && c < 8) {
      const piece = board[r][c];
      if (piece) {
        if (piece.color === attackerColor && (piece.type === 'bishop' || piece.type === 'queen')) return true;
        break;
      }
      r += dr;
      c += dc;
    }
  }

  return false;
}

// Apply move to a copy of the board
function applyMove(board, fromRow, fromCol, toRow, toCol, promotionType) {
  const newBoard = copyBoard(board);
  const piece = newBoard[fromRow][fromCol];
  if (!piece) return newBoard;
  newBoard[toRow][toCol] = { ...piece };
  newBoard[fromRow][fromCol] = null;
  if (promotionType) {
    newBoard[toRow][toCol].type = promotionType;
  }
  return newBoard;
}

// Main exported function
export function uciToSan(board, fromRow, fromCol, toRow, toCol, promotionType = null, isEnPassant = false) {
  const piece = board[fromRow][fromCol];
  if (!piece) return '';

  const isPawn = piece.type === 'pawn';
  const targetPiece = board[toRow][toCol];
  const isCapture = !!targetPiece || (isPawn && toCol !== fromCol) || isEnPassant;

  const destFile = String.fromCharCode(97 + toCol);
  const destRank = (8 - toRow).toString();
  const destination = destFile + destRank;

  let san = '';

  if (isPawn) {
    if (isCapture) {
      const srcFile = String.fromCharCode(97 + fromCol);
      san = srcFile + 'x' + destination;
    } else {
      san = destination;
    }
  } else {
    san = pieceSymbol[piece.type] + (isCapture ? 'x' : '') + destination;
  }

  // Promotion
  if (promotionType) {
    const promoLetter = promotionType === 'queen' ? 'Q' : promotionType === 'rook' ? 'R' : promotionType === 'bishop' ? 'B' : 'N';
    san += '=' + promoLetter;
  }

  // Check detection
  const afterBoard = applyMove(board, fromRow, fromCol, toRow, toCol, promotionType);
  const opponent = getOpponent(piece.color);
  const kingPos = findKing(afterBoard, opponent);
  if (kingPos) {
    const [kingRow, kingCol] = kingPos;
    if (isSquareAttacked(afterBoard, kingRow, kingCol, piece.color)) {
      san += '+';
    }
  }

  return san;
}