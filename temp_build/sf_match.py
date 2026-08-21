import argparse
import math
import sys

import chess
import chess.engine

LUNA = r"D:\chess_engine\Luna\chess_engine.exe"
SF = r"D:\chess_engine\temp_build\sf\stockfish\stockfish-windows-x86-64-avx2.exe"


def play_game(luna, sf, luna_white, movetime_s, max_plies, game_no):
    board = chess.Board()
    game_key = f"game{game_no}"
    while not board.is_game_over(claim_draw=False) and board.ply() < max_plies:
        if board.can_claim_draw():
            break
        engine = luna if (board.turn == chess.WHITE) == luna_white else sf
        result = engine.play(board, chess.engine.Limit(time=movetime_s), game=game_key)
        if result.move is None:
            break
        board.push(result.move)

    if board.is_checkmate():
        luna_color = chess.WHITE if luna_white else chess.BLACK
        pts = 1.0 if board.turn != luna_color else 0.0
        reason = "checkmate"
    elif board.is_stalemate():
        pts = 0.5
        reason = "stalemate"
    elif board.is_insufficient_material():
        pts = 0.5
        reason = "insufficient material"
    elif board.can_claim_threefold_repetition():
        pts = 0.5
        reason = "threefold"
    elif board.can_claim_fifty_moves():
        pts = 0.5
        reason = "fifty-move"
    else:
        pts = 0.5
        reason = "adjudicated (ply cap)"

    return pts, reason, board


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--games", type=int, default=10)
    ap.add_argument("--elo", type=int, default=1600, help="Stockfish UCI_Elo 1320-3190")
    ap.add_argument("--movetime", type=float, default=0.5, help="seconds per move")
    ap.add_argument("--max-plies", type=int, default=300)
    args = ap.parse_args()

    points = 0.0
    played = 0
    results = {"W": 0, "D": 0, "L": 0}

    with chess.engine.SimpleEngine.popen_uci(LUNA) as luna, \
         chess.engine.SimpleEngine.popen_uci(SF) as sf:
        sf.configure({"UCI_LimitStrength": True, "UCI_Elo": args.elo})
        print(f"Luna vs Stockfish 18 @ UCI_Elo {args.elo}, {args.movetime}s/move, "
              f"{args.games} games", flush=True)

        for g in range(args.games):
            luna_white = (g % 2 == 0)
            try:
                pts, reason, board = play_game(luna, sf, luna_white,
                                               args.movetime, args.max_plies, g)
            except chess.engine.EngineError as e:
                print(f"game {g+1}: ENGINE ERROR: {e}", flush=True)
                pts, reason, board = 0.0, "engine error", chess.Board()
            played += 1
            points += pts
            results["W" if pts == 1 else "D" if pts == 0.5 else "L"] += 1
            color = "White" if luna_white else "Black"
            print(f"game {g+1}: Luna({color}) {'+' if pts==1 else '=' if pts==0.5 else '-'} "
                  f"{reason} ({board.ply()} plies) | total {points}/{played}", flush=True)

    print(f"\nfinal: Luna scored {points}/{played} "
          f"(W{results['W']} D{results['D']} L{results['L']}) vs SF18@{args.elo}")
    if played and 0 < points / played < 1:
        s = points / played
        diff = -400 * math.log10(1 / s - 1)
        est = args.elo + diff
        err = 400 / math.sqrt(played)
        print(f"estimated Luna Elo: {est:.0f} (+/- {err:.0f})")


if __name__ == "__main__":
    main()
