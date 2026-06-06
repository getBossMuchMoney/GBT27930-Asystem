/*
 * TempCal.h
 *
 *  Created on: 2025Äê5ÔÂ6ÈÕ
 *      Author: Miller Xue
 */



#ifndef SRCS_TEMPCAL_H_
#define SRCS_TEMPCAL_H_

#include "F28x_Project.h"

#define   cTableSize   35

typedef struct
{
    float wTempDegree;
    float wAdcSampleValue;
}stTempCal;

extern float sTemperatureCal(float uwAdcSample);
extern float   Adc2Volt(Uint16 adc_data);
#endif /* SRCS_TEMPCAL_H_ */
