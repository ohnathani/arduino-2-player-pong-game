#include <U8g2lib.h>

// SH1106 128x64, pins at top → upright
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Buttons (1 per player)
#define P1_BTN 2
#define P2_BTN 3

// Paddle
const int PADDLE_W = 2;
const int PADDLE_H = 12;
int paddle1Y = 26;
int paddle2Y = 26;

// Paddle direction (-1 up, 1 down)
int dir1 = 1;
int dir2 = 1;

// Ball
int ballX = 60;
int ballY = 30;
int ballDX = 1;
int ballDY = 1;

// Timing
unsigned long lastMove = 0;
int ballInterval = 25; // adjustable for acceleration
const int PADDLE_SPEED = 2;

// Score
int score1 = 0;
int score2 = 0;

// Flash
bool flash = false;
unsigned long flashStart = 0;
const int FLASH_DURATION = 3; // ms

void setup() {
  pinMode(P1_BTN, INPUT_PULLUP);
  pinMode(P2_BTN, INPUT_PULLUP);
  u8g2.begin();
  resetBall();
}

void loop() {
  handleInput();
  updateBall();
  movePaddles();

  // Turn off flash after duration
  if (flash && millis() - flashStart > FLASH_DURATION) flash = false;

  drawFrame();
}

void handleInput() {
  static bool p1Last = HIGH;
  static bool p2Last = HIGH;

  // Player 1 button toggle
  bool p1Now = digitalRead(P1_BTN);
  if (p1Last == HIGH && p1Now == LOW) dir1 = -dir1;
  p1Last = p1Now;

  // Player 2 button toggle
  bool p2Now = digitalRead(P2_BTN);
  if (p2Last == HIGH && p2Now == LOW) dir2 = -dir2;
  p2Last = p2Now;
}

void movePaddles() {
  paddle1Y += dir1 * PADDLE_SPEED;
  paddle2Y += dir2 * PADDLE_SPEED;

  if (paddle1Y < 0) { paddle1Y = 0; dir1 = 1; }
  if (paddle1Y + PADDLE_H > 64) { paddle1Y = 64 - PADDLE_H; dir1 = -1; }
  if (paddle2Y < 0) { paddle2Y = 0; dir2 = 1; }
  if (paddle2Y + PADDLE_H > 64) { paddle2Y = 64 - PADDLE_H; dir2 = -1; }
}

void updateBall() {
  if (millis() - lastMove < ballInterval) return;
  lastMove = millis();

  ballX += ballDX;
  ballY += ballDY;

  // Bounce top/bottom
  if (ballY <= 0 || ballY >= 63) ballDY = -ballDY;

  // Left paddle
  if (ballX <= PADDLE_W) {
    if (ballY >= paddle1Y && ballY <= paddle1Y + PADDLE_H) {
      ballDX = -ballDX;
      accelerateBall();
    } else {
      score2++;
      triggerFlash();
      resetBall();
    }
  }

  // Right paddle
  if (ballX >= 127 - PADDLE_W) {
    if (ballY >= paddle2Y && ballY <= paddle2Y + PADDLE_H) {
      ballDX = -ballDX;
      accelerateBall();
    } else {
      score1++;
      triggerFlash();
      resetBall();
    }
  }
}

void accelerateBall() {
  if (ballInterval > 10) ballInterval--;
}

void resetBall() {
  ballX = 64;
  ballY = 32;
  ballDX = (random(0, 2) == 0 ? 1 : -1);
  ballDY = (random(0, 2) == 0 ? 1 : -1);
  ballInterval = 25;
}

void triggerFlash() {
  flash = true;
  flashStart = millis();

  u8g2.firstPage();
  do {
    u8g2.setDrawColor(1);          // white
    u8g2.drawBox(0, 0, 128, 64);   // full screen
  } while (u8g2.nextPage());

  delay(FLASH_DURATION);
  flash = false;
}

void drawFrame() {
  u8g2.firstPage();
  do {
    if (flash) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, 0, 128, 64);
      return;
    }

    // Middle dashed line
    for (int y = 0; y < 64; y += 4) u8g2.drawPixel(64, y);

    // Paddles
    u8g2.drawBox(0, paddle1Y, PADDLE_W, PADDLE_H);
    u8g2.drawBox(127 - PADDLE_W, paddle2Y, PADDLE_W, PADDLE_H);

    // Ball
    u8g2.drawBox(ballX, ballY, 2, 2);

    // Score
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(50, 10); u8g2.print(score1);
    u8g2.setCursor(75, 10); u8g2.print(score2);

  } while (u8g2.nextPage());
}
