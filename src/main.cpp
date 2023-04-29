// naam esp: esptetris
// wachtwoord esp: esptetris

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <Wire.h>
#include <ArduinoNunchuk.h>
#include <iostream>
#include <string>
#include <WiFi.h>
#include "OTAlib.h"
#include <PubSubClient.h>

using namespace std;
#define NOCOLOR        matrix.Color888(0, 0, 0)
#define CYAN           matrix.Color888(0, 16, 16)
#define YELLOW         matrix.Color888(16, 16, 0)
#define ORANGE         matrix.Color888(48, 16, 0)
#define BLUE           matrix.Color888(0, 0, 16)
#define RED            matrix.Color888(16, 0, 0)
#define GREEN          matrix.Color888(0, 16, 0)
#define PURPLE         matrix.Color888(16, 0, 16)
#define WHITE          matrix.Color888(16, 16, 16)

#define NOPIECE         0
#define I_ID            1
#define SQUARE_ID       2
#define REVERSED_L_ID   3
#define L_ID            4
#define REVERSED_Z_ID   5
#define Z_ID            6
#define T_ID            7
 
#define SPAWNX          19
#define SPAWNY          9
#define SPAWNROTATION   0
#define UPLIMIT         0
#define LEFTLIMIT       11
#define RIGHTLIMIT      31
#define DOWNLIMIT       63
#define FALLDELAY       200

#define INPUTDELAY      1
#define MAXSPEEDDELAY   50
#define DELETIONDELAY   200
#define TRIGGERMAXSPEED 300
#define PLACEMENTDELAY  400
#define FRAMEDELAY      750
#define GAMEOVERDELAY   1500

#define LEFT            0
#define RIGHT           1
#define DOWN            2
#define HARDDROP        3
#define ROTATE          4
#define OFF             0
#define ON              1
#define XCOORDINATE     0
#define YCOORDINATE     1
 
#define CLK             15
#define OE              33
#define LAT             32
#define A               12
#define B               16
#define C               17
#define D               18

#define buttonLinks     13
#define buttonRechts    19
#define buttondrop      4
#define buttonrotate    2
#define resetbutton     0
#define buttonsnelbeneden 35
int score = 0;
bool bereikt = false;
bool score_changed;

//OTA
OTAlib ota("NETGEAR68", "excitedtuba713");

//MQTT -
#define SSID          "NETGEAR68"
#define PWD           "excitedtuba713"
#define MQTT_SERVER   "192.168.0.190"  
#define MQTT_PORT     1883

topic = "esp_tetris/output";

WiFiClient espClient;
PubSubClient client(espClient);

const char *ssid = "NETGEAR68";
const char *password = "excitedtuba713";

void setup_wifi()
{
  delay(10);
  Serial.println("Connecting to WiFi..");

  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// -MQTT

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);
ArduinoNunchuk controller = ArduinoNunchuk();
unsigned int ledColorMatrix[64][3];
unsigned long long movementTime;
unsigned int color;
char pieceID;
char x, y;
char shadowx;
char shadowy;
char rotationState;
bool button_down_lock;
bool button_left_lock;
bool button_right_lock;
bool button_hardrop_lock;
bool button_rotate_lock;
short basePieceCoordinates[7][4][2] = {
    { {-1, 0}, {0, 0}, {1, 0}, {2, 0} },    // I
    { {0, 0}, {1, 0}, {0, 1}, {1, 1} },     // SQUARE
    { {1, 0}, {-1, 1}, {0, 1}, {1, 1} },    // L
    { {-1, 0}, {-1, 1}, {0, 1}, {1, 1} },   // REVERSED_L
    { {-1, 0}, {0, 0}, {0, 1}, {1, 1} },    // Z
    { {0, 0}, {1, 0}, {-1, 1}, {0, 1} },    // REVERSED_Z
    { {0, 0}, {-1, 1}, {0, 1}, {1, 1} }     // T
};

