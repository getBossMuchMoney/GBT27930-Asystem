/*******************************************************************************
 * File Name: TemperatureCal.c
 * Description: Calculate the temperature from the sampling value.
 * Author:  Miller Xue
 * Version: 00
 * Date:
*******************************************************************************/
#include "F28x_Project.h"
#include "string.h"
#include "TempCal.h"

stTempCal stTemperatureArry[cTableSize] = {
    {-40.0f, 99.1f},   {-30, 57.54},   {-20, 34.60},   {-10, 21.48},   {0,   13.72},
    {5,   11.08},   {10,  9.001},   {15,  7.357},   {20,  6.048},   {25,  5.0},
    {30,  4.156},   {35 , 3.472},   {40,  2.914},   {45,  2.458},   {50,  2.083},
    {55,  1.773},   {60,  1.515},   {65,  1.3},   {70,  1.12},   {75,  0.9683},
    {80,  0.8404},   {85 , 0.7319},   {90,  0.6397},   {95,  0.5608},   {100,  0.4933},
    {105,  0.4352},  {110,  0.3851},  {115, 0.3418},   {120,  0.3042},  {125,  0.2714},
    {130,  0.2428},   {135 , 0.2178},   {140,  0.1959},   {145,  0.1765},   {150,  0.1711}
};
int16_t wTabIndexLeft,wTabIndexRight,wTabIndexMid;
int16_t wPositionLeft,wPositionRight;
float dwKm,dwX_x1,dwY_Out;

float   sTemperatureCal(float uwAdcSample);
float   Adc2Volt(Uint16 adc_data);

float   sTemperatureCal(float uwAdcSample)
{
    wTabIndexLeft = 0;
    wTabIndexRight = cTableSize - 1;
    wPositionLeft = 0;
    wPositionRight = wPositionLeft + 1;
    wTabIndexMid = 0;
    dwKm = 0;
    dwX_x1 = 0;
    dwY_Out = 0;

    if(wTabIndexRight <= 0)
    {
        wTabIndexRight = 0;
        return(stTemperatureArry[0].wTempDegree);
    }

    if(uwAdcSample > stTemperatureArry[0].wAdcSampleValue)
    {
        return(stTemperatureArry[0].wTempDegree);
    }
    else if(uwAdcSample <= stTemperatureArry[cTableSize-1].wAdcSampleValue)
    {
        return(stTemperatureArry[cTableSize-1].wTempDegree);
    }

    wTabIndexMid = (wTabIndexLeft + wTabIndexRight)>>1;

    while(wTabIndexLeft <= (wTabIndexRight - 2))
    {
        wTabIndexMid = (wTabIndexLeft + wTabIndexRight)>>1;
        if((stTemperatureArry[wTabIndexMid].wAdcSampleValue) <= uwAdcSample)
        {
            wTabIndexRight = wTabIndexMid;
        }
        else
        {
            wTabIndexLeft = wTabIndexMid;
        }
    }

    if(wTabIndexLeft == (wTabIndexRight - 1))
    {
        wPositionLeft = wTabIndexLeft;
        wPositionRight = wTabIndexRight;
    }
    else
    {
        return(stTemperatureArry[0].wAdcSampleValue);
    }

    //y = [(y2-y1)/(x2-x1)]*(x-x1) + y1
    dwKm = (stTemperatureArry[wPositionRight].wTempDegree - stTemperatureArry[wPositionLeft].wTempDegree);
    dwKm = dwKm/(stTemperatureArry[wPositionRight].wAdcSampleValue - stTemperatureArry[wPositionLeft].wAdcSampleValue);
    dwX_x1 = (float)(uwAdcSample - stTemperatureArry[wPositionLeft].wAdcSampleValue)*dwKm;
    dwY_Out = dwX_x1 + stTemperatureArry[wPositionLeft].wTempDegree;

    return(dwY_Out);
}

float Adc2Volt(Uint16 adc_data)
{
    float volt_val;
    volt_val = ((float)adc_data) * 3.3f / 4095.0f; // 0-4095 <==> 0-3.3V
    return volt_val;
}



