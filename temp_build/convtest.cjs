// convtest.cjs - plays a given FEN with Luna on both sides (Luna always moves).
// Prints the full move log and the final result (checkmate/stalemate/100-move).
const { spawn } = require('child_process');

const EXE = process.argv[2];
const MOVETIME = process.argv[3] ? parseInt(process.argv[3]) : 1000;
const startFen = process.argv[4];

let engine;
let lineBuf = '';
let uciReady = false;
let waiting = null; // { match, resolve }
let lastMate = null; // mate score of last info line in current go
let gameMoves = [];

function init() {
  return new Promise((resolve, reject) => {
    engine = spawn(EXE, []);
    engine.on('error', reject);
    engine.stdout.setEncoding('utf8');
    engine.stdout.on('data', (d) => {
      lineBuf += d;
      if (lineBuf.includes('uciok')) uciReady = true;
      let lines = lineBuf.split('\n');
      lineBuf = lines.pop();
      for (const ln of lines) {
        const t = ln.trim();
        if (!t) continue;
        if (t.startsWith('info')) {
          if (t.includes('TB') || t.includes('Tablebase') || t.includes('KPK')) console.error(t);
          const mm = t.match(/\bmate (-?\d+)/);
          if (mm) lastMate = parseInt(mm[1]);
        }
        if (waiting && t.startsWith(waiting.match)) {
          const w = waiting; waiting = null;
          w.resolve(t);
        }
      }
    });
    engine.stdin.write('uci\n');
    const t0 = Date.now();
    const iv = setInterval(() => {
      if (uciReady) { clearInterval(iv); resolve(); }
      else if (Date.now() - t0 > 5000) { clearInterval(iv); reject(new Error('no uciok')); }
    }, 20);
  });
}

function waitFor(match) {
  return new Promise((resolve) => {
    waiting = { match, resolve };
  });
}

async function go(positionCmd) {
  lastMate = null;
  engine.stdin.write(positionCmd + '\ngo movetime ' + MOVETIME + '\n');
  const bm = await waitFor('bestmove');
  return { mv: bm.split(' ')[1] };
}

function fenWithMoves() {
  const base = startFen.split(' ').slice(0, 4).join(' ');
  return base + ' moves ' + gameMoves.join(' ');
}

async function main() {
  await init();
  console.log('FEN:', startFen);
  console.log('---');
  for (let ply = 0; ply < 400; ply++) {
    const posCmd = 'position fen ' + fenWithMoves();
    const r = await go(posCmd);
    if (!r.mv || r.mv === '(none)' || r.mv === '0000') {
      const wtm = gameMoves.length % 2 === 0;
      console.log('NO MOVES at ply ' + ply + ' (side-to-move ' + (wtm ? 'White' : 'Black') + ') — lastMate=' + lastMate);
      break;
    }
    gameMoves.push(r.mv);
    const mover = (gameMoves.length + (startFen.split(' ')[1] === 'w' ? 0 : 1)) % 2 === 1 ? 'W' : 'B';
    console.log('[' + String(ply).padStart(3) + '] ' + mover + ': ' + r.mv + '  mate=' + lastMate);
  }
  console.log('---');
  console.log('GAME:', gameMoves.join(' '));
  console.log('LENGTH:', (gameMoves.length / 2).toFixed(1) + ' moves, ' + gameMoves.length + ' plies');
  console.log('OUTCOME:', lastMate !== null && lastMate >= 0 ? 'White mates' : lastMate !== null ? 'White is mated' : '(mate score not seen)');
  engine.kill();
  process.exit(0);
}

main().catch((e) => { console.error(e); process.exit(1); });