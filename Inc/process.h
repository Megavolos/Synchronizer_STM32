#ifndef PROCESS_H
#define PROCESS_H
#define DATA_BUFFER_LENGTH 256
typedef struct
{
    unsigned int data[DATA_BUFFER_LENGTH];
    unsigned char dataIndex;
    float filterOut;
    unsigned char isSignalCaptured;
    unsigned char isSignalPresent;
    unsigned char isReady;
    unsigned char minSamplesCounter;
    unsigned int samplesCounter;
    unsigned int sum;
    //пороги измерений (в процентах) от верха и низа первообразной (интеграла)
    unsigned char bottomMeasureLevel;
    unsigned char topMeasureLevel;
    //***
    unsigned char cutLevel;
    float filterCoeff;
    float integralValue;
} Signal;
typedef struct
{
    unsigned char x1,x2;
    unsigned int y1,y2;


} DataToSend;
typedef enum
{
    PIEZO2MEMS2 = 2,
    PIEZO4MEMS0 = 4

} SensorMode;
typedef enum
{
    STOP,
    POINTS, //только 4 точки на уровнях bottomMeasureLevel и topMeasureLevel. Каждая пара точек - значение АЦП и номер семпла от начала превышения порога cutLevel
    TAN,    //частное (тангенс)
    DIFF    //разница
} MeasureMode;  
unsigned char processSample (Signal* signal);
void resetSignalAfterSend(unsigned char channel);
void process (unsigned char* adc);

#endif // PROCESS_H
