import "./ChessBoard.css";

function ChessBoard() {
  const squares = Array(64).fill(null);

  return (
    <div className="chess-board">
      {squares.map((_, i) => {
        const isWhite = (Math.floor(i / 8) + i) % 2 === 0;

        return (
          <div
            key={i}
            className={`square ${isWhite ? "square-light" : "square-dark"}`}
          />
        );
      })}
    </div>
  );
}

export default ChessBoard;