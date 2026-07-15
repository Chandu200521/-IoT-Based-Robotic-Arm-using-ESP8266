#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <vector>
#include <string>
#include <sstream>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

struct ServoPins {
  uint8_t channel;
  String servoName;
  int initialPosition;
};

std::vector<ServoPins> servoPins = {
  {0, "Base", 90},
  {1, "Shoulder", 90},
  {2, "Elbow", 90},
  {3, "Gripper", 90},
  {4, "Wrist", 90},
  {5, "Rotator", 90}
};

struct RecordedStep {
  std::vector<int> values;
  int delayInStep;
};

std::vector<RecordedStep> recordedSteps;
bool recordSteps = false;
bool playRecordedSteps = false;
bool loopPlayback = false;
unsigned long previousTimeInMilli = millis();

const char* ssid = "RobotArm";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsRobotArmInput("/RobotArmInput");

int angleToPulse(int angle) {
  return map(angle, 0, 180, 102, 512);
}

const char* htmlHomePage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>6 DOF Robot Arm</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #f0f0f0;
      padding: 10px;
    }
    .servo-container {
      background: #fff;
      margin: 10px auto;
      padding: 12px;
      border-radius: 8px;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
      max-width: 400px;
    }
    .label {
      font-size: 16px;
      margin-bottom: 5px;
    }
    .slider {
      width: 100%;
    }
    .deg-display {
      font-size: 16px;
      margin-left: 10px;
    }
    .button {
      margin: 6px;
      padding: 8px 16px;
      font-size: 14px;
      background: #007BFF;
      color: white;
      border: none;
      border-radius: 4px;
    }
    .button:hover {
      background: #0056b3;
    }
  </style>
</head>
<body>
  <h2>Robotic Arm Controller</h2>
  <div id="servoControls"></div>
  <button class="button" onclick="sendData('Record',1)">Start Recording</button>
  <button class="button" onclick="sendData('Record',0)">Stop Recording</button>
  <button class="button" onclick="sendData('Play',1)">Play Once</button>
  <button class="button" onclick="sendData('Loop',1)">Play Loop</button>
  <button class="button" onclick="sendData('Loop',0)">Stop Loop</button>

  <script>
    const servoNames = ["Base", "Shoulder", "Elbow", "Gripper", "Wrist", "Rotator"];
    let gateway = `ws://${window.location.hostname}/RobotArmInput`;
    let websocket;

    function createSliders() {
      let html = "";
      servoNames.forEach(name => {
        html += `
        <div class="servo-container">
          <div class="label">
            ${name}: <span id="val_${name}" class="deg-display">90&deg;</span>
          </div>
          <input type="range" min="0" max="180" value="90" id="slider_${name}" class="slider">
        </div>`;
      });
      document.getElementById("servoControls").innerHTML = html;
    }

    function sendData(key, value) {
      if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(`${key},${value}`);
      }
    }

    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onmessage = (event) => {
        let [key, val] = event.data.split(",");
        document.getElementById("slider_" + key).value = val;
        document.getElementById("val_" + key).innerText = val + "°";
      };
    }

    window.onload = () => {
      createSliders();
      initWebSocket();
      servoNames.forEach(name => {
        document.getElementById("slider_" + name).oninput = (e) => {
          let val = e.target.value;
          document.getElementById("val_" + name).innerText = val + "°";
          sendData(name, val);
        };
      });
    };
  </script>
</body>
</html>
)rawliteral;

void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "File Not Found");
}

void onRobotArmInputWebSocketEvent(AsyncWebSocket *server,
                                   AsyncWebSocketClient *client,
                                   AwsEventType type,
                                   void *arg,
                                   uint8_t *data,
                                   size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("Client %u connected\n", client->id());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("Client %u disconnected\n", client->id());
      break;
    case WS_EVT_DATA: {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        std::string msg((char *)data, len);
        std::istringstream ss(msg);
        std::string key, val;
        std::getline(ss, key, ',');
        std::getline(ss, val, ',');
        int value = atoi(val.c_str());

        if (key == "Record") {
          recordSteps = value;
          if (recordSteps) {
            recordedSteps.clear();
            previousTimeInMilli = millis();
          }
        } else if (key == "Play") {
          playRecordedSteps = value;
          loopPlayback = false;
        } else if (key == "Loop") {
          loopPlayback = value;
          playRecordedSteps = value;
        } else {
          std::vector<int> currentValues;
          for (int i = 0; i < servoPins.size(); i++) {
            if (servoPins[i].servoName.equals(key.c_str())) {
              writeServoValues(i, value);
            }
            currentValues.push_back(value);
          }

          if (recordSteps) {
            unsigned long now = millis();
            RecordedStep step = {currentValues, now - previousTimeInMilli};
            previousTimeInMilli = now;
            recordedSteps.push_back(step);
          }
        }
      }
      break;
    }
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void writeServoValues(int index, int value) {
  pwm.setPWM(servoPins[index].channel, 0, angleToPulse(value));
}

void playRecordedRobotArmSteps() {
  if (recordedSteps.empty()) return;
  do {
    for (auto &step : recordedSteps) {
      for (int i = 0; i < step.values.size(); i++) {
        writeServoValues(i, step.values[i]);
        wsRobotArmInput.textAll(servoPins[i].servoName + "," + String(step.values[i]));
      }
      delay(step.delayInStep);
    }
  } while (loopPlayback);
  playRecordedSteps = false;
}

void setUpPinModes() {
  pwm.begin();
  pwm.setPWMFreq(50);
  delay(10);
  for (auto &s : servoPins) {
    pwm.setPWM(s.channel, 0, angleToPulse(s.initialPosition));
  }
}

void setup() {
  Serial.begin(115200);
  setUpPinModes();
  WiFi.softAP(ssid, password);
  Serial.print("WiFi AP IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  wsRobotArmInput.onEvent(onRobotArmInputWebSocketEvent);
  server.addHandler(&wsRobotArmInput);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  wsRobotArmInput.cleanupClients();
  if (playRecordedSteps) {
    playRecordedRobotArmSteps();
  }
}