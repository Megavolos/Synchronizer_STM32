#include "process.h"

unsigned char MEMS=1;
Signal piezo0, piezo1, piezo2, piezo3, mems0, mems1;
DataToSend piezo0ToSend, piezo1ToSend, piezo2ToSend, piezo3ToSend, mems0ToSend, mems1ToSend;
DataToSend* sendArray[4];
Signal* signalsArray[4];
unsigned char i=0;
SensorMode sensorMode = PIEZO2MEMS2;
MeasureMode measureMode = STOP;
void sendPointsToUart(DataToSend *data)
{
    //организовываем передачу по UART
}
float sendDifferenceToUart(unsigned char channel1, unsigned char channel2)
{
    //угол наклона вычисляется по тангенсу: (y2-y1)/(x2-x1)
    unsigned int numerator1=sendArray[channel1]->y2 - sendArray[channel1]->y1;
    unsigned int denominator1=sendArray[channel1]->x2 - sendArray[channel1]->x1;
    unsigned int numerator2=sendArray[channel2]->y2 - sendArray[channel2]->y1;
    unsigned int denominator2=sendArray[channel2]->x2 - sendArray[channel2]->x1;
    float tan1=numerator1/denominator1;
    float tan2=numerator2/denominator2;
    float diff=tan2-tan1;
    return diff;

}
void filter (unsigned char* adc)
{
    for (i=0; i<2; i++)
    {

        signalsArray[i]->filterOut=signalsArray[i]->filterOut + signalsArray[i]->filterCoeff *(adc[i]-signalsArray[i]->filterOut);
        if (signalsArray[i]->dataIndex)
        {
            signalsArray[i]->data[signalsArray[i]->dataIndex]=signalsArray[i]->data[signalsArray[i]->dataIndex-1]+signalsArray[i]->filterOut; //добавляем в буфер отфильтрованный семпл
        }
        else
        {
            signalsArray[i]->data[signalsArray[i]->dataIndex]=signalsArray[i]->filterOut; //добавляем в буфер отфильтрованный семпл

        }

    }
    for (i=2; i<4; i++)
    {

        signalsArray[i]->filterOut=signalsArray[i]->filterOut + signalsArray[i]->filterCoeff *(adc[i]-signalsArray[i]->filterOut);
        signalsArray[i]->data[signalsArray[i]->dataIndex]=signalsArray[i]->filterOut; //добавляем в буфер отфильтрованный семпл


    }
}

void measureAndSend(unsigned char channel)
{
    unsigned char j=0;
    unsigned int topLevel=((float)signalsArray[channel]->topMeasureLevel/100)*signalsArray[channel]->data[signalsArray[channel]->dataIndex];
    unsigned int bottomLevel=((float)signalsArray[channel]->bottomMeasureLevel/100)*signalsArray[channel]->data[signalsArray[channel]->dataIndex];
    unsigned char bottomFound=0;
    for (j=0; j<signalsArray[channel]->dataIndex; j++)
    {
        if (!bottomFound)
        {
            if (signalsArray[channel]->data[j]>=bottomLevel)
            {
                sendArray[channel]->x1=j;
                sendArray[channel]->y1=signalsArray[channel]->data[j];
                bottomFound=1;
            }
        }
        else
        {
            if (signalsArray[channel]->data[j]>topLevel)
            {
                sendArray[channel]->x2=j;
                sendArray[channel]->y2=signalsArray[channel]->data[j];
                sendPointsToUart(sendArray[channel]);
                break;
            }
        }
    }
    resetSignalAfterSend(channel);

}

void process (unsigned char* adc)
{
// adc - массив [0..3], где [0..1] - пьезо, [2..3] - mems
    

    filter(adc);
    

    for (i=0; i<sensorMode; i++)
    {
        if (signalsArray[i]->filterOut > signalsArray[i]->cutLevel) //Если на выходе фильтра уровень выше отсечки (для пьезо)
        {
            if (!signalsArray[i]->isReady) //инициализация после захвата фронта или других действий
            {
                resetSignalAfterSend(i);
                resetSignalAfterSend(i+2);
            }

            signalsArray[i]->samplesCounter++; //увеличиваем счетчик семплов
            signalsArray[i]->isSignalPresent=1; //ставим флаг, что мы захватили сигнал (пока неизвестно, может быть ложный)
            signalsArray[i]->dataIndex++; //Внимание! Максимумальный индекс - DATA_BUFFER_LENGTH


        }
        else //если сигнал ниже порога cutLevel
        {
            if (signalsArray[i]->samplesCounter>signalsArray[i]->minSamplesCounter) //если количество семплов, которые больше cutLevel, больше minSamplesCounter
            {
                signalsArray[i]->isSignalCaptured=1; //считаем, что сигнал захвачен, и можно приступать к измерениям
                measureAndSend(i);


            }
            else
            {

            }
            signalsArray[i]->isReady=0;
            signalsArray[i]->dataIndex=0;
        }
    }
    if (sensorMode==)
    {
        for (i=2; i<4; i++)
        {
            if (signalsArray[i-2]->isSignalPresent)
            {
                measureAndSend(i);
            }
        }
    }

}
void resetSignalAfterSend(unsigned char channel)
{
    signalsArray[channel]->integralValue=0;
    signalsArray[channel]->isSignalCaptured=0;
    signalsArray[channel]->isSignalPresent=0;
    signalsArray[channel]->samplesCounter=0;
    signalsArray[channel]->sum=0;
    signalsArray[channel]->dataIndex=0;
    signalsArray[channel]->isReady=1;
//    for (j=0; j<DATA_BUFFER_LENGTH; j++)
//    {
//        signalsArray[channel]->data[j]=0;
//    }
}

void initSignals()
{
    unsigned int j=0;
    signalsArray[0]=&piezo0;
    signalsArray[1]=&piezo1;
    signalsArray[2]=(sensorMode==PIEZO2MEMS2)? &mems0 : &piezo0;
    signalsArray[3]=(sensorMode==PIEZO2MEMS2)? &mems1 : &piezo1;
    sendArray[0]= &piezo0ToSend;
    sendArray[1] = &piezo1ToSend;
    sendArray[2] = (sensorMode==PIEZO2MEMS2)? &mems0ToSend : &piezo0ToSend;
    sendArray[3] = (sensorMode==PIEZO2MEMS2)? &mems1ToSend : &piezo1ToSend;
    for (i=0; i<4; i++)
    {
       signalsArray[i]->cutLevel=10;
       signalsArray[i]->filterCoeff=1;
       signalsArray[i]->filterOut=0;
       signalsArray[i]->integralValue=0;
       signalsArray[i]->isSignalCaptured=0;
       signalsArray[i]->isSignalPresent=0;
       signalsArray[i]->minSamplesCounter=20;
       signalsArray[i]->samplesCounter=0;
       signalsArray[i]->sum=0;
       signalsArray[i]->dataIndex=0;
       signalsArray[i]->topMeasureLevel=80;
       signalsArray[i]->bottomMeasureLevel=20;
       for (j=0; j<DATA_BUFFER_LENGTH; j++)
       {
           signalsArray[i]->data[j]=0;
       }
    }

}
