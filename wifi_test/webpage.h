#pragma once

//R"rawliteral allows to write through several lines
// Progmen tells the compiller to write in the flash, not the RAM.

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Drone control</title>
<style>
body { font-family: sans-serif; text-align: center; user-select: none; -webkit-user-select: none; }
button { width: 120px; height: 120px; font-size: 24px; margin: 10px; touch-action: none; }
#joy { position: relative; width: 200px; height: 200px; margin: 20px auto;
       border-radius: 50%; background: #ddd; touch-action: none; }
#knob { position: absolute; width: 80px; height: 80px; border-radius: 50%;
        background: #555; left: 60px; top: 60px; }
</style>
</head><body>
<h1>Drone control</h1>

<div id="joy"><div id="knob"></div></div>

<button id="up">UP</button>
<button id="down">DOWN</button>

<script>
const socket = new WebSocket('ws://' + location.hostname + ':81/');

function sendMsg(text) {
  if (socket.readyState === WebSocket.OPEN) {
    socket.send(text);
  }
}

// ---------- UP / DOWN buttons ----------
function holdButton(id, command) {
  const btn = document.getElementById(id);
  let pressed = false;

  function press() {
    pressed = true;
    sendMsg(command);
  }

  function release() {
    if (pressed) {
      pressed = false;
      sendMsg('STOP');
    }
  }

  btn.addEventListener('pointerdown', press);
  btn.addEventListener('pointerup', release);
  btn.addEventListener('pointercancel', release);
  btn.addEventListener('pointerleave', release);
}

holdButton('up', 'UP');
holdButton('down', 'DOWN');

// ---------- Joystick ----------
const joy = document.getElementById('joy');
const knob = document.getElementById('knob');
const RADIUS = 60;
let dragging = false;
let lastSent = '';

function sendJoystick(x, y) {
  const msg = 'J,' + x + ',' + y;
  if (msg !== lastSent) {
    lastSent = msg;
    sendMsg(msg);
  }
}

function moveKnob(event) {
  const rect = joy.getBoundingClientRect();
  let dx = event.clientX - (rect.left + rect.width / 2);
  let dy = event.clientY - (rect.top + rect.height / 2);
  const dist = Math.hypot(dx, dy);
  if (dist > RADIUS) {
    dx = dx * RADIUS / dist;
    dy = dy * RADIUS / dist;
  }
  knob.style.transform = 'translate(' + dx + 'px,' + dy + 'px)';
  sendJoystick(Math.round(dx / RADIUS * 100), Math.round(-dy / RADIUS * 100));
}

function releaseJoy() {
  if (!dragging) return;
  dragging = false;
  knob.style.transform = 'translate(0px,0px)';
  sendJoystick(0, 0);
}

joy.addEventListener('pointerdown', (e) => {
  dragging = true;
  joy.setPointerCapture(e.pointerId);
  moveKnob(e);
});
joy.addEventListener('pointermove', (e) => { if (dragging) moveKnob(e); });
joy.addEventListener('pointerup', releaseJoy);
joy.addEventListener('pointercancel', releaseJoy);

document.addEventListener('contextmenu', (e) => e.preventDefault());
</script>
</body></html>
)rawliteral";