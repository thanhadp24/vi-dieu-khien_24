#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>

// Thông tin WiFi
const char* ssid = "Quang Minh";    // Tên WiFi
const char* password = "khongcanbiet";           // Mật khẩu WiFi

// Khởi tạo server và Servo
ESP8266WebServer server(80);        
Servo myServo;                      
int servoPin = D4;                  
int currentPos = 90;                // Vị trí mặc định của Servo
int trigPin = D1; // Chân Trig
int echoPin = D2; // Chân Echo

long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // Đổi thời gian sang khoảng cách (cm)
}

// Giao diện HTML
void handleRoot() {
  String HTML = "<!DOCTYPE html>\
  <html lang='vi'>\
  <head>\
  <meta charset='UTF-8'>\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
  <title>Điều khiển Servo & Cảm biến khoảng cách</title>\
  <style>\
  body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f8ff; margin: 0; padding: 0; }\
  h1 { color: #333; margin-top: 20px; }\
  .status { font-size: 20px; color: #007bff; margin-bottom: 20px; }\
  .input { margin: 20px 0; font-size: 18px; padding: 10px; width: 80px; text-align: center; }\
  .btn { display: inline-block; padding: 10px 20px; font-size: 18px; color: white; background-color: #007bff; border: none; border-radius: 5px; cursor: pointer; }\
  .btn:hover { background-color: #0056b3; }\
  </style>\
  <script>\
  function sendAngle() {\
    var angle = document.getElementById('angleInput').value;\
    if (angle >= 0 && angle <= 180) {\
      var xhttp = new XMLHttpRequest();\
      xhttp.onreadystatechange = function() {\
        if (this.readyState == 4 && this.status == 200) {\
          document.getElementById('status').innerHTML = 'Vị trí hiện tại của Servo: ' + angle + '°';\
        }\
      };\
      xhttp.open('GET', '/servo?angle=' + angle, true);\
      xhttp.send();\
    } else {\
      alert('Vui lòng nhập góc từ 0 đến 180.');\
    }\
  }\
  function updateDistance() {\
    var xhttp = new XMLHttpRequest();\
    xhttp.onreadystatechange = function() {\
      if (this.readyState == 4 && this.status == 200) {\
        document.getElementById('distanceValue').innerHTML = 'Khoảng cách: ' + this.responseText + ' cm';\
      }\
    };\
    xhttp.open('GET', '/distance', true);\
    xhttp.send();\
  }\
  setInterval(updateDistance, 1000); // Cập nhật khoảng cách mỗi giây\
  </script>\
  </head>\
  <body>\
  <h1>Điều khiển Servo & Cảm biến khoảng cách</h1>\
  <div class='status' id='status'>Vị trí hiện tại của Servo: " + String(currentPos) + "°</div>\
  <input type='number' id='angleInput' class='input' min='0' max='180' placeholder='Nhập góc'>\
  <button class='btn' onclick='sendAngle()'>Đặt góc</button>\
  <div class='status' id='distanceValue'>Khoảng cách: Đang tải...</div>\
  </body>\
  </html>";

  server.send(200, "text/html", HTML);
}


// Điều khiển Servo qua tham số góc
void handleServoControl() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    if (angle >= 0 && angle <= 180) {
      myServo.write(angle);           
      currentPos = angle;             
      Serial.println("Servo moved to " + String(currentPos) + "°");
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Góc không hợp lệ. Vui lòng nhập góc từ 0 đến 180.");
    }
  } else {
    server.send(400, "text/plain", "Yêu cầu không hợp lệ. Cần có tham số 'angle'.");
  }
}

void handleDistanceSensor() {
  long distance = measureDistance();
  server.send(200, "text/plain", String(distance));
}


void handleNotFound() {
  server.send(404, "text/plain", "Không tìm thấy trang yêu cầu.");
}

void setup() {
  myServo.attach(servoPin, 500, 2500);
  myServo.write(currentPos);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);              
  Serial.println("Servo Control & MeasureDistance");

  // Kết nối WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Đang kết nối WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());

  // Khởi tạo dịch vụ MDNS
  if (MDNS.begin("servoESP8266")) {
    Serial.println("MDNS responder started");
    Serial.println("Truy cập http://servoESP8266 để điều khiển Servo");
  }

  server.on("/", handleRoot);                 // Giao diện chính
  server.on("/servo", HTTP_GET, handleServoControl); // Điều khiển Servo
  server.on("/distance", HTTP_GET, handleDistanceSensor); //Giá trị cảm biến siêu âm
  server.onNotFound(handleNotFound);          // Xử lý trang lỗi
  server.begin();                             
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); 
  MDNS.update();        
}