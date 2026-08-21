// convtest.js - plays a given FEN with Luna on both sides (Luna always moves).
// Prints the full move log and the final result (checkmate/stalemate/perpetual).
const { spawn } = require('child_process');

const EXE = process.argv[2];
const MOVETIME = process.argv[3] ? parseInt(process.argv[3]) : 1000;
const startFen = process.argv[4];

let engine, buf = '';
let gameMoves = [];

function init() {
  return new Promise((res, rej) => {
    engine = spawn(EXE, []);
    engine.on('error', rej);
    engine.stdout.setEncoding('utf8');
    engine.stdout.on('data', (d) => { buf += d; });
    engine.stdin.write('uci\n');
    setTimeout(() => {
      if (buf.includes('uciok')) res();
      else rej(new Error('no uciok'));
    }, 1500);
  });
}

function doSearch(posCmd) {
  return new Promise((res, rej) => {
    buf = '';
    engine.stdin.write(posCmd + '\ngo movetime ' + MOVETIME + '\n');
    const t0 = Date.now();
    const iv = setInterval(() => {
      if (buf.includes('bestmove')) {
        clearInterval(iv);
        // capture bestmove line only
        const m = buf.split('\n').filter(l => l.startsWith('bestmove'));
        const line = m[m.length - 1];
        const mv = line.split(' ')[1];
        // mate detection: last "info depth" line may contain "mate N"
        let mate = null, cp = null;
        const infoLines = buf.split('\n').filter(l => l.startsWith('info'));
        for (const il of infoLines) {
          const mm = il.match(/\bmate (-?\d+)/);
          if (mm) mate = parseInt(mm[1]);
          const cc = il.match(/\bcp (-?\d+)/);
          if (cc) cp = parseInt(cc[1]);
        }
        res({ mv, mate, cp, info: infoLines.join('\n') });
      } else if (Date.now() - t0 > MOVETIME + 10000) {
        clearInterval(iv);
        rej(new Error('search timeout, buf=' + buf.slice(-400)));
      }
    }, 15);
  });
}

function fenWithMoves() {
  const base = startFen.split(' ').slice(0, 4).join(' ');
  return base + ' moves ' + gameMoves.join(' ');
}

async function main() {
  await init();
  console.log('FEN:', startFen);
  const startStm = startFen.split(' ')[1];
  let stm = startStm; // 'w' or 'b'
  for (let ply = 0; ply < 600; ply++) {
    const pos = 'position fen ' + fenWithMoves();
    let r;
    try { r = await doSearch(pos); }
    catch (e) {
      console.error('  search error:', e.message);
      break;
    }
    if (r.mv === '(none)' || !r.mv) {
      // stalemate or checkmate
      console.log('[' + ply + '] ' + stm + ' has no moves — game over (mate=' + r.mate + ', cp=' + r.cp + ')');
      console.log('  result: ' + (r.mate === 0 ? 'CHECKMATE' : 'STALEMATE'));
      break;
    }
    gameMoves.push(r.mv);
    console.log('[' + ply + '] ' + stm + ': ' + r.mv + '   score cp=' + r.cp + ' mate=' + r.mate);
    // even if search reported mate N, keep going until actual mate
    stm = stm === 'w' ? 'b' : 'w';
  }
  console.log('---');
  console.log('GAME:', gameMoves.join(' '));
  console.log('LENGTH:', gameMoves.length / 2 + ' moves (game), ' + gameMoves.length + ' plies');
  engine.kill();
  process.exit(0);
}

main().catch((e) => { console.error(e); process.exit(1); });