//Declaration functions:
bool canFall();
bool isPositionAvailable(int x_, int y_);
void getCoordinates(int x_, int y_, int* positionx, int* positiony, int pieceIndex);
bool isAnActualAndTurnedOffPixel(int positionx, int positiony);
void createShadow();
void deleteCurrentPiece();
void deleteShadow();
void drawNewPiece(int x_, int y_, int pieceColor);
void setupNewGame();
void setupNewTurn();
void setupNewPiece();
void clearScreen();
void clearLedColorMatrix();
void addPieceToMatrix();
unsigned int getColorByID(char pieceID);
unsigned long long getRow(char positiony);
unsigned long long turnOnBits(char positionx, char positiony);
unsigned long long turnOffBits(char positionx, char positiony);
char getPieceID(int positionx, int positiony);
bool isLedOn(char positionx, char positiony);
void updateLedColorMatrix(int positionx, int positiony, int pieceID_);
void getInput(int delaytime);
void updatePosition();
void checkForInput(int button);
bool isPressed(int button);
void doAction(int button);
void doAdaptiveMovementTo(int button);
bool isLocked(int button);
void lockButton(int button);
void unlockButton(int button);
void moveTo(int button);
bool move(int positionx, int positiony);
void hardDrop(int positionx, int positiony);
void rotate();
void endGame();
bool isRestartButtonPressed(int delaytime);
void deleteAllPieces();
void createFrame();
void printGameOver();
void deleteFullLines();
bool isLineFull(int positiony);
void deleteLine(int positiony);
void dropLinesFrom(int positiony);
void dropPixel(int positionx, int positiony);
void display_score();
void display_voornaam();
void display_achternaam();
void print_well_done();


struct score_word{
    int score;
    char letter;
    bool bereikt;
};

score_word omzet[] = {
    {50,'H',false},
    {100,'U',false},
    {200,'G',false},
    {300,'O',false},
    {400,'C',false},
    {500,'O',false},
    {600,'O',false},
    {700,'L',false},
    {800,'E',false},
    {900,'N',false},
    {950,'S',false},
};

void setup() {
    // OTA
    ota.setHostname("esptetris");  
    ota.setPassword("esptetris");
    ota.begin();

    //MQTT -
    setup_wifi();
    client.setServer(MQTT_SERVER, MQTT_PORT);
    // - MQTT

    pinMode(buttonLinks, INPUT_PULLUP);
    pinMode(buttonRechts, INPUT_PULLUP); 
    pinMode(buttondrop,INPUT_PULLUP);
    pinMode(buttonrotate,INPUT_PULLUP);
    pinMode(resetbutton,INPUT_PULLUP);
    pinMode(buttonsnelbeneden,INPUT);
    matrix.begin();
    randomSeed(analogRead(34));
    setupNewGame();
}

//MQTT -
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // creat unique client ID
    // in Mosquitto broker enable anom. access
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe(topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }
}
//- MQTT


void loop() {
    //MQTT -
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
    //- MQTT

    while(score != 1000){
        if (score == 0){
            display_score();
        }
        matrix.drawLine(0, 10, 64, 10, WHITE);
        matrix.drawLine(8,32,8,11,WHITE);
        score_changed=false;
        
        getInput(FALLDELAY);
        if (canFall()) move(x, y+1);
        else setupNewTurn();
        if (score_changed){
            display_score();

            if (score >= 400){
                if(score == 400){
                    for (int i = 0; i < 56; i++){
                        for(int j = 0; j<9; j++){
                            matrix.writePixel(i,j,NOCOLOR);
                        }
                    }
                }
                display_achternaam();
            }
            else{display_voornaam();}
        }
    }   
    print_well_done();
    if (isRestartButtonPressed(750)) {
    for (int i = 0; i < 64; i++){
        for(int j = 0; j<32; j++){
            matrix.writePixel(i,j,NOCOLOR);
        }
    }
    setupNewGame();
    return;
    }
}




