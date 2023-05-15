#include "Tetris_function.h"
#include "RGBmatrixPanel.h"

Tetris_function::Tetris_function(){
    score = 0;
    bereikt = false;
    score_changed = false;
    controller = ArduinoNunchuk();
    movementTime = 0;
    color = 0;
    pieceID = 0;
    x = 0;
    y = 0;
    shadowx = 0;
    shadowy = 0;
    rotationState = 0;
    button_down_lock = false;
    button_left_lock = false;
    button_right_lock = false;
    button_hardrop_lock = false;
    button_rotate_lock = false;
    int tempBasePieceCoordinates[7][4][2] = {
    { {-1, 0}, {0, 0}, {1, 0}, {2, 0} },    // I
    { {0, 0}, {1, 0}, {0, 1}, {1, 1} },     // SQUARE
    { {1, 0}, {-1, 1}, {0, 1}, {1, 1} },    // L
    { {-1, 0}, {-1, 1}, {0, 1}, {1, 1} },   // REVERSED_L
    { {-1, 0}, {0, 0}, {0, 1}, {1, 1} },    // Z
    { {0, 0}, {1, 0}, {-1, 1}, {0, 1} },    // REVERSED_Z
    { {0, 0}, {-1, 1}, {0, 1}, {1, 1} }     // T
};

for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 2; k++) {
            basePieceCoordinates[i][j][k] = tempBasePieceCoordinates[i][j][k];
        }
    }
}
    matrix=new RGBmatrixPanel(A, B, C, D, CLK, LAT, OE, false, 64);

   score_word temp[] = {
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
        {950,'S',false}
    };

    for (int i = 0; i < sizeof(temp) / sizeof(score_word); i++) {
        omzet[i] = temp[i];
    }
    unsigned int ledColorMatrix[64][3];

}


bool Tetris_function::canFall() {
    if (!isPositionAvailable(x, y+1)) {
        getInput(PLACEMENTDELAY);
        if (!isPositionAvailable(x, y+1)) return false;
    }
    return true;
}

bool Tetris_function::isPositionAvailable(int x_, int y_) {
    for (int pieceIndex = 0; pieceIndex < 4; ++pieceIndex) {
        int positionx, positiony;
        getCoordinates(x_, y_, &positionx, &positiony, pieceIndex);
        if (!isAnActualAndTurnedOffPixel(positionx, positiony)) return false;
    }
    return true;
}

