
#include "M5Cardputer.h"
#include "SD.h"
#include "Audio.h"
#include "SPI.h"
#include "FS.h"

#define UI_COLOR PURPLE
#define LOCK_COLOR GREEN

#define SD_SCK 40
#define SD_MISO 39
#define SD_MOSI 14
#define SD_CS 12

#define I2S_DOUT 42
#define I2S_BCLK 41
#define I2S_LRCK 43

Audio audio;

const uint8_t border_width = 4;

bool is_mute = false;
bool is_playing = false;
uint8_t dice_count = 0;

void drawDice(const uint8_t num, const int32_t x, const int32_t y, const int32_t size, const bool locked){
    if(num < 1 || num > 6) return;
    
    const float radius = (size / 9);
    const float border = border_width * (1 + locked);
    
    M5Cardputer.Display.setColor(locked ? LOCK_COLOR : UI_COLOR);
    M5Cardputer.Display.fillRect(x,y,size,size);
    M5Cardputer.Display.setColor(BLACK);
    M5Cardputer.Display.fillRect(x+border_width,y+border_width,size-border_width*2,size-border_width*2);
    M5Cardputer.Display.setColor(locked ? LOCK_COLOR : UI_COLOR);

    switch ( num ) {
        case 1:
            M5Cardputer.Display.fillCircle(x + size/2,y + size/2,radius);
            break;

        case 2:
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius,y + border_width*2 + radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + size - border_width*2 - radius, radius);
            break;

        case 3:
            M5Cardputer.Display.fillCircle(x + size/2,y + size/2,radius);
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius,y + border_width*2 + radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + size - border_width*2 - radius, radius);
            break;

        case 4:
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius,y + border_width*2 + radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + size - border_width*2 - radius, radius);
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius, y + size - border_width*2 - radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + border_width*2 + radius, radius);
            break;

        case 5:
            M5Cardputer.Display.fillCircle(x + size/2,y + size/2,radius);
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius,y + border_width*2 + radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + size - border_width*2 - radius, radius);
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius, y + size - border_width*2 - radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + border_width*2 + radius, radius);
            break;

        case 6:
            M5Cardputer.Display.fillCircle(x + radius + border_width*2, y + size/2,radius);
            M5Cardputer.Display.fillCircle(x + size - radius - border_width*2, y + size/2,radius);
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius,y + border_width*2 + radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + size - border_width*2 - radius, radius);
            M5Cardputer.Display.fillCircle(x + border_width*2 + radius, y + size - border_width*2 - radius, radius);
            M5Cardputer.Display.fillCircle(x + size - border_width*2 - radius, y + border_width*2 + radius, radius);
            break;
    };

}

void drawHomeScreen(){
    M5Cardputer.Display.setFont(&fonts::FreeSansBold24pt7b);
    M5Cardputer.Display.setTextSize(0.45);
    M5Cardputer.Display.drawString("YAMB GAME",
                                   M5Cardputer.Display.width() / 2,
                                   M5Cardputer.Display.height() / 5);

    M5Cardputer.Display.drawString("DICE ROLLER",
                                   M5Cardputer.Display.width() / 2,
                                   1.8 * M5Cardputer.Display.height() / 5);
                                   
    M5Cardputer.Display.setFont(&fonts::FreeSansBold24pt7b);
    M5Cardputer.Display.drawString("Choose the number",
                                   M5Cardputer.Display.width() / 2,
                                   3.2 * M5Cardputer.Display.height() / 5);
    M5Cardputer.Display.drawString("of die (5 - 9)",
                                   M5Cardputer.Display.width() / 2,
                                   4 * M5Cardputer.Display.height() / 5);
}

void drawInfoMessage(const String message){
    M5Cardputer.Display.clear();
    M5Cardputer.Display.setTextSize(0.65);
    M5Cardputer.Display.setFont(&fonts::FreeSansBold24pt7b);
    M5Cardputer.Display.drawString(message,
                                M5Cardputer.Display.width() / 2,
                                M5Cardputer.Display.height() / 2);
}

void drawHelpMenu(){
    M5Cardputer.Display.setFont(&fonts::FreeSansBold18pt7b);
    M5Cardputer.Display.setTextSize(0.4);

    const int startPos = M5Cardputer.Display.height() / 6;
    const int stepPos = M5Cardputer.Display.height() / 6;
    const int leftMargin = 10;

    const String helpInfos[5] = {"Press 1-9 to lock the dice",
                               "Press c to unlock all die", 
                               "Press s to shuffle the die", 
                               "Press r to reset the program",
                               "Press m to mute"};

    M5Cardputer.Display.setTextDatum(middle_left);
    for(int i = 0; i < 5; i++){
        M5Cardputer.Display.drawString(helpInfos[i], leftMargin, startPos + (i*stepPos));
    }
    M5Cardputer.Display.setTextDatum(middle_center);
}

uint8_t die[9] = {1,1,1,1,1,1,1,1,1};
uint8_t past_die[9];
bool locked[9] = {0,0,0,0,0,0,0,0,0};
bool past_locked[9];
bool info_was_up = false;

