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
  const [board] = useState(
    parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
  );

  return (
    <div className="board">
      {board.map((row, r) =>
        row.map((sq, c) => (
          <div
            key={`${r}-${c}`}
            className={`square ${(r + c) % 2 === 0 ? "light" : "dark"}`}
          >
            {sq && (
              <img
                src={pieces[`${sq.color}_${sq.type}`]}
                className="piece"
                draggable="false"
              />
            )}
          </div>
        ))
      )}
    </div>
  );
}

export default ChessBoard;