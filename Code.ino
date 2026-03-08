#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "ESP32_ARM";
const char* password = "12345678";

WebServer server(80);

Servo s1,s2,s3,s4;

int pos[4]={90,90,90,90};
int target[4]={90,90,90,90};

bool playMode=false;
bool recordMode=false;

struct Step{
int a,b,c,d;
};

Step steps[300];
int stepCount=0;

unsigned long lastRecord=0;

String page = R"====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>

body{
font-family:Arial;
text-align:center;
background:#111;
color:white;
}

h2{
color:#00ffaa;
}

.slider{
width:90%;
}

button{
width:120px;
padding:12px;
margin:8px;
font-size:16px;
border:none;
border-radius:8px;
}

.rec{background:red;color:white;}
.play{background:green;color:white;}
.stop{background:orange;color:white;}

#msg{
margin-top:20px;
font-size:18px;
color:#00ffaa;
}

</style>
</head>

<body>

<h2>ESP32 Robot Arm</h2>

Servo 1<br>
<input class="slider" type="range" min="0" max="180" value="90" oninput="sv(0,this.value)"><br>

Servo 2<br>
<input class="slider" type="range" min="0" max="180" value="90" oninput="sv(1,this.value)"><br>

Servo 3<br>
<input class="slider" type="range" min="0" max="180" value="90" oninput="sv(2,this.value)"><br>

Servo 4<br>
<input class="slider" type="range" min="0" max="180" value="90" oninput="sv(3,this.value)"><br><br>

<button class="rec" onclick="record()">Start Recording</button>
<button class="play" onclick="play()">Play Loop</button>
<button class="stop" onclick="stop()">STOP</button>

<div id="msg">Ready</div>

<script>

function sv(s,v){
fetch("/servo?s="+s+"&v="+v);
}

function record(){
fetch("/record");
document.getElementById("msg").innerText="Recording Started";
}

function play(){
fetch("/play");
document.getElementById("msg").innerText="Playing Loop";
}

function stop(){
fetch("/stop");
document.getElementById("msg").innerText="Stopped";
}

</script>

</body>
</html>
)====";

void smoothMove(Servo &servo,int &current,int target){
if(current<target) current++;
else if(current>target) current--;
servo.write(current);
}

void handleRoot(){
server.send(200,"text/html",page);
}

void handleServo(){
int s=server.arg("s").toInt();
int v=server.arg("v").toInt();
target[s]=v;
server.send(200,"text/plain","");
}

void handleRecord(){
recordMode=true;
playMode=false;
stepCount=0;
server.send(200,"text/plain","");
}

void handlePlay(){
playMode=true;
recordMode=false;
server.send(200,"text/plain","");
}

void handleStop(){
playMode=false;
recordMode=false;
server.send(200,"text/plain","");
}

void setup(){

WiFi.softAP(ssid,password);

s1.attach(13);
s2.attach(12);
s3.attach(14);
s4.attach(27);

server.on("/",handleRoot);
server.on("/servo",handleServo);
server.on("/record",handleRecord);
server.on("/play",handlePlay);
server.on("/stop",handleStop);

server.begin();
}

void loop(){

server.handleClient();

smoothMove(s1,pos[0],target[0]);
smoothMove(s2,pos[1],target[1]);
smoothMove(s3,pos[2],target[2]);
smoothMove(s4,pos[3],target[3]);

delay(20);

/* RECORD */

if(recordMode && millis()-lastRecord>400 && stepCount<300){

steps[stepCount]={target[0],target[1],target[2],target[3]};
stepCount++;

lastRecord=millis();
}

/* PLAY */

if(playMode && stepCount>0){

for(int i=0;i<stepCount;i++){

if(!playMode) break;

target[0]=steps[i].a;
target[1]=steps[i].b;
target[2]=steps[i].c;
target[3]=steps[i].d;

while(
pos[0]!=target[0] ||
pos[1]!=target[1] ||
pos[2]!=target[2] ||
pos[3]!=target[3]
){

smoothMove(s1,pos[0],target[0]);
smoothMove(s2,pos[1],target[1]);
smoothMove(s3,pos[2],target[2]);
smoothMove(s4,pos[3],target[3]);

delay(25);
server.handleClient();
}

delay(500);
}

}

}
