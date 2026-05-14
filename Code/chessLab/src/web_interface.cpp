#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "board_config.h"
#include "hall_sensor.h"
#include "chess_game.h"
#include "logger.h"
#include "web_interface.h"

static WebServer        server(80);
static WebSocketsServer webSocket(81);

// ------------------------------------------------------------
// HTML stocké en flash (PROGMEM) pour économiser la RAM
// ------------------------------------------------------------
static const char HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>ChessLab</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0f0f1a;min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;color:#e8e8e8;padding:16px;gap:16px}
h1{font-size:1.3rem;letter-spacing:4px;font-weight:300;text-transform:uppercase;color:#ccc}
.board-container{display:flex;flex-direction:column;align-items:center;gap:4px}
.board-row{display:flex;align-items:center;gap:4px}
.label{width:18px;font-size:.7rem;color:#666;text-align:center;user-select:none}
.col-labels{display:flex;gap:0;padding-left:22px}
.col-labels .label{width:calc(min(88vw,88vh,420px)/8)}
.board{display:grid;grid-template-columns:repeat(8,1fr);width:min(88vw,88vh,420px);height:min(88vw,88vh,420px);border-radius:6px;overflow:hidden;box-shadow:0 8px 32px rgba(0,0,0,.6)}
.sq{position:relative;display:flex;align-items:center;justify-content:center;transition:box-shadow .2s}
.sq.light{background:#f0d9b5}.sq.dark{background:#b58863}
.piece{font-size:calc(min(88vw,88vh,420px)/10.5);line-height:1;opacity:0;transform:scale(.4);transition:opacity .18s,transform .18s;user-select:none;pointer-events:none}
.sq.on .piece{opacity:1;transform:scale(1)}
.ws .piece{color:#fff;text-shadow:0 1px 5px #000,0 0 2px #000}
.bs .piece{color:#111;text-shadow:0 0 4px rgba(255,255,255,.5),0 1px 3px #000}
.led-g{box-shadow:inset 0 0 0 3px rgba(0,220,60,.8)!important}
.led-o{box-shadow:inset 0 0 0 3px rgba(255,150,0,.65)!important}
.led-r{box-shadow:inset 0 0 0 3px rgba(255,50,30,.6)!important}
.led-r.on .piece{color:#e53935;text-shadow:0 0 8px rgba(255,60,40,.7)}
.footer{display:flex;flex-direction:column;align-items:center;gap:12px}
.dot{width:8px;height:8px;border-radius:50%;background:#444;display:inline-block;margin-right:6px;transition:background .3s}
.dot.ok{background:#4caf50}
#status{font-size:.8rem;color:#666;display:flex;align-items:center}
.btn{padding:13px 44px;background:#c62828;color:#fff;border:none;border-radius:10px;font-size:1rem;font-weight:600;letter-spacing:1px;cursor:pointer;transition:background .15s,transform .1s;-webkit-tap-highlight-color:transparent}
.btn:active{background:#8e0000;transform:scale(.96)}
#setupPanel{background:rgba(20,20,35,.97);border:1px solid #2a2a3a;border-radius:12px;padding:14px 20px;text-align:center;max-width:360px;width:100%;transition:opacity .5s,transform .5s}
#setupPanel.hidden{opacity:0;transform:translateY(16px);pointer-events:none;height:0;padding:0;border:none;overflow:hidden}
#sIcon{font-size:2rem;margin-bottom:6px;animation:pulse 1.4s ease-in-out infinite}
#sTitle{font-size:.95rem;font-weight:600;letter-spacing:2px;text-transform:uppercase;color:#e8e8e8;margin-bottom:4px}
#sDesc{font-size:.75rem;color:#666;margin-bottom:10px}
#sProg{font-size:.85rem;color:#999}
#sProg b{font-size:1.3rem;color:#4caf50}
@keyframes pulse{0%,100%{opacity:.5}50%{opacity:1}}
</style>
</head>
<body>
<h1>&#9820; ChessLab</h1>
<div class="board-container">
  <div class="col-labels" id="colLabels"></div>
  <div class="board-row">
    <div style="display:flex;flex-direction:column;gap:0" id="rowLabels"></div>
    <div class="board" id="board"></div>
  </div>
</div>
<div id="setupPanel">
  <div id="sIcon">&#9817;</div>
  <div id="sTitle">Position de d&eacute;part</div>
  <div id="sDesc">Placez les 32 pi&egrave;ces sur les rangs 1&middot;2 et 7&middot;8</div>
  <div id="sProg"><b id="pCount">0</b>&thinsp;/ 32 pi&egrave;ces plac&eacute;es</div>
</div>
<div class="footer">
  <div id="status"><span class="dot" id="dot"></span><span id="statusText">Connexion...</span></div>
  <button class="btn" onclick="doReset()">RESET</button>
</div>
<script>
const R=8,C=8;
const BOARD=document.getElementById('board');
const squares=[];

// Symboles position de depart en coords AFFICHAGE (apres transposition capteur→ecran)
// Ligne affichage = colonne capteur (rang echecs)
// Blanc en bas : rang 1 → ligne d'affichage 7, rang 8 → ligne 0
const START=[
  ['♜','♞','♝','♛','♚','♝','♞','♜'],  // ligne 0 = rang 8 (noirs)
  ['♟','♟','♟','♟','♟','♟','♟','♟'],  // ligne 1 = rang 7
  ['','','','','','','',''],
  ['','','','','','','',''],
  ['','','','','','','',''],
  ['','','','','','','',''],
  ['♙','♙','♙','♙','♙','♙','♙','♙'],  // ligne 6 = rang 2
  ['♖','♘','♗','♕','♔','♗','♘','♖'],  // ligne 7 = rang 1 (blancs)
];

for(let r=0;r<R;r++){
  for(let c=0;c<C;c++){
    const sq=document.createElement('div');
    sq.className='sq '+((r+c)%2===0?'light':'dark');
    if(r<=1) sq.classList.add('bs');  // pieces noires
    if(r>=6) sq.classList.add('ws');  // pieces blanches
    const p=document.createElement('div');
    p.className='piece';
    p.textContent=START[r][c];
    sq.appendChild(p);
    BOARD.appendChild(sq);
    squares.push(sq);
  }
}

// Etiquettes lignes : rang 8 en haut → rang 1 en bas
const rowLabels=document.getElementById('rowLabels');
const sqH='calc(min(88vw,88vh,420px)/8)';
for(let r=0;r<R;r++){
  const d=document.createElement('div');
  d.className='label';
  d.style.height=sqH;d.style.lineHeight=sqH;
  d.textContent=R-r;
  rowLabels.appendChild(d);
}

// Etiquettes colonnes : A-H (fichiers)
const colLabels=document.getElementById('colLabels');
for(let c=0;c<C;c++){
  const d=document.createElement('div');
  d.className='label';
  d.textContent=String.fromCharCode(65+c);
  colLabels.appendChild(d);
}

const dot=document.getElementById('dot');
const statusText=document.getElementById('statusText');
const setupPanel=document.getElementById('setupPanel');
const pCount=document.getElementById('pCount');
let ws,prevPhase=-1;

function connect(){
  ws=new WebSocket('ws://'+location.hostname+':81/');
  ws.onopen=()=>{dot.className='dot ok';statusText.textContent='Connect\xe9';};
  ws.onclose=()=>{dot.className='dot';statusText.textContent='D\xe9connect\xe9 — reconnexion...';setTimeout(connect,2000);};
  ws.onmessage=(e)=>{
    const d=JSON.parse(e.data);
    const ph=(d.ph!==undefined)?d.ph:0;
    let cnt=0;

    for(let sr=0;sr<R;sr++){
      for(let sc=0;sc<C;sc++){
        const v=d.b[sr][sc];
        // TRANSPOSITION + blanc en bas :
        // capteur(sr,sc) → affichage(ligne = R-1-sc, col = sr)
        const dr=R-1-sc;
        const sq=squares[dr*C+sr];
        const piece=sq.querySelector('.piece');

        sq.classList.toggle('on',v===1);
        sq.classList.remove('led-g','led-o','led-r');

        // Symbole de piece
        if(ph===0) piece.textContent=(v===1?(START[dr][sr]||'●'):'');
        else       piece.textContent=(v===1?'●':'');

        // Couleur LED sur la case
        const isTarget=(sc===0||sc===1||sc===6||sc===7);
        if(ph===0){
          if(isTarget){if(v===1){sq.classList.add('led-g');cnt++;}else sq.classList.add('led-o');}
          else if(v===1) sq.classList.add('led-r');
        } else {
          if(v===1) sq.classList.add('led-r');
        }
      }
    }

    pCount.textContent=cnt;
    if(ph===0) setupPanel.classList.remove('hidden');
    if(ph===1&&prevPhase===0){
      document.getElementById('sIcon').textContent='✓';
      document.getElementById('sTitle').textContent='Partie lanc\xe9e !';
      document.getElementById('sDesc').textContent='Bonne position — la partie commence';
      document.getElementById('sProg').style.display='none';
      setTimeout(()=>setupPanel.classList.add('hidden'),2500);
    }
    prevPhase=ph;
  };
}
connect();

function doReset(){
  fetch('/reset').then(()=>{statusText.textContent='Calibration...';dot.className='dot';setTimeout(()=>{dot.className='dot ok';statusText.textContent='Connect\xe9';},3000);}).catch(()=>{});
}
</script>
</body>
</html>
)rawhtml";

// ------------------------------------------------------------
// WebSocket events
// ------------------------------------------------------------
static void on_ws_event(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_CONNECTED)
        LOG("[Web] Client WS #%u connecté\n", num);
    else if (type == WStype_DISCONNECTED)
        LOG("[Web] Client WS #%u déconnecté\n", num);
}

// ------------------------------------------------------------
// HTTP routes
// ------------------------------------------------------------
static void on_root() {
    server.send_P(200, "text/html", HTML);
}

static void on_reset() {
    server.send(200, "text/plain", "OK");
    LOG("[Web] Reset demandé — recalibration...\n");
    hall_calibrate();
}

// ------------------------------------------------------------
// API publique
// ------------------------------------------------------------
void web_init() {
    server.on("/", on_root);
    server.on("/reset", on_reset);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(on_ws_event);
    LOG("[Web] HTTP port 80 — WebSocket port 81\n");
}

void web_update() {
    server.handleClient();
    webSocket.loop();
}

void web_broadcast_state(int dev[BOARD_SIZE][BOARD_SIZE]) {
    if (webSocket.connectedClients() == 0) return;

    // N'envoyer que si l'état a changé (+ heartbeat toutes les 2 s)
    // → réduit les TX WiFi pendant les scans hall
    static uint8_t  prev[BOARD_SIZE][BOARD_SIZE] = {};
    static uint32_t last_ms = 0;

    uint32_t now = millis();
    bool changed = false;
    for (int r = 0; r < BOARD_SIZE && !changed; r++)
        for (int c = 0; c < BOARD_SIZE && !changed; c++)
            if (((dev[r][c] >= THRESHOLD) ? 1 : 0) != prev[r][c]) changed = true;

    if (!changed && (now - last_ms < 2000)) return;
    last_ms = now;

    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            prev[r][c] = (dev[r][c] >= THRESHOLD) ? 1 : 0;

    char json[200];
    char* p = json;
    p += sprintf(p, "{\"b\":[");
    for (int r = 0; r < BOARD_SIZE; r++) {
        *p++ = '[';
        for (int c = 0; c < BOARD_SIZE; c++) {
            *p++ = (dev[r][c] >= THRESHOLD) ? '1' : '0';
            if (c < BOARD_SIZE-1) *p++ = ',';
        }
        *p++ = ']';
        if (r < BOARD_SIZE-1) *p++ = ',';
    }
    sprintf(p, "],\"ph\":%d}", (int)game_get_phase());
    webSocket.broadcastTXT(json);
}
