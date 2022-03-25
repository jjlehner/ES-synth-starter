#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include "ThreadSafeList.hpp"
#include "Tasks.hpp"
#include "ES_CAN.h"
#include "CANFrame.hpp"
#include "SoundGenerator.hpp"
#include "Recorder.hpp"

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

volatile Knob k0;
volatile Knob k1;
volatile Knob k2(0, 8);
volatile Knob k3;

void sampleISR();

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;
SemaphoreHandle_t CAN_TX_Semaphore;

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

TaskHandle_t scanKeysHandler = nullptr;
TaskHandle_t displayUpdateHandler = nullptr;
TaskHandle_t decodeHandler = nullptr;
TaskHandle_t transmitHandler = nullptr;

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

    //Start ISR
#ifndef PROFILING
    TIM_TypeDef *Instance = TIM1;
    auto *sampleTimer = new HardwareTimer(Instance);
    sampleTimer->setOverflow(22000, HERTZ_FORMAT);
    sampleTimer->attachInterrupt(sampleISR);
    sampleTimer->resume();

    //establishPosition();

     xTaskCreate(Tasks::scanKeysTask,/* Function that implements the task */
                 "scanKeys",/* Text name for the task */
                 128,/* Stack size in words, not bytes*/
                 nullptr,/* Parameter passed into the task */
                 3,/* Task priority*/
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
                 64,
                 nullptr,
                 2,
                 &decodeHandler
     );
     xTaskCreate(Tasks::transmitTask,
                 "transmitTask",
                 64,
                 nullptr,
                 2,
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

    std::array<uint8_t,8> fakeMessage = { 0x50,0xB,0xC,0xD,0xA,0xB,0xC,0xD};
    for(int i = 0; i<32;i++){
        xQueueSend(msgInQ, fakeMessage.data(), NULL);
    }

    starttime = micros();
    Tasks::decodeTask(nullptr);
    uint32_t decodeTaskLength = (micros() - starttime)/PROFILING_REPEATS;

    for(int i = 0; i<32;i++){
        xQueueSend(msgOutQ, fakeMessage.data(), NULL);
    }
    starttime = micros();
    Tasks::transmitTask(nullptr);
    uint32_t transmitTaskLength = (micros() - starttime)/PROFILING_REPEATS_TRANSMIT_TASK;

    for(size_t i = 0; i<5;i++) {
        notesPressed.push_back(Note{
                static_cast<uint8_t>(i), 4, micros(), PhaseAccPool::aquirePhaseAcc()});
    }
    starttime = micros();
    for(int i = 0; i < PROFILING_REPEATS; i++){
        sampleISR();
    }
    uint32_t sampleISRLength = (micros() - starttime)/PROFILING_REPEATS;

    starttime = micros();
    for(size_t i = 0; i < 3; i++)
        CAN_TX_ISR();
    uint32_t canTXISRLength = (micros() - starttime)/3;

    Serial.println("----Results of Profiling----");
    Serial.println(("Scan Key Task           - " + std::to_string(scanKeyTaskLength)).c_str());
    Serial.println(("Display Update Task     - " + std::to_string(displayUpdateTaskLength)).c_str());
    Serial.println(("Message Decode Task     - " + std::to_string(decodeTaskLength)).c_str());
    Serial.println(("Message Transmit Task   - " + std::to_string(transmitTaskLength)).c_str());
    Serial.println(("Sample ISR Length       - " + std::to_string(sampleISRLength)).c_str());
    Serial.println(("CAN Tx ISR Length       - " + std::to_string(canTXISRLength)).c_str());

#endif
}

void loop() {

#ifndef PROFILING
    static size_t i = 0;
    if(i==100000){
        i=0;
        Serial.println(uxTaskGetStackHighWaterMark(scanKeysHandler));
        Serial.println(uxTaskGetStackHighWaterMark(displayUpdateHandler));
        Serial.println(uxTaskGetStackHighWaterMark(decodeHandler));
        Serial.println(uxTaskGetStackHighWaterMark(transmitHandler));
        Serial.println("");
    }
    else{
        i++;
    }
#endif
}

void sampleISR() {
    int32_t vOut = soundGenerator.getSound();
    analogWrite(OUTR_PIN, vOut + 128);
}

void CAN_TX_ISR() {
#ifdef PROFILING
    xSemaphoreGive(CAN_TX_Semaphore);
#else 
    xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
#endif
}