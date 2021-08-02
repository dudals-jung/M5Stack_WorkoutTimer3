#include <M5Stack.h>
#include <SimpleTimer.h>
#include <Wire.h>
#include <time.h>

#define TRUNON_INTERVAL 35
#define TRUNOFF_INTERVAL 3

#define WorkoutBoxHeight 20
#define WorkoutBoxWidth  15
#define WorkoutBoxGapX  3
#define WorkoutBoxGapY  0

enum eWorkoutMode
{
    wmRest,
    wmWorkout,
};

enum tRunMode
{
    rmStop,
    rmStart,
    rmPause,
    rmTimeOver,
};

typedef struct
{
    eWorkoutMode WorkoutMode;
    uint16_t Time;
} tWorkout;

tWorkout Workout[]=
{
    { wmWorkout , 0 },
    { wmRest , 30 },
};

SimpleTimer timer;

uint16_t WorkoutIndex = 0;

uint16_t WorkoutCount = 0;
uint16_t RestCount = 0;

uint16_t TimeRemaining = 0;
uint16_t RunTime = 0;
uint16_t TotalTime = 0;
uint16_t preTimeRemaining = 0;
uint16_t PreRunTime = 0;

int TimerCallback1msID;

tRunMode RunMode = rmStop;

unsigned long lastTimerTime = millis();

bool ToggledLastFewSeconds = false;
bool Toogled1000ms = false;
uint16_t TimerCallbackCount = 0;
uint16_t ToogleCount = 0;
uint16_t Countdown = 0;

void TimerCallback1ms(void)
{
    TimerCallbackCount = ( TimerCallbackCount + 1) % 4;

    if ( TimerCallbackCount == 0 )
        Toogled1000ms = true;

//    if ( Toogled1000ms == true)
    {
    
    if ( Workout[WorkoutIndex % 2].Time != 0 )
    {
        if ( TimeRemaining < 10 )
        {
            ToggledLastFewSeconds = !ToggledLastFewSeconds;
        }
        else
        {
            ToggledLastFewSeconds = false;
        }
                        if ( TimeRemaining < 5 )
                        {
                            if ( preTimeRemaining != TimeRemaining )
                            {
                                // M5.Speaker.tone(3000, 10);
                                M5.Speaker.beep();
                            }
                        }
                
                        preTimeRemaining = TimeRemaining;

    }
    else
    {
        ToggledLastFewSeconds = false;
    }
    }
    DrawInformation();
}

void setup()
{
    int i = 0;

    M5.begin();
    M5.Power.begin();

//    Serial.begin(115200);  일부 아두이노 버전에서 컴피일-다운로드 후 정상적으로 동작하지 않는 경우가 있음
//
    DrawRunMode();
    InitWorkout();

    TimerCallback1msID = timer.setInterval(250, TimerCallback1ms);
    timer.enable(TimerCallback1msID);
}

void DrawRunMode(void)
{
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
    
    switch ( RunMode )
    {
        case rmStop:
            M5.Lcd.fillRoundRect(5, 205, 90, 35, 3, TFT_WHITE);
            M5.Lcd.drawString("Start", 47, 225, 4);
            break;
        case rmStart:
            M5.Lcd.fillRoundRect(5, 205, 90, 35, 3, TFT_WHITE);
            M5.Lcd.drawString("Stop", 47, 225, 4);
            break;
        case rmPause:
            M5.Lcd.fillRoundRect(5, 205, 90, 35, 3, TFT_DARKGREY);
            break;
        case rmTimeOver:
            M5.Lcd.fillRoundRect(5, 205, 90, 35, 3, TFT_WHITE);
            M5.Lcd.drawString("Stop", 47, 225, 4);
            break;
    }
    
    switch ( RunMode )
    {
        case rmStop:
            M5.Lcd.fillRoundRect(110, 205, 100, 35, 3, TFT_DARKGREY);
            break;
        case rmStart:
            M5.Lcd.fillRoundRect(110, 205, 100, 35, 3, TFT_WHITE);
            M5.Lcd.drawString("Pause", 160, 225, 4);
            break;
        case rmPause:
            M5.Lcd.fillRoundRect(110, 205, 100, 35, 3, TFT_WHITE);
            M5.Lcd.drawString("Paused", 160, 225, 4);
            break;
        case rmTimeOver:
            M5.Lcd.fillRoundRect(110, 205, 100, 35, 3, TFT_DARKGREY);
            break;
    }

    switch ( RunMode )
    {
        case rmStop:
            M5.Lcd.fillRoundRect(225, 205, 90, 35, 3, TFT_DARKGREY);
            break;
        case rmStart:
            M5.Lcd.fillRoundRect(225, 205, 90, 35, 3, TFT_WHITE);
            M5.Lcd.drawString("Next", 270, 225, 4);
            break;
        case rmPause:
            M5.Lcd.fillRoundRect(225, 205, 90, 35, 3, TFT_DARKGREY);
            break;
        case rmTimeOver:
            M5.Lcd.fillRoundRect(225, 205, 90, 35, 3, TFT_WHITE);
            M5.Lcd.drawString("Next", 270, 225, 4);
            break;
    }
}

