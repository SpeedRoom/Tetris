#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <ArduinoNunchuk.h>
#include <iostream>
#include <string>
#include "OTAlib.h"
using namespace std;

#define NOCOLOR        matrix->Color888(0, 0, 0)
#define CYAN           matrix->Color888(0, 16, 16)
#define YELLOW         matrix->Color888(16, 16, 0)
#define ORANGE         matrix->Color888(48, 16, 0)
#define BLUE           matrix->Color888(0, 0, 16)
#define RED            matrix->Color888(16, 0, 0)
#define GREEN          matrix->Color888(0, 16, 0)
#define PURPLE         matrix->Color888(16, 0, 16)
#define WHITE          matrix->Color888(16, 16, 16)

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
#define FALLDELAY       100

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

#define buttonLinks     19 
#define buttonRechts    13 
#define buttondrop      4
#define buttonrotate    2
#define resetbutton     0
#define buttonsnelbeneden 35

struct score_word{
    int score;
    char letter;
    bool bereikt;
};

class Tetris_function{
public:
    int score;
    bool bereikt;
    bool score_changed;
    RGBmatrixPanel* matrix=new RGBmatrixPanel(A, B, C, D, CLK, LAT, OE, false, 64);
    ArduinoNunchuk controller;
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
    short int basePieceCoordinates[7][4][2] ={
        { {-1, 0}, {0, 0}, {1, 0}, {2, 0} },    // I
        { {0, 0}, {1, 0}, {0, 1}, {1, 1} },     // SQUARE
        { {1, 0}, {-1, 1}, {0, 1}, {1, 1} },    // L
        { {-1, 0}, {-1, 1}, {0, 1}, {1, 1} },   // REVERSED_L
        { {-1, 0}, {0, 0}, {0, 1}, {1, 1} },    // Z
        { {0, 0}, {1, 0}, {-1, 1}, {0, 1} },    // REVERSED_Z
        { {0, 0}, {-1, 1}, {0, 1}, {1, 1} }     // T
    };;
    score_word omzet[12]={
        {50,'D',false},
        {100,'A',false},
        {200,'A',false},
        {300,'N',false},
        {400,'P',false},
        {500,'E',false},
        {600,'E',false},
        {700,'T',false},
        {800,'E',false},
        {900,'R',false},
        {950,'S',false},
    };;

    Tetris_function();

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

    void printWellDone();
    
    void printStartGame();

    void screenToBlack();
};