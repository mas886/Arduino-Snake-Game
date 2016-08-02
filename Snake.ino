/******************************************
 * Snake game for Arduino Uno and Adafruit 2,8" touch screen for Arduino
 * 
 * Written by Arnau/mas886/RedRedNose
 * Webpage: http://redrednose.xyz/
 * 
 * MIT license, all text above must be included in any redistribution
 * 
 * Adafruit libraries used:
 *  Adafruit_ILI9341(Screen controller library): https://github.com/adafruit/Adafruit_ILI9341
 *  Adafruit-GFX-Library(Graphics Library): https://github.com/adafruit/Adafruit-GFX-Library
 *  Adafruit_STMPE610(Touchscreen): https://github.com/adafruit/Adafruit_STMPE610
 *  
 ******************************************/
 
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//1=main menu,2=game,3=game over
byte screen;

//Size of the snake (on proper situation it would be 22x23=506 positions of the grid, though it's limited by arduino's memory
#define snakesize 250
//Snake's speed between movement(miliseconds)
#define velocity 200

#define maxx 230
#define minx 190
#define maxy 80
#define miny 40
#define gridx 24
#define gridy 23

void setup(void) {
  randomSeed(analogRead(0));
  Serial.begin(230400);
  Serial.println("Snake!");
  tft.begin();

  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
  tft.setRotation(2);
  printMenu();
}

void printScore(int score){
  tft.fillRect(90,297,90,20,ILI9341_BLACK);
  tft.setCursor(90, 297);
  tft.print(score);
}

void updateSnake(byte newPos[], byte oldPos[]){
  tft.fillRect(oldPos[0]*10,oldPos[1]*10,10,10,ILI9341_BLACK);
  tft.fillRect(newPos[0]*10,newPos[1]*10,10,10,ILI9341_WHITE);
}


void printGameOver(int score, byte grid[gridy][gridx]){
  screen=3;
  for(int y=0;y<gridy;y++){
    for(int x=0;x<gridx;x++){
      grid[y][x]=0;
    }
  }
  tft.fillRect(40,55,160,180,ILI9341_LIGHTGREY);
  tft.drawRect(41,56,158,178,ILI9341_MAROON);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(4);
  tft.setCursor(55, 60);
  tft.print("Game");
  tft.setCursor(90, 90);
  tft.print("Over");
  tft.setTextColor(ILI9341_NAVY);
  tft.setTextSize(3);
  tft.setCursor(70, 120);
  tft.print("Score");
  tft.drawRect(65,150,110,30,ILI9341_NAVY);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setCursor(70, 155);
  tft.print(score);
  tft.fillRect(80,185,80,30,ILI9341_RED);
  tft.setCursor(85, 188);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Play");
}

void printGameScreen(){
  screen=2;
  tft.fillScreen(ILI9341_BLACK);
  tft.drawLine(0, 230, 240, 230, ILI9341_YELLOW);
  //Arrow up-1
  tft.drawLine(30,240,30,280,ILI9341_WHITE);
  tft.drawLine(30,240,10,260,ILI9341_WHITE);
  tft.drawLine(30,240,50,260,ILI9341_WHITE);
  //Arrow right-2
  tft.drawLine(70,260,110,260,ILI9341_WHITE);
  tft.drawLine(110,260,90,240,ILI9341_WHITE);
  tft.drawLine(110,260,90,280,ILI9341_WHITE);
  //Arrow down-3
  tft.drawLine(150,240,150,280,ILI9341_WHITE);
  tft.drawLine(150,280,130,260,ILI9341_WHITE);
  tft.drawLine(150,280,170,260,ILI9341_WHITE);
  //Arrow left-4
  tft.drawLine(190,260,230,260,ILI9341_WHITE);
  tft.drawLine(190,260,210,240,ILI9341_WHITE);
  tft.drawLine(190,260,210,280,ILI9341_WHITE);
  //----------------
  tft.drawLine(0, 290, 240, 290, ILI9341_YELLOW);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10 , 297);
  tft.println("Score: ");
  printScore(0);
  tft.fillRect(180,295,50,20,ILI9341_RED);
  tft.setCursor(182, 297);
  tft.print("Menu");
}

