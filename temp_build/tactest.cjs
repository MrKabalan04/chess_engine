// tactest.cjs - runs Luna against tactical positions with known best moves.
// Usage: node tactest.cjs <exe> [movetime]
const { spawn } = require('child_process');

const EXE = process.argv[2];
const MOVETIME = process.argv[3] ? parseInt(process.argv[3]) : 1000;

const SUITE = [
  { name: 'WAC.001 Qg6',        fen: '2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1', ok: ['g3g6'] },
  { name: 'WAC.002 Rxb2',       fen: '8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1',              ok: ['b3b2'] },
  { name: 'WAC.003 Rg3',        fen: '5rk1/1ppb3p/p1pb4/6q1/3P1p1r/2P1R2P/PP1BQ1P1/5RKN w - - 0 1', ok: ['e3g3'] },
  { name: 'WAC.004 Qxh7#',      fen: 'r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - - 0 1',  ok: ['h6h7'] },
  { name: 'WAC.005 Qc4+',       fen: '5k2/6pp/p1qN4/1p1p4/3P4/2PKP2Q/PP3r2/3R4 b - - 0 1',          ok: ['c6c4'] },
  { name: 'backrank Rxd8',      fen: '3r2k1/5ppp/8/8/8/8/5PPP/3R2K1 w - - 0 1',                     ok: ['d1d8'] },
  { name: 'WAC.010 Qd1+',       fen: '1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/6K1 b - - 0 1',        ok: ['d6d1'] },
  { name: 'matein2 Qd8+',       fen: 'r1b2k1r/ppp1bppp/8/1B1Q4/5q2/2P5/PPP2PPP/R3R1K1 w - - 0 1',   ok: ['d5d8'] },
  { name: 'promo race b1=Q',    fen: '8/1k6/8/8/8/8/1p6/3K4 b - - 0 1',                             ok: ['b2b1q', 'b2b1r', 'b2b1b', 'b2b1n'] },
  { name: 'free queen Kxe2',    fen: '4k3/8/8/8/8/8/4q3/4K2R w - - 0 1',                            ok: ['e1e2'] },
];

let engine, lineBuf = '', waiting = null, lastInfo = '', uciReady = false;

function init() {
  return new Promise((resolve, reject) => {
    engine = spawn(EXE, []);
    engine.on('error', reject);
    engine.stdout.setEncoding('utf8');
    engine.stdout.on('data', (d) => {
      lineBuf += d;
      let lines = lineBuf.split('\n');
      lineBuf = lines.pop();
      for (const ln of lines) {
        const t = ln.trim();
        if (!t) continue;
        if (t.includes('uciok')) uciReady = true;
        if (t.startsWith('info')) lastInfo = t;
        if (waiting && t.startsWith(waiting.match)) {
          const w = waiting; waiting = null; w.resolve(t);
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
  return new Promise((resolve) => { waiting = { match, resolve }; });
}

async function runOne(fen) {
  lastInfo = '';
  engine.stdin.write('position fen ' + fen + '\n');
  engine.stdin.write('go movetime ' + MOVETIME + '\n');
  const bm = await waitFor('bestmove');
  const mv = bm.split(' ')[1];
  const dm = lastInfo.match(/\bdepth (\d+)/);
  const sm = lastInfo.match(/\bscore (cp (-?\d+)|mate (-?\d+))/);
  return {
    mv,
    depth: dm ? parseInt(dm[1]) : 0,
    score: sm ? (sm[2] !== undefined ? 'cp ' + sm[2] : 'mate ' + sm[3]) : '?',
  };
}

async function main() {
  await init();
  let pass = 0;
  const rows = [];
  for (const t of SUITE) {
    const r = await runOne(t.fen);
    const good = t.ok.includes(r.mv);
    if (good) pass++;
    rows.push(`${good ? 'PASS' : 'FAIL'}  ${t.name.padEnd(20)} got=${(r.mv || '(none)').padEnd(7)} want=${t.ok[0].padEnd(7)} d=${String(r.depth).padEnd(3)} ${r.score}`);
  }
  console.log(rows.join('\n'));
  console.log(`\n${pass}/${SUITE.length} passed`);
  engine.kill();
  process.exit(0);
}

main().catch((e) => { console.error(e); process.exit(1); });