void DrawWorkoutCount()
{
    uint16_t LoopCount ;
    uint16_t x, y, temp  ;

    if ( ( WorkoutCount * (WorkoutBoxWidth + WorkoutBoxGapX)) > 320)
        temp = ((320 / WorkoutCount) - WorkoutBoxGapX); 
    else
        temp = WorkoutBoxWidth; 

    M5.Lcd.fillRect(0, 0, 319, 5 + WorkoutBoxHeight + WorkoutBoxGapY, TFT_BLACK);

    for (LoopCount = 0; LoopCount < WorkoutCount; LoopCount++)
    {
        x = LoopCount * (temp + WorkoutBoxGapX);
        y = 5; // (bool)Workout[LoopCount].WorkoutMode * ( WorkoutBoxHeight + WorkoutBoxGapY) ;
  
        if ( LoopCount < (WorkoutCount-1) )
        {
            M5.Lcd.fillRect(x, y, temp, WorkoutBoxHeight, TFT_GREEN);
        }
        else
        {
            if ( WorkoutCount == RestCount )
                M5.Lcd.fillRect(x, y, temp, WorkoutBoxHeight,  TFT_YELLOW);
            else
                M5.Lcd.fillRect(x, y, temp, WorkoutBoxHeight,  TFT_RED);
        }
    }
}

void DrawInformation()
{
    char temp[30];

    uint16_t LoopCount ;
    uint16_t Workouts = 0;
    uint16_t x,y ;

    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    M5.Lcd.setTextDatum(MC_DATUM);

    M5.Lcd.drawRoundRect(5, 50, 150, 60, 3, TFT_GREEN);

    sprintf(temp,"Workout");
    M5.Lcd.drawString(temp, 77, 70, 4);

    sprintf(temp,"  %2d - %2d  ", WorkoutCount, RestCount);
    M5.Lcd.drawString(temp, 77, 95, 4);

    M5.Lcd.drawRoundRect(165, 50, 150, 60, 3, TFT_GREEN);

    sprintf(temp,"Total");
    M5.Lcd.drawString(temp, 240, 70, 4);

    sprintf(temp,"%02d : %02d : %02d  ", (TotalTime % (60 * 60 * 24))/(60 * 60), (TotalTime % (60 * 60)) / 60, TotalTime % 60);
    M5.Lcd.drawString(temp, 245, 95, 4);

    M5.Lcd.drawRoundRect(5, 120, 310, 70, 3, TFT_GREEN);

    M5.Lcd.setTextDatum(ML_DATUM);

    if ( RunMode == rmStart)
    {
        if (Workout[WorkoutIndex % 2].WorkoutMode == wmRest)
        {
                sprintf(temp,"Rest         ");
                M5.Lcd.drawString(temp, 15, 155, 4);
        }
        else
        {
                sprintf(temp,"Workout ");
                M5.Lcd.drawString(temp, 15, 155, 4);
        }
    }
    else
    {
        sprintf(temp,"Watting    ");
        M5.Lcd.drawString(temp, 15, 155, 4);
    }

    M5.Lcd.setTextDatum(TL_DATUM);

    if ( Workout[WorkoutIndex % 2].Time != 0 )
    {
        sprintf(temp,"%02d : %02d", TimeRemaining / 60, TimeRemaining % 60 );
    }
    else
    {
        sprintf(temp,"%02d : %02d", RunTime / 60, RunTime % 60 );
    }

    if ( ToggledLastFewSeconds == false )
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    else
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);

    M5.Lcd.drawString(temp, 140, 130, 7);
}

