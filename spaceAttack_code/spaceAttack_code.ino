#include <U8g2lib.h>
#include <Wire.h>

// OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/21, /* data=*/20, /* reset=*/U8X8_PIN_NONE);

// Potansiyometre ve LDR
const int potansiyometrePin = A0;
const int ldrPin = A1;
// Silah ve Can LED'leri
const int silahPinleri[] = {24, 25, 26};
const int canPinleri[] = {27, 28, 29};
// Butonlar
const int atisButonPin = 51;
const int yukariButonPin = 52;
const int asagiButonPin = 53;
const int secimButonPin = 22;
// Buzzer
const int buzzerPin = 8;
// 7 Segment Bağlantıları
const int segmentPinleri[][7] = {
  {30, 31, 32, 33, 34, 35, 36},
  {37, 38, 39, 40, 41, 42, 43},
  {44, 45, 46, 47, 48, 49, 50}
};


// Can ve dokunulmazlık süresi
const unsigned long dokunulmazlikSuresi = 3000;
unsigned long dokunulmazlikBaslangici;
int can = 3;

struct OyunNesnesi {
  int x, y;
};

OyunNesnesi uzayGemisi = {10, 30};
OyunNesnesi engel[5];
OyunNesnesi mermi[3];
int engelHizi = 1;
bool mermiAktif[3] = {false, false, false};

// Oyun için değişkenler
int puan = 0;
bool oyunAktif = true;

void oyunBaslat() {
  oyunAktif = true;
  puan = 0;
  can = 3;
  dokunulmazlikBaslangici = 0;
  engelHizi = 1;
  uzayGemisi.y = 32;

  for (int i = 0; i < 5; i++) {
    engel[i].x = 120 + i * 20;
    engel[i].y = random(15, 50);
  }
}

void engelleriCiz() {
  for (int i = 0; i < 5; i++) {
    u8g2.drawBox(engel[i].x, 0, 3, engel[i].y);
    u8g2.drawBox(engel[i].x, engel[i].y + 15, 3, 64 - engel[i].y - 15);
  }
}
void mermiCiz() {
  for (int i = 0; i < 3; i++) {
    if (mermiAktif[i]) {
      u8g2.drawBox(mermi[i].x, mermi[i].y, 2, 2);
    }
  }
}

bool carpismaKontrol() {
  for (int i = 0; i < 5; i++) {
    if (abs(uzayGemisi.x - engel[i].x) < 5 && (uzayGemisi.y < engel[i].y || uzayGemisi.y > engel[i].y + 15)) {
      return true;
    }
  }
  return false;
}
void ledleriGuncelle() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(canPinleri[i], i < can);
    digitalWrite(silahPinleri[i], mermiAktif[i]);
  }
}

void oyunEkrani() {
  u8g2.clearBuffer();

  // Uzay gemisi
  u8g2.drawBox(uzayGemisi.x, uzayGemisi.y, 5, 5);
  engelleriCiz();
  mermiCiz();



  u8g2.sendBuffer();
}
bool mermiCarpismaKontrol(int mermiIndeks) {
  for (int i = 0; i < 5; i++) {
    if (abs(mermi[mermiIndeks].x - engel[i].x) < 3 && abs(mermi[mermiIndeks].y - engel[i].y) < 15) {
      engel[i].x = 120 + i * 20;
      engel[i].y = random(15, 50);
      return true;
    }
  }
  return false;
}
void yaz7Segment(int sayi, int segmentIndex) {
  const byte rakamlar[10][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
  };

  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPinleri[segmentIndex][i], rakamlar[sayi][i]);
  }
}

void goster7Segment(int puan) {
  int yuzler = puan / 100;
  int onlar = (puan / 10) % 10;
  int birler = puan % 10;

  yaz7Segment(yuzler, 0);
  delay(5);
  yaz7Segment(onlar, 1);
  delay(5);
  yaz7Segment(birler, 2);
  delay(5);
}



void oyunBittiEkrani() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 20, "Oyun Bitti");
  u8g2.drawStr(0, 40, "Puan:");
  u8g2.setCursor(50, 40);
  u8g2.print(puan);
  u8g2.sendBuffer();
}

void setup() {
  // Butonlar ve LED'ler için pinMode ayarı
  pinMode(atisButonPin, INPUT_PULLUP);
  pinMode(yukariButonPin, INPUT_PULLUP);
  pinMode(asagiButonPin, INPUT_PULLUP);
  pinMode(secimButonPin, INPUT_PULLUP);

  pinMode(potansiyometrePin, INPUT);


  for (int i = 0; i < 3; i++) {
    pinMode(silahPinleri[i], OUTPUT);
    pinMode(canPinleri[i], OUTPUT);
  }

  pinMode(buzzerPin, OUTPUT);

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB10_tr);

oyunBaslat();
}
void loop() {
if (oyunAktif) {
// Uzay gemisini potansiyometre ile hareket ettir
   uzayGemisi.y = map(analogRead(potansiyometrePin), 0, 1023, 0, 59);
   // Engellerin hareketi ve çarpışma kontrolü
for (int i = 0; i < 5; i++) {
  engel[i].x -= engelHizi;

  if (engel[i].x < -3) {
    engel[i].x = 120;
    engel[i].y = random(15, 50);
    puan++;
  }
}
// Butonlar ve işlevleri
  if (digitalRead(atisButonPin) == LOW) {
    for (int i = 0; i < 3; i++) {
      if (!mermiAktif[i]) {
        mermi[i].x = uzayGemisi.x + 5;
        mermi[i].y = uzayGemisi.y + 2;
        mermiAktif[i] = true;
        break;
      }
    }
  }
// Mermi hareketi ve çizimi
 for (int i = 0; i < 3; i++) {
      if (mermiAktif[i]) {
        mermi[i].x += 2;
        if (mermi[i].x > 128) {
          mermiAktif[i] = false;
        } else if (mermiCarpismaKontrol(i)) {
          mermiAktif[i] = false;
          puan++;
        }
      }
    }
    ledleriGuncelle();
    goster7Segment(puan);    
  

if (carpismaKontrol() && millis() - dokunulmazlikBaslangici > dokunulmazlikSuresi) {
  can--;
  if (can <= 0) {
    oyunAktif = false;
  } else {
    dokunulmazlikBaslangici = millis();
  }
}

oyunEkrani();
mermiCiz();
} else {
oyunBittiEkrani();
// Oyunu yeniden başlatma
if (millis() - dokunulmazlikBaslangici > dokunulmazlikSuresi) {
  oyunBaslat();
}
}
}