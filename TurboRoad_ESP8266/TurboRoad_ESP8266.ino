#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// =====================================================
// OLED AYARLARI
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA D2
#define OLED_SCL D1

#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// WIFI
// =====================================================

const char* AP_NAME = "TURBO-ROAD";
const char* AP_PASSWORD = "12345678";

ESP8266WebServer server(80);

// =====================================================
// OYUN AYARLARI
// =====================================================

const int HUD_HEIGHT = 10;

const int ROAD_LEFT = 16;
const int ROAD_RIGHT = 111;

const int CAR_WIDTH = 11;
const int CAR_HEIGHT = 14;

const int PLAYER_Y = 48;

// 3 şerit
const int laneX[3] = {
  27,
  59,
  91
};

int playerLane = 1;
float playerX = 59.0;

// =====================================================
// RAKİP ARAÇ
// =====================================================

struct Enemy {
  int lane;
  float y;
  byte style;
};

Enemy enemies[3];

// =====================================================
// OYUN DURUMU
// =====================================================

int score = 0;
int highScore = 0;
int level = 1;

bool gameOver = false;

unsigned long lastFrame = 0;
const unsigned long FRAME_TIME = 33;

int roadOffset = 0;

// =====================================================
// TELEFON WEB ARAYÜZÜ
// =====================================================

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport"
content="width=device-width,initial-scale=1.0,maximum-scale=1.0,user-scalable=no">

<title>Turbo Road</title>

<style>
* {
  box-sizing: border-box;
  -webkit-user-select: none;
  user-select: none;
  -webkit-tap-highlight-color: transparent;
}

html, body {
  margin: 0;
  width: 100%;
  height: 100%;
  overflow: hidden;
  background: #02060b;
}

body {
  font-family: Arial, Helvetica, sans-serif;
  color: white;
  display: flex;
  justify-content: center;
}

.app {
  width: 100%;
  max-width: 460px;
  height: 100%;
  display: flex;
  flex-direction: column;
  padding: 22px 18px 28px;
  background:
    radial-gradient(
      circle at 50% 15%,
      #174a6d 0%,
      #081827 38%,
      #02060b 78%
    );
}

.header {
  width: 100%;
  text-align: center;
  flex-shrink: 0;
}

.logo {
  font-size: 29px;
  font-weight: 900;
  letter-spacing: 4px;
  color: #ffffff;
  text-shadow:
    0 0 7px #00d9ff,
    0 0 18px rgba(0,217,255,.8);
}

.subtitle {
  margin-top: 6px;
  font-size: 10px;
  letter-spacing: 4px;
  color: #78eaff;
}

.panel {
  width: 100%;
  margin-top: 20px;
  padding: 15px;
  border-radius: 20px;
  border: 1px solid rgba(38,225,255,.30);
  background: rgba(4,19,32,.82);
  box-shadow:
    0 12px 35px rgba(0,0,0,.35),
    0 0 22px rgba(0,200,255,.07);
}

.stats {
  width: 100%;
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 8px;
}

.stat {
  min-width: 0;
  height: 66px;
  border-radius: 13px;
  background: rgba(255,255,255,.055);
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
}

.stat-label {
  font-size: 9px;
  letter-spacing: 1.4px;
  color: #77bad1;
}

.stat-value {
  margin-top: 4px;
  font-size: 23px;
  line-height: 24px;
  font-weight: 800;
  color: #ffffff;
}

.connection {
  margin-top: 13px;
  min-height: 20px;
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 7px;
  font-size: 11px;
  letter-spacing: .8px;
  color: #8ce9ff;
}

.dot {
  width: 7px;
  height: 7px;
  border-radius: 50%;
  background: #35ff8a;
  box-shadow: 0 0 8px #35ff8a;
}