void printMenu(){
  screen=1;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(5);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(30 , 30);
  tft.println("SNAKE!");
  tft.fillRect(50,120,120,50,ILI9341_GREEN);
  tft.setTextSize(3);
  tft.setCursor(63 , 133);
  tft.println("Start");
  tft.setCursor(0 , 310);
  tft.setTextSize(1);
  tft.println("Created by Arnau/Redrednose/mas886");
}

void loop()
{
  byte snakeBuffer[snakesize][2]={0};
  byte grid[gridy][gridx]={0};
  long addpos;
  long delpos;
  byte arrow, lastarrow;
  int score;
  byte newPos[2];
  byte oldPos[2];
  bool gendot;
  bool incaxys;
  byte increment;
  bool initgame=false;
  while(true){
    //This if will update the snake position while we are in the game screen
    if (screen==2){
      if((arrow!=0)&&(((lastarrow+2!=arrow)&&(lastarrow-2!=arrow))||(score==0))){
        if((arrow%2)!=0){
          incaxys=true;
        }else{
          incaxys=false;
        }
        if((arrow>1)&&(arrow<4)){
          increment=1;
        }else{
          increment=-1;
        }
        lastarrow=arrow;
      }
      arrow=0;
      newPos[incaxys]+=increment;
      if((newPos[0]<0)|(newPos[0]>23)|(newPos[1]<0)|(newPos[1]>22)){        
        printGameOver(score,grid);
        
      }else{
        snakeBuffer[addpos][0]=newPos[0];
        snakeBuffer[addpos][1]=newPos[1];
        addpos++;
        if(addpos>(snakesize-1)){
          addpos=0;
        }
        grid[newPos[1]][newPos[0]]++;
        grid[oldPos[1]][oldPos[0]]=0;

        //Check various grid events
        switch(grid[newPos[1]][newPos[0]]){
          case 2:
            printGameOver(score, grid);
            break;
          case 4:
            score+=10;
            printScore(score);
            gendot=true;
            if((score/10)<snakesize){
              delpos--;
            }
          default:
            updateSnake(newPos,oldPos);
            delpos++;
            if(delpos>(snakesize-1)){
              delpos=0;
            }
            oldPos[0]=snakeBuffer[delpos][0];
            oldPos[1]=snakeBuffer[delpos][1];
            delay(velocity);
            break;
        }
      }
      //We generate a random dot to feed the snake
      while(gendot){
        byte posy=random(gridy-1);
        byte posx=random(gridx-1);
        if(grid[posy][posx]==0){
          tft.fillRect(posx*10,posy*10,10,10,ILI9341_CYAN);
          grid[posy][posx]=3;
          gendot=false;
        }
      }
    }
    // Button control, only triggered when there's data from the touchscreen
    while (!ts.bufferEmpty()) {
      // Retrieve a point  
      TS_Point p = ts.getPoint();
      // Scale from ~0->4000 to tft.width using the calibration #'s
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

      switch(screen){
        case 1:
        //menu and start game parameters
          if((p.x>=80)&&(p.x<=190)&&(p.y>=140)&&(p.y<=190)){
            initgame=true;
          }
          break;
        case 2:
        //Check arrows on in game screen
          if (!((p.x>=10)&&(p.x<=60)&&(p.y>=10)&&(p.y<=25))){
            for(int c=0,g=1;c<=180;c+=60,g++){
              if((p.x>=minx-c)&&(p.x<=maxx-c)&&(p.y>=miny)&&(p.y<=maxy)){
                arrow=g;
              }
            }
          }else if((p.x>=10)&&(p.x<=60)&&(p.y>=10)&&(p.y<=25)){
            printMenu();
          }
          break;
        case 3:
          if((p.x>=10)&&(p.x<=60)&&(p.y>=10)&&(p.y<=25)){
            printMenu();
          }else if((p.x>=80)&&(p.x<=160)&&(p.y>=100)&&(p.y<=130)){
            initgame=true;
          }
          break;
      }
      if (initgame){//Set variables for the game initialization
        initgame=false;
        arrow=2;
        lastarrow=1;
        score=0;
        addpos=1;
        delpos=0;
        incaxys=false;
        increment=1;
        gendot=true;
        newPos[0]=-1;newPos[1]=0;
        printGameScreen();
      }
    }
  }
}
