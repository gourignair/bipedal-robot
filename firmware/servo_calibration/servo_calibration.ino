/*
 * MEGaTron Bipedal Robot
 * Real-Time Servo Calibration Tool
 *
 * Author: Gouri Girish Nair
 * University of Kent
 *
 * Independently developed as part of the
 * MEGaTron Engineering Team Project.
 *
 * This program creates a web interface for
 * adjusting six servo joints using a smartphone.
 *
 * Hardware: ESP32 / M5StickS3 and M5 8Servo Unit
 */

#include <Wire.h>
#include "M5_UNIT_8SERVO.h"
#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>

M5_UNIT_8SERVO walk_bot;
WebServer server(80);
const char* ssid = "MEGaTron";
const char* password = "robot2927";
#define R_foot 0 //servo channel for right foot
#define R_knee 1 //servo channel for right knee
#define R_hip 2 //servo channel for right hip
#define L_foot 4 //servo channel for left foot
#define L_knee 5 //servo channel for left knee
#define L_hip 6 //servo channel for left hip
//current servo positions, updated live as sliders are moved
int R_foot_current = 116;
int R_knee_current = 28;
int R_hip_current = 45;
int L_foot_current = 64;
int L_knee_current = 156;
int L_hip_current = 140;
//starting pose loaded when the tool initialises
int RF_crouch = 116, RK_crouch = 28, RH_crouch = 45;
int LF_crouch = 64, LK_crouch = 156, LH_crouch = 140;
//commands a servo to a given angle and saves the new position to memory
void setServoAndSave(int servo, int angle)
{
angle = constrain(angle, 0, 180); //prevents angles outside safe operating range
walk_bot.setServoAngle(servo, angle);
if (servo == R_foot) R_foot_current = angle;
if (servo == R_knee) R_knee_current = angle;
if (servo == R_hip) R_hip_current = angle;
if (servo == L_foot) L_foot_current = angle;
if (servo == L_knee) L_knee_current = angle;
if (servo == L_hip) L_hip_current = angle;
}
//receives a servo id and angle from the web interface and applies it
void handleServo()
{
if (server.hasArg("id") && server.hasArg("angle"))
{
setServoAndSave(server.arg("id").toInt(), server.arg("angle").toInt());
server.send(200, "text/plain", "OK");
}
else
{
server.send(400, "text/plain", "Missing id or angle");
}
}
//returns current servo angles as JSON so the web page can load current positions on startup
void handleAngles()
{
String json = "{";
json += "\"0\":" + String(R_foot_current) + ",";
json += "\"1\":" + String(R_knee_current) + ",";
json += "\"2\":" + String(R_hip_current) + ",";
json += "\"4\":" + String(L_foot_current) + ",";
json += "\"5\":" + String(L_knee_current) + ",";
json += "\"6\":" + String(L_hip_current) + "}";
server.send(200, "application/json", json);
}
//serves the slider interface webpage to any connected device
void handleRoot()
{
String page = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
<title>MEGaTron Servos</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>
body { background:#111; font-family:monospace; color:#eee; padding:20px; }
h2 { color:#00d4ff; letter-spacing:3px; margin-bottom:20px; }
.leg { margin-bottom:24px; }
.leg-label { color:#888; font-size:0.7rem; letter-spacing:3px; margin-bottom:10px; }
.row { display:flex; align-items:center; gap:12px; margin-bottom:10px; }
.name { width:48px; font-size:0.8rem; color:#aaa; }
input[type=range] {
flex:1; -webkit-appearance:none; height:4px;
background:#333; border-radius:2px; outline:none; cursor:pointer;
}
input[type=range]::-webkit-slider-thumb {
-webkit-appearance:none; width:20px; height:20px;
border-radius:50%; background:#00d4ff;
box-shadow:0 0 8px #00d4ff; cursor:pointer;
}
.val { width:32px; text-align:right; color:#00d4ff; font-size:0.85rem; }
hr { border-color:#222; margin:16px 0; }
</style>
</head>
<body>
<h2>SERVO CONTROL</h2>
<div class='leg'>
<div class='leg-label'>RIGHT LEG</div>
<div class='row'><span class='name'>FOOT</span><input type='range' min='0' max='180' id='s0'
oninput='move(0,this.value)'><span class='val' id='v0'>-</span></div>
<div class='row'><span class='name'>KNEE</span><input type='range' min='0' max='180' id='s1'
oninput='move(1,this.value)'><span class='val' id='v1'>-</span></div>
<div class='row'><span class='name'>HIP</span> <input type='range' min='0' max='180' id='s2'
oninput='move(2,this.value)'><span class='val' id='v2'>-</span></div>
</div>
<hr>
<div class='leg'>
<div class='leg-label'>LEFT LEG</div>
<div class='row'><span class='name'>FOOT</span><input type='range' min='0' max='180' id='s4'
oninput='move(4,this.value)'><span class='val' id='v4'>-</span></div>
<div class='row'><span class='name'>KNEE</span><input type='range' min='0' max='180' id='s5'
oninput='move(5,this.value)'><span class='val' id='v5'>-</span></div>
<div class='row'><span class='name'>HIP</span> <input type='range' min='0' max='180' id='s6'
oninput='move(6,this.value)'><span class='val' id='v6'>-</span></div>
</div>
<script>
const timers = {};
function move(id, val) {
document.getElementById('v' + id).textContent = val; //updates displayed angle value
clearTimeout(timers[id]);
timers[id] = setTimeout(() => { //40ms debounce prevents flooding the server with requests
fetch('/servo?id=' + id + '&angle=' + val).catch(() => {});
}, 40);
}
fetch('/angles').then(r => r.json()).then(data => { //loads current angles on page startup
for (const [id, angle] of Object.entries(data)) {
const s = document.getElementById('s' + id);
const v = document.getElementById('v' + id);
if (s) { s.value = angle; v.textContent = angle; }
}
}).catch(() => {});
</script>
</body>
</html>
)rawhtml";
server.send(200, "text/html", page);
}
void setup()
{
M5.begin();
walk_bot.begin(&Wire, 9, 10, M5_UNIT_8SERVO_DEFAULT_ADDR);
walk_bot.setAllPinMode(SERVO_CTL_MODE);
//initialise all servos to starting crouch position
setServoAndSave(R_foot, RF_crouch);
setServoAndSave(R_knee, RK_crouch);
setServoAndSave(R_hip, RH_crouch);
setServoAndSave(L_foot, LF_crouch);
setServoAndSave(L_knee, LK_crouch);
setServoAndSave(L_hip, LH_crouch);
WiFi.softAP(ssid, password); //creates the MEGaTron WiFi access point
server.on("/", handleRoot); //serves the slider webpage
server.on("/servo", handleServo); //handles incoming angle commands
server.on("/angles", handleAngles); //returns current angles to the webpage
server.begin();
}
void loop()
{
server.handleClient(); //continuously listens for incoming web requests
}


