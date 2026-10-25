
/*
 * MEGaTron Bipedal Robot
 * Walking Algorithm - Project Report Extract
 *
 * Developed collaboratively by the MEGaTron software team.
 *
 * NOTE:
 * This file documents the walking algorithm
 * from the university group project.
 *
 * The original setup() function and web
 * command handlers are not included.
 *
 * This extract is not standalone firmware.
 */

#include <Wire.h>
#include "M5_UNIT_8SERVO.h"
#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>

M5_UNIT_8SERVO walk_bot; //creates the servo unit object used to send angle commands
WebServer server(80); //creates the web server running on port 80
const char* ssid = "MEGaTron"; //name of the WiFi network broadcast by the robot
const char* password = "robot2927"; //password required to connect to the robot network
#define R_foot 0 //servo channel number for the right foot
#define R_knee 1 //servo channel number for the right knee
#define R_hip 2 //servo channel number for the right hip
#define L_foot 4 //servo channel number for the left foot
#define L_knee 5 //servo channel number for the left knee
#define L_hip 6 //servo channel number for the left hip
bool walking = false; //tracks whether the robot is currently in a walking state
bool stopRequested = false; //flag set when the stop command is received mid-walk
//current servo positions used as starting points for smooth interpolation
int R_foot_current = 79; int R_knee_current = 93; int R_hip_current = 69;
int L_foot_current = 93; int L_knee_current = 100; int L_hip_current = 108;
//servo angles for the default standing pose
int RF_stand=79, RK_stand=93, RH_stand=69;
int LF_stand=93, LK_stand=100, LH_stand=108;
//servo angles for the crouching pose
int RF_crouch=91, RK_crouch=28, RH_crouch=0;
int LF_crouch=99, LK_crouch=156, LH_crouch=180;
//servo angles for the walking ready stance
int RF_walkStand=94, RK_walkStand=61, RH_walkStand=55;
int LF_walkStand=84, LK_walkStand=128, LH_walkStand=133;
int stepDelay = 12; //delay in milliseconds between each interpolation step, controls movement speed
//moves all six servos simultaneously from their current positions to the target angles
//interpolation ensures smooth transitions rather than sudden jumps between poses
void slowMoveAll(int RF, int RK, int RH, int LF, int LK, int LH) {
int maxSteps = max({abs(RF-R_foot_current), abs(RK-R_knee_current),
abs(RH-R_hip_current), abs(LF-L_foot_current),
abs(LK-L_knee_current), abs(LH-L_hip_current)});
if (maxSteps == 0) return; //no movement needed if already at target position
for (int i = 0; i <= maxSteps; i++) {
walk_bot.setServoAngle(R_foot, R_foot_current + ((RF-R_foot_current)*i)/maxSteps);
walk_bot.setServoAngle(R_knee, R_knee_current + ((RK-R_knee_current)*i)/maxSteps);
walk_bot.setServoAngle(R_hip, R_hip_current + ((RH-R_hip_current)*i)/maxSteps);
walk_bot.setServoAngle(L_foot, L_foot_current + ((LF-L_foot_current)*i)/maxSteps);
walk_bot.setServoAngle(L_knee, L_knee_current + ((LK-L_knee_current)*i)/maxSteps);
walk_bot.setServoAngle(L_hip, L_hip_current + ((LH-L_hip_current)*i)/maxSteps);
server.handleClient(); //keeps the web server responsive during movement
delay(stepDelay); //pause between each interpolation step
}
}
//executes one full walking cycle consisting of four movement phases
void walkStep() {
slowMoveAll(94, 52, 55, 61, 136, 109); delay(40); //leg steps forward
if (stopRequested) return; //checks for stop command before continuing
slowMoveAll(105, 56, 63, 72, 134, 122); delay(40); // weight transfers to left leg
if (stopRequested) return;
slowMoveAll(129, 54, 82, 66, 142, 120); delay(40); //right leg steps forward
if (stopRequested) return;
slowMoveAll(105, 56, 63, 72, 134, 122); delay(40); //weight transfers to right leg
}
void loop() {
server.handleClient(); //continuously checks for incoming web interface commands
if (walking) {
walkStep(); //executes one walking cycle if walking is active
if (stopRequested) {
slowMoveAll(RF_walkStand,RK_walkStand,RH_walkStand,
LF_walkStand,LK_walkStand,LH_walkStand); //returns to stable standing pose on stop
walking = false;
stopRequested = false;
}
}
}
