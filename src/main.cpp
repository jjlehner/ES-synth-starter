#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include "ThreadSafeArray.hpp"
#include "ThreadSafeList.hpp"
#include "Tasks.hpp"
#include "ES_CAN.h"
#include "CANFrame.hpp"
#include "SoundGenerator.hpp"

//Pin definitions
//Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

//Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

//Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

//Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

//Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

volatile uint8_t TX_Message[8] = {0};

volatile Knobs k0;
volatile Knobs k1;
volatile Knobs k2(0,8);
volatile Knobs k3;

ThreadSafeArray threadSafeArray;

void sampleISR();

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;
SemaphoreHandle_t CAN_TX_Semaphore;

std::atomic<int32_t> currentStepSize;

//Sound generation
ThreadSafeList<Note> notesPressed;
SoundGenerator soundGenerator = SoundGenerator();

//Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
    digitalWrite(REN_PIN, LOW);
    digitalWrite(RA0_PIN, bitIdx & 0x01);
    digitalWrite(RA1_PIN, bitIdx & 0x02);
    digitalWrite(RA2_PIN, bitIdx & 0x04);
    digitalWrite(OUT_PIN, value);
    digitalWrite(REN_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(REN_PIN, LOW);
}

void CAN_TX_ISR();

bool boardWest(){
    return IOHelper::getMatrixValue(5,3);
}
void establishPosition(){
    IOHelper::setRow(6);
    digitalWrite(OUT_PIN, true);
    delayMicroseconds(500);
    uint8_t devicesSeen = 0;
    std::array<uint8_t, 8> message;
    bool westMost = boardWest();
    Serial.println("Started");
    while(!westMost){
        xQueueReceive(msgInQ, message.data(), 0);
        devicesSeen = message[0]+1;
        westMost = boardWest();
    }
    message[0] = devicesSeen;
    CAN_TX(0x123, message.data());
    IOHelper::setRow(6);
    digitalWrite(OUT_PIN, false);
    Serial.println(devicesSeen);
}
void setup() {
    // put your setup code here, to run once:
    //Initialise UART
    Serial.begin(9600);

    msgInQ = xQueueCreate(36, 8);
    msgOutQ = xQueueCreate(36, 8);
    CAN_TX_Semaphore = xSemaphoreCreateCounting(3, 3);
    CAN_Init(false);
#ifndef PROFILING
    CAN_RegisterRX_ISR(CANFrame::receiveISR);
    CAN_RegisterTX_ISR(CAN_TX_ISR);
    setCANFilter(0x123, 0x7ff);
    CAN_Start();
#endif

    //Set pin directions
    pinMode(RA0_PIN, OUTPUT);
    pinMode(RA1_PIN, OUTPUT);
    pinMode(RA2_PIN, OUTPUT);
    pinMode(REN_PIN, OUTPUT);
    pinMode(OUT_PIN, OUTPUT);
    pinMode(OUTL_PIN, OUTPUT);
    pinMode(OUTR_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(C0_PIN, INPUT);
    pinMode(C1_PIN, INPUT);
    pinMode(C2_PIN, INPUT);
    pinMode(C3_PIN, INPUT);
    pinMode(JOYX_PIN, INPUT);
    pinMode(JOYY_PIN, INPUT);

    //Initialise display
    setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
    delayMicroseconds(2);
    setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
    u8g2.begin();
    setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

    threadSafeArray.initMutex();

    //Start ISR
#ifndef PROFILING
    TIM_TypeDef *Instance = TIM1;
    auto *sampleTimer = new HardwareTimer(Instance);
    sampleTimer->setOverflow(22000, HERTZ_FORMAT);
    sampleTimer->attachInterrupt(sampleISR);
    sampleTimer->resume();

    TaskHandle_t scanKeysHandler = nullptr;
    TaskHandle_t displayUpdateHandler = nullptr;
    TaskHandle_t decodeHandler = nullptr;
    TaskHandle_t transmitHandler = nullptr;

    //establishPosition();

     xTaskCreate(Tasks::scanKeysTask,/* Function that implements the task */
                 "scanKeys",/* Text name for the task */
                 256,/* Stack size in words, not bytes*/
                 nullptr,/* Parameter passed into the task */
                 2,/* Task priority*/
                 &scanKeysHandler /* Pointer to store the task handle*/
     );
     xTaskCreate(Tasks::displayUpdateTask,/* Function that implements the task */
                 "displayUpdate",/* Text name for the task */
                 256,/* Stack size in words, not bytes*/
                 nullptr,/* Parameter passed into the task */
                 1,/* Task priority*/
                 &displayUpdateHandler /* Pointer to store the task handle*/
     );
     xTaskCreate(Tasks::decodeTask,
                 "decodeTask",
                 32,
                 nullptr,
                 3,
                 &decodeHandler
     );
     xTaskCreate(Tasks::transmitTask,
                 "transmitTask",
                 32,
                 nullptr,
                 3,
                 &transmitHandler
     );
     vTaskStartScheduler();
#endif

#ifdef PROFILING
    uint32_t starttime = micros();
    Tasks::scanKeysTask(nullptr);
    uint32_t scanKeyTaskLength = (micros() - starttime)/PROFILING_REPEATS;

    starttime = micros();
    Tasks::displayUpdateTask(nullptr);
    uint32_t displayUpdateTaskLength = (micros() - starttime)/PROFILING_REPEATS;

    std::array<uint8_t,8> fakeMessage = { 0xA,0xB,0xC,0xD,0xA,0xB,0xC,0xD};
    for(int i = 0; i<32;i++){
        xQueueSend(msgInQ, fakeMessage.data(), NULL);
    }

    starttime = micros();
    Tasks::decodeTask(nullptr);
    uint32_t decodeTaskLength = (micros() - starttime)/PROFILING_REPEATS_TRANSMIT_TASK;

    for(int i = 0; i<32;i++){
        xQueueSend(msgOutQ, fakeMessage.data(), NULL);
    }
    starttime = micros();
    Tasks::transmitTask(nullptr);
    uint32_t transmitTaskLength = (micros() - starttime);

    Serial.println("----Results of Profiling----");
    Serial.println(("Scan Key Task           - " + std::to_string(scanKeyTaskLength)).c_str());
    Serial.println(("Display Update Task     - " + std::to_string(displayUpdateTaskLength)).c_str());
    Serial.println(("Message Decode Task     - " + std::to_string(decodeTaskLength)).c_str());
    Serial.println(("Message Transmitt Task  - " + std::to_string(transmitTaskLength)).c_str());

#endif
}


void loop() {
    // put your main code here, to run repeatedly:
//    Serial.println();
//    char bytes[14];
//    sprintf(bytes,"%d, %d, %d, %d", k3.getRotation(), k2.getRotation(), k1.getRotation(), k0.getRotation());
//    Serial.println(bytes);


}

void sampleISR() {
    // static int32_t phaseAcc = 0;
    // auto t = notesPressed.read();
    // if(t.second){
    //     phaseAcc += t.first[0].getStepSize();
    // }
    // int32_t Vout = phaseAcc >> 24;
    // Vout = Vout >> (8 - k3.getRotation() / 2);
    // analogWrite(OUTR_PIN, Vout + 128);
    int32_t Vout = soundGenerator.getSound();
    analogWrite(OUTR_PIN, Vout + 128);
}

void CAN_TX_ISR() {
    xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}