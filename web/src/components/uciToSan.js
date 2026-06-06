// utils/uciToSan.js

const pieceSymbol = {
  pawn: '',
  knight: 'N',
  bishop: 'B',
  rook: 'R',
  queen: 'Q',
  king: 'K'
};

export function uciToSan(board, fromRow, fromCol, toRow, toCol, promotionType = null) {
  const piece = board[fromRow][fromCol];
  if (!piece) return '';

  const isPawn = piece.type === 'pawn';
  const targetPiece = board[toRow][toCol];
  const isCapture = !!targetPiece || (isPawn && toCol !== fromCol); // en passant later

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

  // TODO: optionally add check/checkmate by calling isSquareAttacked after move
  return san;
}