void NextWorkout(void)
{
    WorkoutIndex++;
    RunMode = rmStart;
    ChangeWorkout();
}

void InitWorkout(void)
{
    TimeRemaining = Workout[WorkoutIndex % 2].Time;

    switch ( Workout[WorkoutIndex % 2].WorkoutMode )
    {
        case wmRest:
            RestCount++;
            break;
        case wmWorkout:
            WorkoutCount++;
            break;
    }
    
    TimerCallbackCount = 0;
    RunTime = 0;
    preTimeRemaining = 0;
    PreRunTime = 0;
    DrawRunMode();
    DrawWorkoutCount();
}

void ChangeWorkout(void)
{
    InitWorkout();
    RunMode = rmStart;
}

void loop()
{
    char temp[30];

    M5.update();

    timer.run();

//    if ( RunMode == rmStart)
    {
        if ( Toogled1000ms == true )
        {
            TimerCallbackCount = 0;
            
            switch ( RunMode )
            {
                case rmStop:

                    break;
                case rmStart:
                    TotalTime++;
                    RunTime++;            
            
                    if ( Workout[WorkoutIndex % 2].Time != 0 )
                    {
                        TimeRemaining--;


                        if (TimeRemaining == 0)
                        {
                            // M5.Speaker.tone(3000, 10);
                            // M5.Speaker.tone(3000, 10);
                            M5.Speaker.beep();
//                            delay(1);
//                            M5.Speaker.beep();

                            RunMode = rmTimeOver;
                            DrawRunMode();
                            // NextWorkout();  // 자동으로 넘어가려면
                        }                
                    }
                    else
                    {
                    }
            
                    if ( (RunTime % 60) == 0 ) // 1분 알림
                    {
                        if ( PreRunTime != RunTime )
                        {
                            PreRunTime = RunTime;
                            // M5.Speaker.tone(2000, 10);
                            M5.Speaker.beep();
                        }
                    }

                    break;  
                case rmPause:
                    break;
                case rmTimeOver:
                    TotalTime++;
                    break;
            }

            Toogled1000ms = false;
        }
    }

    if (M5.BtnA.wasReleased())
    {
//        M5.Speaker.tone(3000, 10);
        M5.Speaker.beep();

        switch ( RunMode )
        {
            case rmStop:
                RunMode = rmStart;
                TotalTime = 0;
                WorkoutIndex = 0;
                TimeRemaining = 0;

                WorkoutCount = 0;
                RestCount = 0;
                
                ChangeWorkout();
                break;
            case rmStart:
                RunMode = rmStop;
                break;
            case rmPause:
                break;
            case rmTimeOver:
                break;
        }
    
        DrawRunMode();
    }

    if (M5.BtnB.wasReleased())
    {
        // M5.Speaker.tone(3000, 10);
        M5.Speaker.beep();

        switch ( RunMode )
        {
            case rmStop:
                break;
            case rmStart:
                RunMode = rmPause;
                break;
            case rmPause:
                RunMode = rmStart;
                break;
            case rmTimeOver:
                break;
        }

        DrawRunMode();
    }
    
    if (M5.BtnC.wasReleased())
    {
//        M5.Speaker.tone(3000, 10);
        M5.Speaker.beep();

        switch ( RunMode )
        {
            case rmStop:
                break;
            case rmStart:
            case rmTimeOver:
                NextWorkout();
                break;
            case rmPause:
                break;
        }
    }
}
