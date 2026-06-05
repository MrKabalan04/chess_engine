  import "./ChessBoard.css";
  import parseFen from "./parseFen";
  import { useState } from "react";

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

    function handleClick(row, col) {
      
      const piece = board[row][col];

      if (!piece && !selectedSquare) return;

      if (piece && !selectedSquare) {
        setSelectedSquare([row, col]);
        setLegalMoves([[3, 3], [4, 4], [5, 5]]);
        return;
      }
      const isLegalMove = legalMoves.some(m => m[0] === row && m[1] === col);
      if(isLegalMove) {
        const newBoard = board.map(r => r.slice());
        newBoard[row][col] = board[selectedSquare[0]][selectedSquare[1]];
        newBoard[selectedSquare[0]][selectedSquare[1]] = null;
        setBoard(newBoard);
      }
      setSelectedSquare(null);
      setLegalMoves([]);
    }

    return (
      <div className="board">
        {board.map((row, r) =>
          row.map((sq, c) => (
            <div
              key={`${r}-${c}`}
              className={`square ${(r + c) % 2 === 0 ? "light" : "dark"} ${
                selectedSquare && selectedSquare[0] === r && selectedSquare[1] === c ? "selected" : ""
              } ${
                legalMoves.some((m) => m[0] === r && m[1] === c) ? "legal-move" : ""
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
    );
  }

  export default ChessBoard;