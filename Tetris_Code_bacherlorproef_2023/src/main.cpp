#include "Tetris_function.h"
#include <RGBmatrixPanel.h>

Tetris_function tetris;

void setup() {

  //Setup buttons
  pinMode(buttonLinks, INPUT_PULLUP);
  pinMode(buttonRechts, INPUT_PULLUP); 
  pinMode(buttondrop,INPUT_PULLUP);
  pinMode(buttonrotate,INPUT_PULLUP);
  pinMode(resetbutton,INPUT_PULLUP);
  pinMode(buttonsnelbeneden,INPUT);

  //Start Tetris
  tetris.matrix->begin();
  randomSeed(analogRead(34));
  tetris.setupNewGame();
}

void loop() {
    if (tetris.isRestartButtonPressed(750)){
        tetris.screenToBlack();
        while(tetris.score < 1000){
        if(tetris.score ==0){
            tetris.display_score();
        }
        tetris.matrix->drawLine(0, 10, 64, 10, tetris.matrix->Color888(16, 16, 16)); 
        tetris.matrix->drawLine(8,32,8,11,tetris.matrix->Color888(16, 16, 16)); 
        tetris.score_changed=false;
        tetris.getInput(FALLDELAY);

        if (tetris.canFall()) tetris.move(tetris.x, (tetris.y)+1);
        else tetris.setupNewTurn();

        if (tetris.score_changed){
            tetris.display_score();
            if (tetris.score >= 400){
                if(tetris.score == 400){
                    for (int i = 0; i < 56; i++){
                        for(int j = 0; j<9; j++){
                            tetris.matrix->writePixel(i,j,tetris.matrix->Color888(0,0,0));
                        }
                    }
                }
                tetris.display_achternaam();
            }
            else{tetris.display_voornaam();}
        }
        }   
        tetris.printWellDone();
        // kijken of de resetbutton wordt ingedrukt op nieuwe game te starten
        if (tetris.isRestartButtonPressed(500)) {
            for (int i = 0; i < 64; i++){
                for(int j = 0; j<32; j++){
                    tetris.matrix->writePixel(i,j,tetris.matrix->Color888(0,0,0));
                }
            }
            tetris.setupNewGame();
            return;
        }
    }


  

}