const pieceTypes ={
    "p": "pawn",
    "r": "rook",
    "n": "knight",
    "b": "bishop",
    "q": "queen",
    "k": "king"
}

function parseFen(fen) {
    const boardPart = fen.split(' ')[0];
    const rows = boardPart.split("/");
    return rows.map(row => {
        const squares = [];
        for (let char of row) {
            if(isNaN(char)) {
                squares.push({ color: char === char.toUpperCase() ? "white" : "black", type: pieceTypes[char.toLowerCase()] })
            }else{
                const count = parseInt(char);
                for(let i =0; i < count; i++) {
                    squares.push(null);
            }
            }
        }
        return squares;
    });
}

export default parseFen;