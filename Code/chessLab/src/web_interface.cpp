#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "board_config.h"
#include "hall_sensor.h"
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
body{
  background:#0f0f1a;
  min-height:100vh;
  display:flex;
  flex-direction:column;
  align-items:center;
  justify-content:center;
  font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;
  color:#e8e8e8;
  padding:16px;
  gap:16px;
}
h1{
  font-size:1.3rem;
  letter-spacing:4px;
  font-weight:300;
  text-transform:uppercase;
  color:#ccc;
}
.board-container{
  display:flex;
  flex-direction:column;
  align-items:center;
  gap:4px;
}
.board-row{
  display:flex;
  align-items:center;
  gap:4px;
}
.label{
  width:18px;
  font-size:0.7rem;
  color:#666;
  text-align:center;
  user-select:none;
}
.col-labels{
  display:flex;
  gap:0;
  padding-left:22px;
}
.col-labels .label{
  width:calc((min(88vw,88vh,420px)) / 8);
}
.board{
  display:grid;
  grid-template-columns:repeat(8,1fr);
  width:min(88vw,88vh,420px);
  height:min(88vw,88vh,420px);
  border-radius:6px;
  overflow:hidden;
  box-shadow:0 8px 32px rgba(0,0,0,0.6);
}
.sq{
  position:relative;
  display:flex;
  align-items:center;
  justify-content:center;
}
.sq.light{background:#f0d9b5}
.sq.dark {background:#b58863}
.piece{
  width:68%;
  height:68%;
  border-radius:50%;
  background:radial-gradient(circle at 35% 30%, #ff8a80, #c62828);
  box-shadow:0 0 0 0 rgba(255,100,80,0);
  opacity:0;
  transform:scale(0.3);
  transition:opacity .18s ease,transform .18s ease,box-shadow .18s ease;
}
.sq.on .piece{
  opacity:1;
  transform:scale(1);
  box-shadow:0 0 14px 5px rgba(255,80,60,0.55);
}
.footer{
  display:flex;
  flex-direction:column;
  align-items:center;
  gap:12px;
}
.dot{
  width:8px;height:8px;
  border-radius:50%;
  background:#444;
  display:inline-block;
  margin-right:6px;
  transition:background .3s;
}
.dot.ok{background:#4caf50}
#status{font-size:0.8rem;color:#666;display:flex;align-items:center}
.btn{
  padding:13px 44px;
  background:#c62828;
  color:#fff;
  border:none;
  border-radius:10px;
  font-size:1rem;
  font-weight:600;
  letter-spacing:1px;
  cursor:pointer;
  transition:background .15s,transform .1s;
  -webkit-tap-highlight-color:transparent;
}
.btn:active{background:#8e0000;transform:scale(0.96)}
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

<div class="footer">
  <div id="status"><span class="dot" id="dot"></span><span id="statusText">Connexion...</span></div>
  <button class="btn" onclick="doReset()">RESET</button>
</div>

<script>
const R=8,C=8;
const board=document.getElementById('board');
const squares=[];

// Build board squares
for(let r=0;r<R;r++){
  for(let c=0;c<C;c++){
    const sq=document.createElement('div');
    sq.className='sq '+((r+c)%2===0?'light':'dark');
    const p=document.createElement('div');
    p.className='piece';
    sq.appendChild(p);
    board.appendChild(sq);
    squares.push(sq);
  }
}

// Row labels A-H
const rowLabels=document.getElementById('rowLabels');
const sqH=`calc(min(88vw,88vh,420px) / 8)`;
for(let r=0;r<R;r++){
  const d=document.createElement('div');
  d.className='label';
  d.style.height=sqH;
  d.style.lineHeight=sqH;
  d.textContent=String.fromCharCode(65+r);
  rowLabels.appendChild(d);
}

// Col labels 1-8
const colLabels=document.getElementById('colLabels');
for(let c=0;c<C;c++){
  const d=document.createElement('div');
  d.className='label';
  d.textContent=c+1;
  colLabels.appendChild(d);
}

// WebSocket
const dot=document.getElementById('dot');
const statusText=document.getElementById('statusText');
let ws;

function connect(){
  ws=new WebSocket('ws://'+location.hostname+':81/');
  ws.onopen=()=>{
    dot.className='dot ok';
    statusText.textContent='Connecté';
  };
  ws.onclose=()=>{
    dot.className='dot';
    statusText.textContent='Déconnecté — reconnexion...';
    setTimeout(connect,2000);
  };
  ws.onmessage=(e)=>{
    const d=JSON.parse(e.data);
    d.b.forEach((row,r)=>row.forEach((v,c)=>{
      squares[r*C+c].classList.toggle('on',v===1);
    }));
  };
}
connect();

function doReset(){
  fetch('/reset').then(()=>{
    statusText.textContent='Calibration...';
    dot.className='dot';
    setTimeout(()=>{
      dot.className='dot ok';
      statusText.textContent='Connecté';
    },3000);
  }).catch(()=>{});
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

    // JSON compact : {"b":[[0,1,...],[...],...]}
    char json[2 + 4 + BOARD_SIZE * (2 + BOARD_SIZE * 2) + 4];
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
    sprintf(p, "]}");
    webSocket.broadcastTXT(json);
}