bool canFall() {
    if (!isPositionAvailable(x, y+1)) {
        getInput(PLACEMENTDELAY);
        if (!isPositionAvailable(x, y+1)) return false;
    }
    return true;
}

bool isPositionAvailable(int x_, int y_) {
    for (int pieceIndex = 0; pieceIndex < 4; ++pieceIndex) {
        int positionx, positiony;
        getCoordinates(x_, y_, &positionx, &positiony, pieceIndex);
        if (!isAnActualAndTurnedOffPixel(positionx, positiony)) return false;
    }
    return true;
}

void getCoordinates(int x_, int y_, int* positionx, int* positiony, int pieceIndex) {
    rotationState %= 4;
    int baseCoordinateX = basePieceCoordinates[pieceID-1][pieceIndex][XCOORDINATE];
    int baseCoordinateY = basePieceCoordinates[pieceID-1][pieceIndex][YCOORDINATE];
    for (int i = 0; i < rotationState; ++i) {
        int tmp = baseCoordinateY;
        baseCoordinateY = baseCoordinateX;
        baseCoordinateX = -tmp;
    }
    *positionx = x_ + baseCoordinateX;
    *positiony = y_ + baseCoordinateY;
}

bool isAnActualAndTurnedOffPixel(int positionx, int positiony) {
    if (positionx < LEFTLIMIT || positionx > RIGHTLIMIT) return false;
    if (positiony > DOWNLIMIT) return false;
    if (isLedOn(positionx, positiony)) return false;
    return true;
}

void createShadow() {
    shadowx = x;
    shadowy = y;
    while (isPositionAvailable(shadowx, ++shadowy));
    // shadowy has to be decreased because we have checked that
    // that position is not available with the while condition
    drawNewPiece(shadowx, --shadowy, WHITE);
}

void deleteCurrentPiece() {
    drawNewPiece(x, y, NOPIECE);
}

void deleteShadow() {
    drawNewPiece(shadowx, shadowy, NOPIECE);
}

void drawNewPiece(int x_, int y_, int pieceColor) {
    for (int pieceIndex = 0; pieceIndex < 4; ++pieceIndex) {
        int positionx, positiony;
        getCoordinates(x_, y_, &positionx, &positiony, pieceIndex);
        matrix.drawPixel(positiony, positionx, pieceColor);
    }
}

void setupNewGame() {
    score = 0;
    if (score == 0){
        matrix.setRotation(3);
        matrix.drawChar(0,0,'0',WHITE,NOCOLOR,1);
        matrix.setRotation(0);
    }
    button_left_lock = true;
    button_right_lock = true;
    button_down_lock = true;
    button_hardrop_lock = true;
 button_rotate_lock = true;
    clearScreen();
    createFrame();
    clearLedColorMatrix();
    setupNewPiece();
    createShadow();
    drawNewPiece(x, y, color);
}

void setupNewTurn() {
    addPieceToMatrix();
    deleteFullLines();
    setupNewPiece();
    if (isPositionAvailable(SPAWNX, SPAWNY)) {
        createShadow();
        drawNewPiece(SPAWNX, SPAWNY, color);
    } else {
        endGame();
    }
}
// hier nog eens kijken dat we niet dezelfde blokken telkens spawnen
void setupNewPiece() {
    x = SPAWNX;
    y = SPAWNY;
    shadowx = SPAWNX;
    shadowy = SPAWNY;
    rotationState = SPAWNROTATION;
    pieceID = (random() % 7) + 1;
    color = getColorByID(pieceID);
}

void clearScreen() {
    for (int positiony = UPLIMIT; positiony <= DOWNLIMIT; ++positiony)
        for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx)
            matrix.drawPixel(positiony, positionx, NOCOLOR);
}

