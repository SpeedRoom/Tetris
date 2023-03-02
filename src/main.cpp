#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <Wire.h>
#include <ArduinoNunchuk.h>
#include <iostream>
#include <string>
using namespace std;


#define I              { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 } }
#define SQUARE         { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }
#define L              { { 2, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 } }
#define REVERSED_L     { { 0, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 } }
#define Z              { { 0, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 } }
#define REVERSED_Z     { { 1, 1 }, { 2, 1 }, { 0, 2 }, { 1, 2 } }
#define T              { { 1, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 } }
 
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
 
#define SPAWNX          16
#define SPAWNY          11
#define SPAWNROTATION   0
#define UPLIMIT         0
#define LEFTLIMIT       10
#define RIGHTLIMIT      31
#define DOWNLIMIT       63
#define FALLDELAY       100

#define INPUTDELAY      1
#define MAXSPEEDDELAY   50
#define DELETIONDELAY   150
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
int score = 500;
bool bereikt = false;





RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);
ArduinoNunchuk controller = ArduinoNunchuk();
// 32 rows, each row has 16 LEDs
// Every LED has 8 possible values (colors) so it can be stored in 3 bit (from 000 to 111)
// We need 16 * 3 = 48 bit. An unsigned int contain 16 bits on Arduino (bij ons 32), so we'll use 3 unsigned int //TODO unsigned int op esp32 is 32 bit!!!!
unsigned int ledColorMatrix[64][3]; //veranderd
unsigned long long movementTime;
unsigned int color;
char pieceID;
char x, y;
char shadowx;
char shadowy;
char rotationState;  // 4 states, each state is a 90 degrees counterclockwise rotation
// Locks are used to prevent unintentional spam
bool downLock;
bool leftLock;
bool rightLock;
bool cButtonLock;
bool zButtonLock;
char basePieceCoordinates[7][4][2] = {
    I, SQUARE, L,
    REVERSED_L, Z,
    REVERSED_Z, T
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
void display_naam();

struct score_word{
    int score;
    char letter;
};

score_word omzet[] = {
    {200,'G'},
    {400,'E'},
    {600,'E'},
    {800,'R'},
    {1000,'T'},
    {1200,'H'},
    {1400,'O'},
    {1600,'S'},
    {1800,'T'},
    {2000,'E'}
};

void setup() {
    // Serial.begin(230400);
    pinMode(buttonLinks, INPUT_PULLUP); // links knop
    pinMode(buttonRechts, INPUT_PULLUP);
    pinMode(buttondrop,INPUT_PULLUP);
    pinMode(buttonrotate,INPUT_PULLUP);
    matrix.begin();
    // controller.init();
    randomSeed(analogRead(19));
    setupNewGame();
}

void loop() {
    display_score();
    display_naam();
    getInput(FALLDELAY);
    if (canFall()) move(x, y+1);
    else setupNewTurn();   
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
    leftLock = true;
    rightLock = true;
    downLock = true;
    cButtonLock = true;
    zButtonLock = true;
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

unsigned long long getRow(char positiony) {//returned de volledige rij
    // unsigned long long is 64 bit, NIET GROOT GENOEG!!
    //er kunnen 2 * 32 bits in 1 long long, genoeg voor 22 leds

    // unsigned long long byte1 = ledColorMatrix[positiony][0];  // niet nodig
    unsigned long long byte2 = ((unsigned long long) ledColorMatrix[positiony][1]) << 32;   //da was 16 vroeger, moet nu 32 zijn
    unsigned long long byte3 = ledColorMatrix[positiony][2]; 
        // if(positiony == 63){
        //     Serial.println("bytes");
        //     // Serial.println((~byte1),BIN);
        //     Serial.println((~byte2),BIN);
        //     Serial.println((~byte3),BIN);
        //     Serial.println((sizeof(unsigned int)));
        // }
    return byte2 | byte3;
}

unsigned long long turnOnBits(char positionx, char positiony) {
    return getRow(positiony)  &(0b111LL << ((RIGHTLIMIT - positionx)*3)); //((RIGHTLIMIT - positionx)*3) zegt hoeveel 000..000111 moet geshift worden naar links
}

unsigned long long turnOffBits(char positionx, char positiony) {//returned gevraagde rij met positie x op 000
    return getRow(positiony) & ~(0b111LL << ((RIGHTLIMIT - positionx)*3));//*3 omdat iedere led 3 bits heeft om kleur te specifieren
}

char getPieceID(int positionx, int positiony) {
    return turnOnBits(positionx, positiony) >> ((RIGHTLIMIT - positionx)*3);
}

bool isLedOn(char positionx, char positiony) {
    return (turnOnBits(positionx, positiony) ? true: false);
}

void updateLedColorMatrix(int positionx, int positiony, int pieceID_) {
    unsigned long long row = turnOffBits(positionx, positiony) | ((unsigned long long) pieceID << (RIGHTLIMIT - positionx)*3);
    // unsigned long long byte1 = row >> (2 * 32);
    unsigned long long byte2 = (row >> 32) & ~((unsigned int) 0);
    unsigned long long byte3 = row & ~((unsigned int) 0);
    // ledColorMatrix[positiony][0] = byte1;
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
    if (81 < 80 && button == DOWN) return true;
    if (digitalRead(buttondrop) && button == HARDDROP)  return true;
    if (digitalRead(buttonrotate) && button == ROTATE) return true;
    return false;
}

void doAction(int button) {
    if (button == LEFT || button == RIGHT || button == DOWN) doAdaptiveMovementTo(button);
    if (!cButtonLock && button == HARDDROP) hardDrop(x, y);
    if (!zButtonLock && button == ROTATE) rotate();
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
    if (!leftLock && button == LEFT) return true;
    if (!rightLock && button == RIGHT) return true;
    if (!downLock && button == DOWN) return true;
    return false;
}

void lockButton(int button) {
    if (button == LEFT) leftLock = true;
    if (button == RIGHT) rightLock = true;
    if (button == DOWN) downLock = true;
    if (button == HARDDROP) cButtonLock = true;
    if (button == ROTATE) zButtonLock = true;
}

void unlockButton(int button) {
    if (button == LEFT) leftLock = false;
    if (button == RIGHT) rightLock = false;
    if (button == DOWN) downLock = false;
    if (button == HARDDROP) cButtonLock = false;
    if (button == ROTATE) zButtonLock = false;
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
        if (isRestartButtonPressed(750)) {
            setupNewGame();
            return;
        }
        printGameOver();
        if (isRestartButtonPressed(1500)) {
            setupNewGame();
            return;
        }
    }
}

bool isRestartButtonPressed(int delaytime) {
    unsigned long long previousTime = millis();
    while (millis() - previousTime < delaytime) {
        controller.update();
        if (controller.cButton || controller.zButton) return false;
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
    for (int i = 0; i<=scorestr.length();i++){
        matrix.setRotation(3);
        matrix.drawChar(5*i,0,scorestr[i],WHITE,NOCOLOR,1);
        matrix.setRotation(0);
    }
    matrix.drawLine(0, 9, 64, 9, WHITE);
    matrix.drawLine(8,32,8,10,WHITE);
}

void display_naam(){
// bereikt enkel true als we geen hogere score meer hebben dan bepaalde score van onze omzet_map.
//vervolgens worden de bijhorende char op LED geprint, indien index hoger is dan 4 dan doen we -5
//en printen we vervolgens alleen de achternaam en en voornaam gaat weg 
    bereikt = false;
    int i=0;
    while (!bereikt){
        if (score >= omzet[i].score){
            if (i>4){
                matrix.setRotation(3);
                matrix.drawChar(25,(i-5)*7,omzet[i].letter,WHITE,NOCOLOR,1);
                matrix.setRotation(0);
                i++;

            }
            else{
                matrix.setRotation(3);
                matrix.drawChar(25,i*7,omzet[i].letter,WHITE,NOCOLOR,1);
                matrix.setRotation(0);
                i++;
            }  
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

void printGameOver() {
    matrix.drawLine(10, 12, 10, 14, RED);
    matrix.drawLine(11, 15, 13, 15, RED);
    matrix.drawLine(14, 14, 14, 12, RED);
    matrix.drawLine(13, 12, 12, 12, RED);
    matrix.drawPixel(12, 13, RED);
    matrix.drawLine(11, 11, 14, 11, PURPLE);
    matrix.drawLine(11, 9, 14, 9, PURPLE);
    matrix.drawPixel(12, 10, PURPLE);
    matrix.drawPixel(10, 10, PURPLE);
    matrix.drawLine(10, 8, 14, 8, ORANGE);
    matrix.drawLine(10, 4, 14, 4, ORANGE);
    matrix.drawPixel(11, 7, ORANGE);
    matrix.drawPixel(11, 5, ORANGE);
    matrix.drawPixel(12, 6, ORANGE);
    matrix.drawLine(10, 3, 14, 3, BLUE);
    matrix.drawLine(10, 2, 10, 0, BLUE);
    matrix.drawLine(12, 2, 12, 1, BLUE);
    matrix.drawLine(14, 2, 14, 0, BLUE);
    matrix.drawLine(16, 14, 16, 12, YELLOW);
    matrix.drawLine(17, 15, 19, 15, YELLOW);
    matrix.drawLine(17, 11, 19, 11, YELLOW);
    matrix.drawLine(20, 14, 20, 12, YELLOW);
    matrix.drawLine(16, 10, 19, 10, CYAN);
    matrix.drawLine(16, 8, 19, 8, CYAN);
    matrix.drawPixel(20, 9, CYAN);
    matrix.drawLine(16, 7, 20, 7, GREEN);
    matrix.drawLine(16, 6, 16, 4, GREEN);
    matrix.drawLine(18, 6, 18, 5, GREEN);
    matrix.drawLine(20, 6, 20, 4, GREEN);
    matrix.drawLine(16, 3, 20, 3, RED);
    matrix.drawLine(16, 2, 16, 1, RED);
    matrix.drawPixel(17, 1, RED);
    matrix.drawPixel(18, 2, RED);
    matrix.drawPixel(19, 1, RED);
    matrix.drawPixel(20, 0, RED);
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
    //TODO: score vergroten
    score+=50;
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