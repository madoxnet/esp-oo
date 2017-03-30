const char GUIHTML[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
  <head>
    <title>Remote Control</title>
    <meta name="viewport"
          content="width=device-width, initial-scale=1,
                   maximum-scale=1, user-scalable=0"/>
    <meta name="mobile-web-app-capable" content="yes">
    <script type="text/javascript" src="config.js"></script>
    <script type="text/javascript">

var left = null;
var right = null;
var origin = {left:{x:0,y:0},right:{x:0,y:0}};
var cmds = [0,0,0,0,0,0,0,0];
var wsurl = "ws://" + window.location.hostname + ":81/";
var docMinSize = 100;
var cmdstr = "";
var touchscale = 3;
var deadband = 10; //10% deadband
var map_outmin = 150;
var map_outmax = 255;

function init(){
  left = document.getElementById("left");
  right = document.getElementById("right");
  setup = document.getElementById("setup");
  config = document.getElementById("config");
  config.style.visibility = "hidden";
  docMinSize = ( document.documentElement.clientWidth < 
                 document.documentElement.clientHeight ?
                 document.documentElement.clientWidth :
                 document.documentElement.clientHeight );
  document.ontouchstart = touchStart;
  document.ontouchend = touchEnd;
  document.ontouchmove = touchMove;
  document.getElementById("wsurl").value = wsurl;
  document.getElementById("ssid1").value = ssid1;
  document.getElementById("pass1").value = pass1;
  document.getElementById("ssid2").value = ssid2;
  document.getElementById("pass2").value = pass2;
  document.getElementById("hostname").value = hostname;
}

function connectWS(){
  wsurl = document.getElementById('wsurl').value;
  var ws = new WebSocket(wsurl);
  ws.onopen = function(){
    setup.style.visibility = "hidden";
    sendID = setInterval(function(){
      cmdstr = "#";
      cmds.forEach(function(cmd){
        cmdstr += ("00" + cmd.toString(16)).slice(-2);
      });
      ws.send(cmdstr);
    }
    ,100);
    aliveID = setTimeout(function(){
      ws.close();
    }
    , 300);
  };

  ws.onmessage = function (evt) 
  { 
    var msg = evt.data;
    if (msg == '_OO_'){
      clearTimeout(aliveID);
    } else {
      alert(evt.data);
    }
  };

  ws.onclose = function()
  { 
    clearInterval(sendID);
    setup.style.visibility = "visible";
  };
}

function constrain(value,rangemin,rangemax)
{
  return ((value < rangemin) ? rangemin : ((value > rangemax) ? rangemax : value));
}

function absmap(value, in_min, in_max, out_min, out_max)
{
  if(value<0){
    return -((-value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);  
  } else {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
}

function touchStart(evt){
  writeText(evt.target.id, "Start");
  cmds = [0,0,0,0,0,0,0,0];
  origin[evt.target.id].y = Math.round(evt.targetTouches[0].clientY);
}

function touchEnd(evt){
  writeText(evt.target.id, "End");
  cmds = [0,0,0,0,0,0,0,0];
}

function touchMove(evt){
  var delta = origin[evt.target.id].y - Math.round(evt.targetTouches[0].clientY);
  var throttle = 100 * (touchscale * delta / docMinSize);
  var command = Math.round(255 * throttle);
  
  if (Math.abs(throttle) < deadband ){
    command = 0;
  } else {
    command = absmap(throttle, deadband, 100, map_outmin, map_outmax);
    command = Math.round(constrain(command, -255,255));
  }
  
  if(evt.touches.length == 2){  
    writeText(evt.target.id, Math.round(throttle) + "% command:" + command);
    
    if(evt.target.id == "left"){
      if (delta >= 0){
        cmds[0] = command;
        cmds[4] = 0;
      } else {
        cmds[0] = 0;
        cmds[4] = -command;
      }
    } else if (evt.target.id == "right"){
      if (delta >= 0){
        cmds[1] = command;
        cmds[5] = 0;
      } else {
        cmds[1] = 0;
        cmds[5] = -command;
      }
    } else {
    }
  } else {
    cmds = [0,0,0,0,0,0,0,0];
  }
}

function writeText(id, text){
  document.getElementById(id).innerHTML = text;
}
    </script>
    <style type="text/css">
body {
  margin: 0px;
  background-color: #000033;
  overflow: hidden;
  font-size: 12pt;
  font-family: Arial;
  text-align: center;
  color: #FFFFFF;
  -webkit-user-select: none;
  -moz-user-select: none;
  -ms-user-select: none;
  user-select: none;
}
button, input, textarea, select, option {
  color: #FFFFFF;
  font-size: 12pt;
  font-family: Arial, Verdana, Helvetica, sans-serif;
  border: solid #003399 1px;
  background-color: #000440;
  overflow: hidden;
}
.box {
  width: 50%;
  height: 100vh;
  display: inline-block;
  box-sizing: border-box;
  border: 2px ridge #003399;
}
.centre {
  width: 100%;
  height: 100%;
  display: flex;
  justify-content: center; 
  align-items: center;
}
#setup {
  width: 100%;
  height: 100%;
  background-color: #000033;
  position: absolute;
  top: 0px;
  left: 0px;
  z-index:999;
  border:none;
}
#config {
  width: 100%;
  height: 100%;
  background-color: #000033;
  position: absolute;
  top: 0px;
  left: 0px;
  z-index:9999;
  border:none;
}
    </style>
  </head>
  <body onload="init();">
    <div class="box"><div id="left" class="centre">Left</div></div><div class="box"><div id="right" class="centre">Right</div></div>
    <div id="setup">
      <div class="centre">
        <div>
          <h1>ESP-OO Connection</h1>
          <input  id="wsurl" type="text" size="30" value=""/><br><br>
          <!---
          <button onclick="document.getElementById('wsurl').value = 'ws://192.168.1.130:81/';">Dummy URL</button><br>
          --->
          <button onclick="connectWS();" style="width: 300px; height: 100px;">Connect</button>
          <br><br><br>
          <button onclick="config.style.visibility = 'visible';">Config</button>
        <div>
      </div>
    </div>
    <div id="config">
      <div class="centre">
        <div>
          <h1>Optional Configuration</h1>
          <form action="configwifi" method="get">
            <h2>Wi-Fi</h2>
            HOSTNAME : <input type="text"     name="hostname" id="hostname" size="31" value=""/><br>
            SSID1    : <input type="text"     name="ssid1" id="ssid1" size="31" value=""/><br>
            PSK1     : <input type="password" name="pass1" id="pass1" size="31" value=""/><br>
            SSID2    : <input type="text"     name="ssid2" id="ssid2" size="31" value=""/><br>
            PSK2     : <input type="password" name="pass2" id="pass2" size="31" value=""/><br>
            <h2>Control</h2>
            ... TODO ...
            <br><br>
            <input type="submit" value="Submit">
          </form>      
          <br><br>
          <button onclick="config.style.visibility = 'hidden';">Cancel</button>
        <div>
      </div>
    </div>
  </body>
</html>
)=====";