void clearLedColorMatrix() {
    for (int positiony = UPLIMIT; positiony <= DOWNLIMIT; ++positiony) {
        ledColorMatrix[positiony][0] = NOPIECE;
        ledColorMatrix[positiony][1] = NOPIECE;
        ledColorMatrix[positiony][2] = NOPIECE;
    }
}

void addPieceToMatrix() {
    for (int pixelIndex = 0; pixelIndex < 4; ++pixelIndex) {
        int positionx, positiony;
        getCoordinates(x, y, &positionx, &positiony, pixelIndex);
        updateLedColorMatrix(positionx, positiony, pieceID);
    }
}


unsigned int getColorByID(char pieceID) {
    if (pieceID == NOPIECE) return NOCOLOR;
    if (pieceID == I_ID) return CYAN;
    if (pieceID == SQUARE_ID) return YELLOW;
    if (pieceID == REVERSED_L_ID) return ORANGE;
    if (pieceID == L_ID) return BLUE;
    if (pieceID == REVERSED_Z_ID) return RED;
    if (pieceID == Z_ID) return GREEN;
    if (pieceID == T_ID) return PURPLE;
}

unsigned long long getRow(char positiony) {
    unsigned long long byte2 = ((unsigned long long) ledColorMatrix[positiony][1]) << 32;
    unsigned long long byte3 = ledColorMatrix[positiony][2]; 
    return byte2 | byte3;
}

unsigned long long turnOnBits(char positionx, char positiony) {
    return getRow(positiony)  &(0b111LL << ((RIGHTLIMIT - positionx)*3));
}

unsigned long long turnOffBits(char positionx, char positiony) {
    return getRow(positiony) & ~(0b111LL << ((RIGHTLIMIT - positionx)*3));
}

char getPieceID(int positionx, int positiony) {
    return turnOnBits(positionx, positiony) >> ((RIGHTLIMIT - positionx)*3);
}

bool isLedOn(char positionx, char positiony) {
    return (turnOnBits(positionx, positiony) ? true: false);
}

void updateLedColorMatrix(int positionx, int positiony, int pieceID_) {
    unsigned long long row = turnOffBits(positionx, positiony) | ((unsigned long long) pieceID << (RIGHTLIMIT - positionx)*3);
    unsigned long long byte2 = (row >> 32) & ~((unsigned int) 0);
    unsigned long long byte3 = row & ~((unsigned int) 0);
    ledColorMatrix[positiony][1] = byte2;
    ledColorMatrix[positiony][2] = byte3;
}

void getInput(int delaytime) {
    unsigned long long previousTime = millis();
    while (millis() - previousTime < delaytime) {
        controller.update();
        updatePosition();
        delay(INPUTDELAY);
    }
}

void updatePosition() {
    checkForInput(LEFT);
    checkForInput(RIGHT);
    checkForInput(DOWN);
    checkForInput(HARDDROP);
    checkForInput(ROTATE);
}

void checkForInput(int button) {
    if (isPressed(button)) {
        doAction(button);
        lockButton(button);
    } else {
        unlockButton(button);
    }
}

bool isPressed(int button) {
    if (digitalRead(buttonLinks) == 0 && button == LEFT) return true;
    if (digitalRead(buttonRechts) == 0 && button == RIGHT) return true;
    if (digitalRead(buttonsnelbeneden) == 1  && button == DOWN) return true;
    if (digitalRead(buttondrop) == 0 && button == HARDDROP)  return true;
    if (digitalRead(buttonrotate) == 0 && button == ROTATE) return true;
    return false;
}

void doAction(int button) {
    if (button == LEFT || button == RIGHT || button == DOWN) doAdaptiveMovementTo(button);
    if (!button_hardrop_lock && button == HARDDROP) hardDrop(x, y);
    if ( !button_rotate_lock && button == ROTATE) rotate();
}