void drawDie(){

    const int32_t size = M5Cardputer.Display.width() / (dice_count / 2 + 3);
    const int32_t step = size + border_width*2;

    const int32_t xTopPos = (M5Cardputer.Display.width() - floor((float)dice_count / 2) * step + (step-size)) / 2;
    const int32_t yTopPos = (M5Cardputer.Display.height() - 2*step + (step-size)) / 2;

    const int32_t xBotPos = (M5Cardputer.Display.width() - ceil((float)dice_count/2) * step + (step-size)) / 2;
    const int32_t yBotPos = yTopPos + step;

    for(int i = 0; i < dice_count / 2; i++){
        if(past_die[i] == die[i] && locked[i] && locked[i] == past_locked[i] && !info_was_up) continue;
        else drawDice(die[i], xTopPos + step*i, yTopPos, size, locked[i]);

        past_die[i] = die[i];
        past_locked[i] = locked[i];
    }

    for(int i = dice_count / 2; i < dice_count; i++){
        if(past_die[i] == die[i] && locked[i] && locked[i] == past_locked[i] && !info_was_up) continue;
        else drawDice(die[i], xBotPos + step*(i-dice_count/2), yBotPos, size, locked[i]);

        past_die[i] = die[i];
        past_locked[i] = locked[i];
    }
}

void drawPointsMenu(){
    M5Cardputer.Display.setFont(&fonts::FreeSansBold24pt7b);
    M5Cardputer.Display.setTextSize(0.45);

    int locked_points = 0;
    int unlocked_points = 0;
    int total_points = 0;
    
    const int startPos = M5Cardputer.Display.height() / 6;
    const int stepPos = M5Cardputer.Display.height() / 6;
    const int leftMargin = 10;

    for(int i = 0; i < dice_count; i++){
        if(locked[i]) locked_points += die[i];
        else unlocked_points += die[i];

        total_points += die[i];
    }

    M5Cardputer.Display.setTextDatum(middle_left);
    M5Cardputer.Display.setTextColor(LOCK_COLOR);
    M5Cardputer.Display.drawString("Locked Points: " + String(locked_points), leftMargin, startPos);
    M5Cardputer.Display.setTextColor(UI_COLOR);
    M5Cardputer.Display.drawString("Unlocked Points: " + String(unlocked_points), leftMargin, startPos + stepPos);
    M5Cardputer.Display.setTextColor(SILVER);
    M5Cardputer.Display.drawString("Total Points: " + String(total_points), leftMargin, startPos + stepPos*2);
    M5Cardputer.Display.setTextColor(UI_COLOR);
    M5Cardputer.Display.setTextDatum(middle_center);
}


void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextColor(UI_COLOR);
    M5Cardputer.Display.setTextDatum(middle_center);
    drawHomeScreen();
    srand(time(NULL));

    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
    SPI.setFrequency(1000000);
    SD.begin(SD_CS);
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    audio.setVolume(21); // 0...21
}

void loop() {
    M5Cardputer.update();
    
    if(M5Cardputer.Keyboard.isKeyPressed('h')){   
        M5Cardputer.Display.clear();
        drawHelpMenu();
        delay(2000);
        M5Cardputer.Display.clear();
        info_was_up = true;
        drawDie();
        drawDie();
        info_was_up = false;
    }

    if(M5Cardputer.Keyboard.isKeyPressed('p')){   
        M5Cardputer.Display.clear();
        drawPointsMenu();
        delay(2000);
        M5Cardputer.Display.clear();
        info_was_up = true;
        drawDie();
        drawDie();
        info_was_up = false;
    }

    if(M5Cardputer.Keyboard.isKeyPressed('r')){
        M5Cardputer.Display.clear();
        is_playing = false;
        dice_count = 0;
        for(int i = 0; i < 9; i++) {
            die[i] = 1;
            locked[i] = false;
        }
        drawHomeScreen();
        delay(1000);
    }

    if(M5Cardputer.Keyboard.isKeyPressed('c')){
        audio.connecttoFS(SD, "magical.mp3");
        vTaskDelay(1);
        audio.loop();
        while(audio.isRunning()){
            M5Cardputer.update();
            vTaskDelay(1);
            audio.loop();
        }
        
        for(int i = 0; i < dice_count; i++){
            locked[i] = false;
        }
        drawDie();
    }

    if(M5Cardputer.Keyboard.isPressed() && is_playing) {
        if(M5Cardputer.Keyboard.isKeyPressed('s')){
            audio.connecttoFS(SD, "dice_rolling.mp3");
            vTaskDelay(1);
            audio.loop();

            int shuffle_cnt = 0;
            while(audio.isRunning()){
                M5Cardputer.update();
                vTaskDelay(1);
                audio.loop();

                if(shuffle_cnt >= 5) {
                    for(int i = 0; i < dice_count; i++){
                        if(!locked[i]){
                            die[i] = (rand() % 6) + 1;
                        }
                    }
                    drawDie();
                    shuffle_cnt = 0;
                }
                shuffle_cnt++;
            }
        }

        if(M5Cardputer.Keyboard.isKeyPressed('m')){
            is_mute = !is_mute;
            audio.setVolume(21 * (!is_mute));
            delay(700);
        }
        
        for(int i = 1; i <= dice_count; i++) {
            if(M5Cardputer.Keyboard.isKeyPressed((char)(i + '0'))){
                locked[i-1] = !locked[i-1];
                break;
            }
        }

        delay(500);
        drawDie();
    }

    if(M5Cardputer.Keyboard.isPressed() && !is_playing){
        for(int i = 5; i <= 9; i++){
            if(M5Cardputer.Keyboard.isKeyPressed((char)(i + '0'))){
                dice_count = i;
                is_playing = true;
                break;
            }
        }

        if(!is_playing && !M5Cardputer.Keyboard.isKeyPressed('r')){
            drawInfoMessage("Wrong number.");
            delay(800);
            M5Cardputer.Display.clear();
            drawHomeScreen();
        }
        else if(!M5Cardputer.Keyboard.isKeyPressed('r')){
            drawInfoMessage("Great Choice!");
            delay(800);
            M5Cardputer.Display.clear();
            drawDie();
        }
    }
}