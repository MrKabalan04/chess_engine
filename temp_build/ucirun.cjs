const { spawn } = require("child_process");
const p = spawn("D:/chess_engine/temp_build/try0/board.exe", [], { stdio: ["pipe", "pipe", "pipe"] });
p.stdout.on("data", d => process.stdout.write("[OUT] " + d));
p.stderr.on("data", d => process.stdout.write("[ERR] " + d));
p.stdin.write("uci\n");
setTimeout(() => {
  p.stdin.write("position fen 6k1/8/8/8/8/8/4P3/4K3 w - - 0 1\n");
  p.stdin.write("go movetime 600\n");
}, 500);
setTimeout(() => { p.stdin.write("quit\n"); p.kill(); }, 4000);