void doAdaptiveMovementTo(int button) {
    if (!isLocked(button)) {
        moveTo(button);
        movementTime = millis();
    } else if (millis() - movementTime > TRIGGERMAXSPEED) {
        moveTo(button);
        delay(MAXSPEEDDELAY);
    }
}

bool isLocked(int button) {
    if (!button_left_lock && button == LEFT) return true;
    if (!button_right_lock && button == RIGHT) return true;
    if (!button_down_lock && button == DOWN) return true;
    return false;
}

void lockButton(int button) {
    if (button == LEFT) button_left_lock = true;
    if (button == RIGHT) button_right_lock = true;
    if (button == DOWN) button_down_lock = true;
    if (button == HARDDROP) button_hardrop_lock = true;
    if (button == ROTATE) button_rotate_lock = true;
}

void unlockButton(int button) {
    if (button == LEFT) button_left_lock = false;
    if (button == RIGHT) button_right_lock = false;
    if (button == DOWN) button_down_lock = false;
    if (button == HARDDROP) button_hardrop_lock = false;
    if (button == ROTATE) button_rotate_lock = false;
}

void moveTo(int button) {
    if (button == LEFT) move(x+1, y);
    if (button == RIGHT) move(x-1, y);
    if (button == DOWN) move(x, y+1);
}

bool move(int positionx, int positiony) {
    if (!isPositionAvailable(positionx, positiony)) return false;
    deleteCurrentPiece();
    deleteShadow();
    x = positionx;
    y = positiony;
    createShadow();
    drawNewPiece(positionx, positiony, color);
    return true;
}

void hardDrop(int positionx, int positiony) {
    while (move(positionx, ++positiony)) delay(5);
    setupNewTurn();
}

void rotate() {
    if (pieceID == SQUARE_ID) return;
    ++rotationState;
    if (isPositionAvailable(x, y)) {
        --rotationState;
        deleteCurrentPiece();
        deleteShadow();
        ++rotationState;
        createShadow();
        drawNewPiece(x, y, color);
        return;
    }
    --rotationState;
}

void endGame() {
    deleteAllPieces();
    while (1) {
        createFrame();
        if (isRestartButtonPressed(1500)) {
            setupNewGame();
            return;
        }
        printGameOver();
        if (isRestartButtonPressed(750)) {
            setupNewGame();
            return;
        }
    }
}

bool isRestartButtonPressed(int delaytime) {
    unsigned long long previousTime = millis();
    while (millis() - previousTime < delaytime) {
        if (digitalRead(resetbutton)) return false;
        delay(INPUTDELAY);
    }
    return true;
}


void deleteAllPieces() {
    for (int positiony = UPLIMIT; positiony <= DOWNLIMIT; ++positiony)
        for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx)
            if (isLedOn(positionx, positiony)) {
                matrix.drawPixel(positiony, positionx, NOCOLOR);
                delay(50);
            }
}
void display_score(){
    string scorestr = to_string(score);
    reverse(scorestr.begin(),scorestr.end());
    for (int i = 0; i<=scorestr.length();i++){
        matrix.setRotation(3);
        matrix.drawChar(14-(6*i),1,scorestr[i],WHITE,NOCOLOR,1);
        matrix.setRotation(0);
    }
    
}