void Tetris_function::getCoordinates(int x_, int y_, int* positionx, int* positiony, int pieceIndex) {
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

bool Tetris_function::isAnActualAndTurnedOffPixel(int positionx, int positiony) {
    if (positionx < LEFTLIMIT || positionx > RIGHTLIMIT) return false;
    if (positiony > DOWNLIMIT) return false;
    if (isLedOn(positionx, positiony)) return false;
    return true;
}

void Tetris_function::createShadow() {
    shadowx = x;
    shadowy = y;
    while (isPositionAvailable(shadowx, ++shadowy));
    // shadowy has to be decreased because we have checked that
    // that position is not available with the while condition
    drawNewPiece(shadowx, --shadowy, WHITE);
}

void Tetris_function::deleteCurrentPiece() {
    drawNewPiece(x, y, NOPIECE);
}

void Tetris_function::deleteShadow() {
    drawNewPiece(shadowx, shadowy, NOPIECE);
}

void Tetris_function::drawNewPiece(int x_, int y_, int pieceColor) {
    for (int pieceIndex = 0; pieceIndex < 4; ++pieceIndex) {
        int positionx, positiony;
        getCoordinates(x_, y_, &positionx, &positiony, pieceIndex);
        matrix->drawPixel(positiony, positionx, pieceColor);
    }
}

void Tetris_function::setupNewGame() {
    score = 0;
    if (score == 0){
        matrix->setRotation(3);
        matrix->drawChar(0,0,'0',WHITE,NOCOLOR,1);
        matrix->setRotation(0);
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
    printStartGame();
}

void Tetris_function::setupNewTurn() {
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

void Tetris_function::setupNewPiece() {
    x = SPAWNX;
    y = SPAWNY;
    shadowx = SPAWNX;
    shadowy = SPAWNY;
    rotationState = SPAWNROTATION;
    pieceID = (random() % 7) + 1;
    color = getColorByID(pieceID);
}

void Tetris_function::clearScreen() {
    for (int positiony = UPLIMIT; positiony <= DOWNLIMIT; ++positiony)
        for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx)
            matrix->drawPixel(positiony, positionx, NOCOLOR);
}

void Tetris_function::clearLedColorMatrix() {
    for (int positiony = UPLIMIT; positiony <= DOWNLIMIT; ++positiony) {
        ledColorMatrix[positiony][0] = NOPIECE;
        ledColorMatrix[positiony][1] = NOPIECE;
        ledColorMatrix[positiony][2] = NOPIECE;
    }
}

void Tetris_function::addPieceToMatrix() {
    for (int pixelIndex = 0; pixelIndex < 4; ++pixelIndex) {
        int positionx, positiony;
        getCoordinates(x, y, &positionx, &positiony, pixelIndex);
        updateLedColorMatrix(positionx, positiony, pieceID);
    }
}


unsigned int Tetris_function::getColorByID(char pieceID) {
    if (pieceID == NOPIECE) return NOCOLOR;
    if (pieceID == I_ID) return CYAN;
    if (pieceID == SQUARE_ID) return YELLOW;
    if (pieceID == REVERSED_L_ID) return ORANGE;
    if (pieceID == L_ID) return BLUE;
    if (pieceID == REVERSED_Z_ID) return RED;
    if (pieceID == Z_ID) return GREEN;
    if (pieceID == T_ID) return PURPLE;
}

unsigned long long Tetris_function::getRow(char positiony) {
    unsigned long long byte2 = ((unsigned long long) ledColorMatrix[positiony][1]) << 32;
    unsigned long long byte3 = ledColorMatrix[positiony][2]; 
    return byte2 | byte3;
}

unsigned long long Tetris_function::turnOnBits(char positionx, char positiony) {
    return getRow(positiony)  &(0b111LL << ((RIGHTLIMIT - positionx)*3));
}

unsigned long long Tetris_function::turnOffBits(char positionx, char positiony) {
    return getRow(positiony) & ~(0b111LL << ((RIGHTLIMIT - positionx)*3));
}

char Tetris_function::getPieceID(int positionx, int positiony) {
    return turnOnBits(positionx, positiony) >> ((RIGHTLIMIT - positionx)*3);
}

bool Tetris_function::isLedOn(char positionx, char positiony) {
    return (turnOnBits(positionx, positiony) ? true: false);
}

void Tetris_function::updateLedColorMatrix(int positionx, int positiony, int pieceID_) {
    unsigned long long row = turnOffBits(positionx, positiony) | ((unsigned long long) pieceID << (RIGHTLIMIT - positionx)*3);
    unsigned long long byte2 = (row >> 32) & ~((unsigned int) 0);
    unsigned long long byte3 = row & ~((unsigned int) 0);
    ledColorMatrix[positiony][1] = byte2;
    ledColorMatrix[positiony][2] = byte3;
}

void Tetris_function::getInput(int delaytime) {
    unsigned long long previousTime = millis();
    while (millis() - previousTime < delaytime) {
        controller.update();
        updatePosition();
        delay(INPUTDELAY);
    }
}

void Tetris_function::updatePosition() {
    checkForInput(LEFT);
    checkForInput(RIGHT);
    checkForInput(DOWN);
    checkForInput(HARDDROP);
    checkForInput(ROTATE);
}

void Tetris_function::checkForInput(int button) {
    if (isPressed(button)) {
        doAction(button);
        lockButton(button);
    } else {
        unlockButton(button);
    }
}

bool Tetris_function::isPressed(int button) {
    if (digitalRead(buttonLinks) == 0 && button == LEFT) return true;
    if (digitalRead(buttonRechts) == 0 && button == RIGHT) return true;
    if (digitalRead(buttonsnelbeneden) == 1  && button == DOWN) return true;
    if (digitalRead(buttondrop) == 0 && button == HARDDROP)  return true;
    if (digitalRead(buttonrotate) == 0 && button == ROTATE) return true;
    return false;
}

void Tetris_function::doAction(int button) {
    if (button == LEFT || button == RIGHT || button == DOWN) doAdaptiveMovementTo(button);
    if (!button_hardrop_lock && button == HARDDROP) hardDrop(x, y);
    if ( !button_rotate_lock && button == ROTATE) rotate();
}

void Tetris_function::doAdaptiveMovementTo(int button) {
    if (!isLocked(button)) {
        moveTo(button);
        movementTime = millis();
    } else if (millis() - movementTime > TRIGGERMAXSPEED) {
        moveTo(button);
        delay(MAXSPEEDDELAY);
    }
}

bool Tetris_function::isLocked(int button) {
    if (!button_left_lock && button == LEFT) return true;
    if (!button_right_lock && button == RIGHT) return true;
    if (!button_down_lock && button == DOWN) return true;
    return false;
}

void Tetris_function::lockButton(int button) {
    if (button == LEFT) button_left_lock = true;
    if (button == RIGHT) button_right_lock = true;
    if (button == DOWN) button_down_lock = true;
    if (button == HARDDROP) button_hardrop_lock = true;
    if (button == ROTATE) button_rotate_lock = true;
}

void Tetris_function::unlockButton(int button) {
    if (button == LEFT) button_left_lock = false;
    if (button == RIGHT) button_right_lock = false;
    if (button == DOWN) button_down_lock = false;
    if (button == HARDDROP) button_hardrop_lock = false;
    if (button == ROTATE) button_rotate_lock = false;
}

void Tetris_function::moveTo(int button) {
    if (button == LEFT) move(x+1, y);
    if (button == RIGHT) move(x-1, y);
    if (button == DOWN) move(x, y+1);
}

bool Tetris_function::move(int positionx, int positiony) {
    if (!isPositionAvailable(positionx, positiony)) return false;
    deleteCurrentPiece();
    deleteShadow();
    x = positionx;
    y = positiony;
    createShadow();
    drawNewPiece(positionx, positiony, color);
    return true;
}

void Tetris_function::hardDrop(int positionx, int positiony) {
    while (move(positionx, ++positiony)) delay(5);
    setupNewTurn();
}

void Tetris_function::rotate() {
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

void Tetris_function::endGame() {
    deleteAllPieces();
    //client.publish(topic, "game over");
    while (1) {
        createFrame();
        if (isRestartButtonPressed(1500)) {
            setupNewGame();
            screenToBlack();
            return;
        }
        printGameOver();
        if (isRestartButtonPressed(750)) {
            setupNewGame();
            screenToBlack();
            return;
        }
    }
}

bool Tetris_function::isRestartButtonPressed(int delaytime) {
    unsigned long long previousTime = millis();
    while (millis() - previousTime < delaytime) {
        if (digitalRead(resetbutton)) return false;
        delay(INPUTDELAY);
    }
    return true;
}


void Tetris_function::deleteAllPieces() {
    for (int positiony = UPLIMIT; positiony <= DOWNLIMIT; ++positiony)
        for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx)
            if (isLedOn(positionx, positiony)) {
                matrix->drawPixel(positiony, positionx, NOCOLOR);
                delay(50);
            }
}
void Tetris_function::display_score(){
    string scorestr = to_string(score);
    reverse(scorestr.begin(),scorestr.end());
    for (int i = 0; i<=scorestr.length();i++){
        matrix->setRotation(3);
        matrix->drawChar(14-(6*i),1,scorestr[i],WHITE,NOCOLOR,1);
        matrix->setRotation(0);
    }
    
}


void Tetris_function::display_voornaam(){
    bereikt = false;
    int i=0;
    while (!bereikt){
        if (score >= omzet[i].score){
                matrix->setRotation(3);
                matrix->drawChar(25,i*8,omzet[i].letter,WHITE,NOCOLOR,1);
                matrix->setRotation(0);
                i++;
        }  
        else{
            bereikt = true;
            break;
        }
}
}
void Tetris_function::display_achternaam(){
    bereikt = false;
    int i=4;
    while (!bereikt){
        if (score >= omzet[i].score){
                matrix->setRotation(3);
                matrix->drawChar(25,(i-4)*8,omzet[i].letter,WHITE,NOCOLOR,1);
                matrix->setRotation(0);
                i++;
        }
        else{
            bereikt = true;
            break;
        }
}
}
void Tetris_function::createFrame() {
    clearScreen();
}
void Tetris_function::printWellDone(){
    screenToBlack();
    matrix->setRotation(3);      
    matrix->drawChar(5,24,'W',RED,NOCOLOR,1);
    matrix->drawChar(11,24,'E',BLUE,NOCOLOR,1);
    matrix->drawChar(17,24,'L',GREEN,NOCOLOR,1);
    matrix->drawChar(23,24,'L',YELLOW,NOCOLOR,1);
    matrix->drawChar(5,32,'D',PURPLE,NOCOLOR,1);
    matrix->drawChar(11,32,'O',CYAN,NOCOLOR,1);
    matrix->drawChar(17,32,'N',WHITE,NOCOLOR,1);
    matrix->drawChar(23,32,'E',ORANGE,NOCOLOR,1);
    
    matrix->setRotation(0);

    //client.publish(topic, "voltooid");  
}

void Tetris_function::printStartGame(){
    screenToBlack();
    matrix->setRotation(3);      
    matrix->drawChar(1,24,'S',RED,NOCOLOR,1);
    matrix->drawChar(7,24,'T',BLUE,NOCOLOR,1);
    matrix->drawChar(13,24,'A',GREEN,NOCOLOR,1);
    matrix->drawChar(19,24,'R',YELLOW,NOCOLOR,1);
    matrix->drawChar(25,24,'T',YELLOW,NOCOLOR,1);
    matrix->drawChar(5,32,'G',PURPLE,NOCOLOR,1);
    matrix->drawChar(11,32,'A',CYAN,NOCOLOR,1);
    matrix->drawChar(17,32,'M',WHITE,NOCOLOR,1);
    matrix->drawChar(23,32,'E',ORANGE,NOCOLOR,1);
    
    matrix->setRotation(0);
}

void Tetris_function::printGameOver() {
    screenToBlack();
    matrix->setRotation(3);      
    matrix->drawChar(5,24,'G',RED,NOCOLOR,1);
    matrix->drawChar(11,24,'A',BLUE,NOCOLOR,1);
    matrix->drawChar(17,24,'M',GREEN,NOCOLOR,1);
    matrix->drawChar(23,24,'E',YELLOW,NOCOLOR,1);
    matrix->drawChar(5,32,'O',PURPLE,NOCOLOR,1);
    matrix->drawChar(11,32,'V',CYAN,NOCOLOR,1);
    matrix->drawChar(17,32,'E',WHITE,NOCOLOR,1);
    matrix->drawChar(23,32,'R',ORANGE,NOCOLOR,1);
    
    matrix->setRotation(0);

    while(digitalRead(resetbutton)==1){
        delay(20);
    }

    screenToBlack();
}

void Tetris_function::deleteFullLines() {
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

bool Tetris_function::isLineFull(int positiony) {
    for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx)
        if (isAnActualAndTurnedOffPixel(positionx, positiony)) return false;
    return true;
}

void Tetris_function::deleteLine(int positiony) {
    for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx) {
        matrix->drawPixel(positiony, positionx, NOCOLOR);
        updateLedColorMatrix(positionx, positiony, NOPIECE);
    }
    score+=75;
    score_changed = true;
}
                                           
void Tetris_function::dropLinesFrom(int positiony) {
    while (--positiony > UPLIMIT) {
        for (int positionx = LEFTLIMIT; positionx <= RIGHTLIMIT; ++positionx) {
            pieceID = getPieceID(positionx, positiony);
            color = getColorByID(pieceID);
            dropPixel(positionx, positiony);
        }
    }
}

void Tetris_function::dropPixel(int positionx, int positiony) {
    matrix->drawPixel(positiony, positionx, NOCOLOR);
    matrix->drawPixel(positiony+1, positionx, color);
    updateLedColorMatrix(positionx, positiony, NOPIECE);
    updateLedColorMatrix(positionx, positiony+1, pieceID);
}

void Tetris_function::screenToBlack(){
    for (int i = 0; i < 64; i++){
        for(int j = 0; j<32; j++){
            matrix->writePixel(i,j,NOCOLOR);
        }
    }

}