.restart {
  width: 100%;
  height: 43px;
  margin-top: 12px;
  border: 0;
  border-radius: 12px;
  background: linear-gradient(135deg,#137da6,#07415d);
  color: white;
  font-size: 13px;
  font-weight: 700;
  letter-spacing: 1px;
  box-shadow: inset 0 1px 0 rgba(255,255,255,.15);
}

.restart:active {
  transform: scale(.985);
  background: linear-gradient(135deg,#1598c7,#075474);
}

.controller-area {
  width: 100%;
  margin-top: auto;
  padding-top: 25px;
}

.controller {
  width: 100%;
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 24px;
  align-items: start;
}

.control-side {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.control {
  width: 118px;
  height: 118px;
  padding: 0;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  border: 3px solid #26e1ff;
  outline: none;
  background:
    radial-gradient(
      circle at 50% 35%,
      #17658b 0%,
      #0b334c 38%,
      #04131f 78%
    );
  color: #ffffff;
  font-size: 52px;
  line-height: 1;
  box-shadow:
    0 0 17px rgba(0,215,255,.48),
    inset 0 0 24px rgba(0,215,255,.16),
    inset 0 2px 2px rgba(255,255,255,.12);
  transition: transform .07s ease, box-shadow .07s ease;
}

.control.active {
  transform: scale(.91);
  box-shadow:
    0 0 28px rgba(0,225,255,.85),
    inset 0 0 30px rgba(0,215,255,.30);
}

.control-label {
  margin-top: 11px;
  font-size: 10px;
  letter-spacing: 2px;
  color: #6db4cb;
}

.footer {
  margin-top: 18px;
  text-align: center;
  color: #496c7a;
  font-size: 9px;
  letter-spacing: 1px;
}

@media (max-height: 680px) {
  .app {
    padding-top: 12px;
    padding-bottom: 14px;
  }

  .logo {
    font-size: 24px;
  }

  .panel {
    margin-top: 12px;
    padding: 11px;
  }

  .stat {
    height: 54px;
  }

  .control {
    width: 96px;
    height: 96px;
    font-size: 45px;
  }

  .controller-area {
    padding-top: 12px;
  }
}
</style>
</head>

<body>
<div class="app">

  <div class="header">
    <div class="logo">TURBO ROAD</div>
    <div class="subtitle">ESP8266 EDITION</div>
  </div>

  <div class="panel">
    <div class="stats">

      <div class="stat">
        <div class="stat-label">SCORE</div>
        <div class="stat-value" id="score">0</div>
      </div>

      <div class="stat">
        <div class="stat-label">LEVEL</div>
        <div class="stat-value" id="level">1</div>
      </div>

      <div class="stat">
        <div class="stat-label">BEST</div>
        <div class="stat-value" id="high">0</div>
      </div>

    </div>

    <div class="connection" id="status">
      <span class="dot"></span>
      NODEMCU BAGLI
    </div>

    <button class="restart" onclick="restartGame()">
      YENIDEN BASLAT
    </button>
  </div>

  <div class="controller-area">
    <div class="controller">

      <div class="control-side">
        <button id="left" class="control">&#10094;</button>
        <div class="control-label">SOL</div>
      </div>

      <div class="control-side">
        <button id="right" class="control">&#10095;</button>
        <div class="control-label">SAG</div>
      </div>

    </div>

    <div class="footer">
      OLED EKRANA BAKARAK OYNA
    </div>
  </div>

</div>

<script>
let holdTimer = null;
let repeatTimer = null;

const leftButton = document.getElementById("left");
const rightButton = document.getElementById("right");

function sendCommand(command) {
  fetch("/" + command).catch(function() {});
}

function pressButton(button, direction) {
  button.classList.add("active");
  sendCommand(direction);

  clearTimeout(holdTimer);
  clearInterval(repeatTimer);

  holdTimer = setTimeout(function() {
    repeatTimer = setInterval(function() {
      sendCommand(direction);
    }, 230);
  }, 400);
}

function releaseButton(button) {
  button.classList.remove("active");
  clearTimeout(holdTimer);
  clearInterval(repeatTimer);
}

leftButton.addEventListener("pointerdown", function(event) {
  event.preventDefault();
  pressButton(leftButton, "left");
});

rightButton.addEventListener("pointerdown", function(event) {
  event.preventDefault();
  pressButton(rightButton, "right");
});

["pointerup","pointercancel","pointerleave"].forEach(function(eventName) {
  leftButton.addEventListener(eventName, function() {
    releaseButton(leftButton);
  });

  rightButton.addEventListener(eventName, function() {
    releaseButton(rightButton);
  });
});

function restartGame() {
  fetch("/restart")
  .then(function() {
    updateStatus();
  })
  .catch(function() {});
}

function updateStatus() {
  fetch("/status")
  .then(function(response) {
    return response.json();
  })
  .then(function(data) {
    document.getElementById("score").innerText = data.score;
    document.getElementById("level").innerText = data.level;
    document.getElementById("high").innerText = data.high;

    const status = document.getElementById("status");

    if (data.over) {
      status.innerHTML = "GAME OVER";
    } else {
      status.innerHTML = '<span class="dot"></span> NODEMCU BAGLI';
    }
  })
  .catch(function() {
    document.getElementById("status").innerHTML = "BAGLANTI BEKLENIYOR";
  });
}

setInterval(updateStatus, 500);
updateStatus();
</script>

</body>
</html>
)rawliteral";

// =====================================================
// YENİ RAKİP OLUŞTUR
// =====================================================

void spawnEnemy(Enemy &enemy, float startY) {
  enemy.lane = random(0, 3);
  enemy.y = startY;
  enemy.style = random(0, 3);
}

// =====================================================
// OYUNU SIFIRLA
// =====================================================

void resetGame() {
  score = 0;
  level = 1;
  gameOver = false;

  playerLane = 1;
  playerX = laneX[playerLane];

  roadOffset = 0;

  spawnEnemy(enemies[0], -15);
  spawnEnemy(enemies[1], -48);
  spawnEnemy(enemies[2], -82);
}

// =====================================================
// OYUNCU ARABASI
// =====================================================

void drawPlayerCar(int x, int y) {
  display.fillRect(x, y + 3, 2, 4, SH110X_WHITE);
  display.fillRect(x, y + 10, 2, 3, SH110X_WHITE);

  display.fillRect(x + 9, y + 3, 2, 4, SH110X_WHITE);
  display.fillRect(x + 9, y + 10, 2, 3, SH110X_WHITE);

  display.fillRoundRect(x + 2, y, 7, 14, 2, SH110X_WHITE);
  display.fillRect(x, y + 5, 11, 6, SH110X_WHITE);

  display.fillRect(x + 3, y + 2, 5, 3, SH110X_BLACK);
  display.fillRect(x + 3, y + 9, 5, 2, SH110X_BLACK);

  display.drawPixel(x + 5, y, SH110X_BLACK);
  display.drawPixel(x + 3, y + 12, SH110X_BLACK);
  display.drawPixel(x + 7, y + 12, SH110X_BLACK);
}

// =====================================================
// RAKİP ARABA
// =====================================================

void drawEnemyCar(int x, int y, byte style) {
  if (style == 0) {
    display.fillRoundRect(x + 2, y, 7, 14, 2, SH110X_WHITE);
    display.fillRect(x, y + 4, 11, 7, SH110X_WHITE);
    display.fillRect(x + 3, y + 6, 5, 3, SH110X_BLACK);
    display.drawPixel(x + 2, y + 12, SH110X_BLACK);
    display.drawPixel(x + 8, y + 12, SH110X_BLACK);
  }
  else if (style == 1) {
    display.drawRoundRect(x + 2, y, 7, 14, 2, SH110X_WHITE);
    display.fillRect(x, y + 3, 11, 8, SH110X_WHITE);
    display.fillRect(x + 3, y + 5, 5, 3, SH110X_BLACK);
    display.drawFastHLine(x + 3, y + 12, 5, SH110X_WHITE);
  }
  else {
    display.fillRect(x + 2, y, 7, 14, SH110X_WHITE);
    display.fillRect(x, y + 4, 11, 6, SH110X_WHITE);
    display.fillRect(x + 3, y + 1, 5, 3, SH110X_BLACK);
    display.fillRect(x + 3, y + 9, 5, 2, SH110X_BLACK);
  }
}

// =====================================================
// HUD
// =====================================================

void drawHUD() {
  display.fillRect(0, 0, 128, HUD_HEIGHT, SH110X_BLACK);
  display.drawLine(0, 9, 127, 9, SH110X_WHITE);

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(2, 1);
  display.print("S:");
  display.print(score);

  display.setCursor(91, 1);
  display.print("L:");
  display.print(level);
}

// =====================================================
// YOL
// =====================================================

void drawRoad() {
  display.drawLine(
    ROAD_LEFT,
    HUD_HEIGHT,
    ROAD_LEFT,
    63,
    SH110X_WHITE
  );

  display.drawLine(
    ROAD_RIGHT,
    HUD_HEIGHT,
    ROAD_RIGHT,
    63,
    SH110X_WHITE
  );

  for (
    int y = HUD_HEIGHT - 8 + roadOffset;
    y < 64;
    y += 11
  ) {
    display.drawLine(
      ROAD_LEFT - 3,
      y,
      ROAD_LEFT - 1,
      y + 4,
      SH110X_WHITE
    );

    display.drawLine(
      ROAD_RIGHT + 1,
      y,
      ROAD_RIGHT + 3,
      y + 4,
      SH110X_WHITE
    );
  }

  for (
    int y = HUD_HEIGHT - 14 + roadOffset;
    y < 64;
    y += 17
  ) {
    display.drawFastVLine(
      48,
      y,
      7,
      SH110X_WHITE
    );
  }

  for (
    int y = HUD_HEIGHT - 14 + roadOffset;
    y < 64;
    y += 17
  ) {
    display.drawFastVLine(
      80,
      y,
      7,
      SH110X_WHITE
    );
  }
}

// =====================================================
// ÇARPIŞMA
// =====================================================

bool collision(Enemy &enemy) {
  int playerDrawX = (int)playerX;
  int enemyX = laneX[enemy.lane];
  int enemyY = (int)enemy.y;

  return (
    playerDrawX + 2 < enemyX + CAR_WIDTH - 2 &&
    playerDrawX + CAR_WIDTH - 2 > enemyX + 2 &&
    PLAYER_Y + 2 < enemyY + CAR_HEIGHT - 2 &&
    PLAYER_Y + CAR_HEIGHT - 2 > enemyY + 2
  );
}

// =====================================================
// GAME OVER EKRANI
// =====================================================

void drawGameOver() {
  display.clearDisplay();

  display.drawRoundRect(
    5,
    5,
    118,
    54,
    5,
    SH110X_WHITE
  );

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 12);
  display.print("GAME OVER");

  display.setTextSize(1);

  display.setCursor(25, 37);
  display.print("SCORE: ");
  display.print(score);

  display.setCursor(25, 49);
  display.print("BEST : ");
  display.print(highScore);

  display.display();
}

// =====================================================
// OYUN EKRANI
// =====================================================

void drawGame() {
  display.clearDisplay();

  drawRoad();

  for (int i = 0; i < 3; i++) {
    drawEnemyCar(
      laneX[enemies[i].lane],
      (int)enemies[i].y,
      enemies[i].style
    );
  }

  drawPlayerCar(
    (int)playerX,
    PLAYER_Y
  );

  drawHUD();

  display.display();
}

// =====================================================
// RAKİP HIZI
// =====================================================

void updateEnemies() {
  level = 1 + (score / 12);

  if (level > 8) {
    level = 8;
  }

  // Yavaş başlar, skor arttıkça kademesiz hızlanır.
  float speedValue =
    1.25 +
    (score * 0.025);

  if (speedValue > 2.90) {
    speedValue = 2.90;
  }

  for (int i = 0; i < 3; i++) {
    enemies[i].y += speedValue;

    if (
      enemies[i].y >
      SCREEN_HEIGHT + 5
    ) {
      score++;

      enemies[i].lane = random(0, 3);
      enemies[i].style = random(0, 3);
      enemies[i].y = -random(25, 55);
    }
  }
}

// =====================================================
// OYUNCU YUMUŞAK ŞERİT DEĞİŞTİRME
// =====================================================

void updatePlayer() {
  float targetX = laneX[playerLane];
  float difference = targetX - playerX;

  playerX += difference * 0.30;

  if (
    difference > -0.30 &&
    difference < 0.30
  ) {
    playerX = targetX;
  }
}

// =====================================================
// OYUN MOTORU
// =====================================================

void updateGame() {
  if (gameOver) {
    return;
  }

  if (
    millis() - lastFrame <
    FRAME_TIME
  ) {
    return;
  }

  lastFrame = millis();

  int roadSpeed = 2;

  if (score >= 15) {
    roadSpeed = 3;
  }

  if (score >= 40) {
    roadSpeed = 4;
  }

  roadOffset += roadSpeed;

  if (roadOffset >= 17) {
    roadOffset = 0;
  }

  updatePlayer();
  updateEnemies();

  for (int i = 0; i < 3; i++) {
    if (collision(enemies[i])) {
      gameOver = true;

      if (score > highScore) {
        highScore = score;
      }

      drawGameOver();
      return;
    }
  }

  drawGame();
}

// =====================================================
// SOL
// =====================================================

void moveLeft() {
  if (
    !gameOver &&
    playerLane > 0
  ) {
    playerLane--;
  }

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

// =====================================================
// SAĞ
// =====================================================

void moveRight() {
  if (
    !gameOver &&
    playerLane < 2
  ) {
    playerLane++;
  }

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

// =====================================================
// TELEFONA DURUM GÖNDER
// =====================================================

void sendStatus() {
  char json[110];

  snprintf(
    json,
    sizeof(json),
    "{\"score\":%d,\"level\":%d,\"high\":%d,\"over\":%s}",
    score,
    level,
    highScore,
    gameOver ? "true" : "false"
  );

  server.send(
    200,
    "application/json",
    json
  );
}

// =====================================================
// AÇILIŞ EKRANI
// =====================================================

void bootScreen() {
  display.clearDisplay();

  display.drawRoundRect(
    4,
    4,
    120,
    56,
    6,
    SH110X_WHITE
  );

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);

  display.setCursor(33, 10);
  display.print("TURBO");

  display.setCursor(40, 27);
  display.print("ROAD");

  display.setTextSize(1);

  display.setCursor(29, 48);
  display.print("192.168.4.1");

  display.display();
}

// =====================================================
// WIFI BİLGİ EKRANI
// =====================================================

void wifiScreen() {
  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(14, 4);
  display.print("WIFI KUMANDA");

  display.drawLine(
    4,
    15,
    123,
    15,
    SH110X_WHITE
  );

  display.setCursor(4, 21);
  display.print("WiFi:");

  display.setCursor(4, 32);
  display.print("TURBO-ROAD");

  display.setCursor(4, 43);
  display.print("Sifre: 12345678");

  display.setCursor(4, 54);
  display.print("IP: 192.168.4.1");

  display.display();
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  delay(100);

  randomSeed(micros());

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (
    !display.begin(
      OLED_ADDRESS,
      true
    )
  ) {
    Serial.println("OLED bulunamadi!");

    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.display();

  bootScreen();

  delay(1800);

  WiFi.mode(WIFI_AP);

  WiFi.softAP(
    AP_NAME,
    AP_PASSWORD
  );

  Serial.println();
  Serial.println("======================");
  Serial.println("TURBO ROAD HAZIR");

  Serial.print("WiFi: ");
  Serial.println(AP_NAME);

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  wifiScreen();

  delay(2500);

  server.on(
    "/",
    []() {
      server.send_P(
        200,
        "text/html",
        PAGE
      );
    }
  );

  server.on(
    "/left",
    moveLeft
  );

  server.on(
    "/right",
    moveRight
  );

  server.on(
    "/status",
    sendStatus
  );

  server.on(
    "/restart",
    []() {
      resetGame();

      server.send(
        200,
        "text/plain",
        "OK"
      );
    }
  );

  server.begin();

  resetGame();
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  server.handleClient();
  updateGame();
  yield();
}