void display_voornaam(){
    bereikt = false;
    int i=0;
    while (!bereikt){
        if (score >= omzet[i].score){
                matrix.setRotation(3);
                matrix.drawChar(25,i*8,omzet[i].letter,WHITE,NOCOLOR,1);
                matrix.setRotation(0);
                i++;
        }  
        else{
            bereikt = true;
            break;
        }
}
}
void display_achternaam(){
    bereikt = false;
    int i=4;
    while (!bereikt){
        if (score >= omzet[i].score){
                matrix.setRotation(3);
                matrix.drawChar(25,(i-4)*8,omzet[i].letter,WHITE,NOCOLOR,1);
                matrix.setRotation(0);
                i++;
        }
        else{
            bereikt = true;
            break;
        }
}
}
void createFrame() {
    clearScreen();
}
void print_well_done(){
    for (int i = 0; i < 64; i++){
        for(int j = 0; j<32; j++){
            matrix.writePixel(i,j,NOCOLOR);
        }
    }
    matrix.setRotation(3);      
    matrix.drawChar(5,24,'W',RED,NOCOLOR,1);
    matrix.drawChar(11,24,'E',BLUE,NOCOLOR,1);
    matrix.drawChar(17,24,'L',GREEN,NOCOLOR,1);
    matrix.drawChar(23,24,'L',YELLOW,NOCOLOR,1);
    matrix.drawChar(5,32,'D',PURPLE,NOCOLOR,1);
    matrix.drawChar(11,32,'O',CYAN,NOCOLOR,1);
    matrix.drawChar(17,32,'N',WHITE,NOCOLOR,1);
    matrix.drawChar(23,32,'E',ORANGE,NOCOLOR,1);
    
    matrix.setRotation(0);

    //MQTT -
    client.publish(topic, "voltooid");
    //- MQTT
}
void printGameOver() {
    for (int i = 0; i < 64; i++){
        for(int j = 0; j<32; j++){
            matrix.writePixel(i,j,NOCOLOR);
        }
    }
    matrix.setRotation(3);      
    matrix.drawChar(5,24,'G',RED,NOCOLOR,1);
    matrix.drawChar(11,24,'A',BLUE,NOCOLOR,1);
    matrix.drawChar(17,24,'M',GREEN,NOCOLOR,1);
    matrix.drawChar(23,24,'E',YELLOW,NOCOLOR,1);
    matrix.drawChar(5,32,'O',PURPLE,NOCOLOR,1);
    matrix.drawChar(11,32,'V',CYAN,NOCOLOR,1);
    matrix.drawChar(17,32,'E',WHITE,NOCOLOR,1);
    matrix.drawChar(23,32,'R',ORANGE,NOCOLOR,1);
    
    matrix.setRotation(0);

    while(digitalRead(resetbutton)==1){
        delay(20);
    }

    for (int i = 0; i < 64; i++){
        for(int j = 0; j<32; j++){
            matrix.writePixel(i,j,NOCOLOR);
        }
    }
}

void deleteFullLines() {
    int linesDeleted[DOWNLIMIT+1] = { 0 };
    bool deleted = false;
    for (int positiony = DOWNLIMIT; positiony >= UPLIMIT; --positiony) {
        linesDeleted[positiony] = isLineFull(positiony);
        if (linesDeleted[positiony]) {
            deleteLine(positiony);
            deleted = true;
        }
    }
    if (deleted) {
        delay(DELETIONDELAY);
        for (int positiony = 0; positiony <= DOWNLIMIT; ++positiony) 
            if (linesDeleted[positiony]) dropLinesFrom(positiony);
    }
    
}

bool isLineFull(int positiony) {
    for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx)
        if (isAnActualAndTurnedOffPixel(positionx, positiony)) return false;
    return true;
}

void deleteLine(int positiony) {
    for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx) {
        matrix.drawPixel(positiony, positionx, NOCOLOR);
        updateLedColorMatrix(positionx, positiony, NOPIECE);
    }
    score+=25;
    score_changed = true;
}
                                           
void dropLinesFrom(int positiony) {
    while (--positiony > UPLIMIT) {
        for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx) {
            pieceID = getPieceID(positionx, positiony);
            color = getColorByID(pieceID);
            dropPixel(positionx, positiony);
        }
    }
}

void dropPixel(int positionx, int positiony) {
    matrix.drawPixel(positiony, positionx, NOCOLOR);
    matrix.drawPixel(positiony+1, positionx, color);
    updateLedColorMatrix(positionx, positiony, NOPIECE);
    updateLedColorMatrix(positionx, positiony+1, pieceID);
}