#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <string.h>

//Constants
const uint32_t interval = 100; //Display update interval

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

volatile int32_t currentStepSize;

volatile uint8_t keyArray[7];

const static int32_t STEPSIZES[] = {int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -9. / 12.) / 22000.),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -8. / 12.) / 22000.0),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -7. / 12.) / 22000.0),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -6. / 12.) / 22000.0),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -5. / 12.) / 22000.0),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -4. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -3. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -2. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -1. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., 0. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., 1. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2.0, 2. / 12.) / 22000.), int32_t(0.0)};
static const char* NOTES [] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "No Key"};

void setRow(uint8_t rowIdx);

uint8_t readCols();

inline uint8_t decode_to_idx(uint16_t bits);

void sampleISR();

[[noreturn]] void scanKeysTask(void * pvParameters);
[[noreturn]] void displayUpdateTask(void * pvParameters);

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

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

void setup() {
    // put your setup code here, to run once:

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
    TIM_TypeDef *Instance = TIM1;
    auto *sampleTimer= new HardwareTimer(Instance);
    sampleTimer->setOverflow(22000, HERTZ_FORMAT);
    sampleTimer->attachInterrupt(sampleISR);
    sampleTimer->resume();
    TaskHandle_t scanKeysHandle = NULL;
    xTaskCreate(scanKeysTask,/* Function that implements the task */
                "scanKeys",/* Text name for the task */
                64,/* Stack size in words, not bytes*/
                NULL,/* Parameter passed into the task */
                2,/* Task priority*/
                &scanKeysHandle /* Pointer to store the task handle*/
                );
    xTaskCreate(displayUpdateTask,/* Function that implements the task */
                "displayUpdate",/* Text name for the task */
                256,/* Stack size in words, not bytes*/
                NULL,/* Parameter passed into the task */
                1,/* Task priority*/
                &scanKeysHandle /* Pointer to store the task handle*/
    );
    vTaskStartScheduler();
    //Initialise UART
    Serial.begin(9600);
}


void loop() {
    // put your main code here, to run repeatedly:
}

inline int shift_n(const int PIN, size_t n) {
    return digitalRead(PIN) << n;
}

uint8_t readCols() {
    return shift_n(C0_PIN, 3) + shift_n(C1_PIN, 2) + shift_n(C2_PIN, 1) + shift_n(C3_PIN, 0);
}

void setRow(uint8_t rowIdx) {
    digitalWrite(REN_PIN, LOW);
    digitalWrite(RA0_PIN, rowIdx & 0x1);
    digitalWrite(RA1_PIN, rowIdx & 0x2);
    digitalWrite(RA2_PIN, rowIdx & 0x4);
    digitalWrite(REN_PIN, HIGH);
}

// decodes the hexcode to an idx
inline uint8_t decode_to_idx(uint16_t bits) {
    bits = ~bits;
    for (size_t i = 0; i < 12; i++) {
        if (bits & 1) {
            return 11-i;
        }
        bits >>= 1;
    }
    return 12;
}

void sampleISR() {
    static int32_t phaseAcc= 0;
    phaseAcc+= currentStepSize;
    int32_t Vout = phaseAcc >> 24;
    analogWrite(OUTR_PIN, Vout + 128);
}

void scanKeysTask(void *pvParameters) {
    const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime= xTaskGetTickCount();
    while (true) {
        for (size_t i = 0; i < 3; i++) {
            setRow(i);
            delayMicroseconds(3);
            uint8_t keys = readCols();
            keyArray[i] = keys;
        }
        uint16_t to_be_printed = (keyArray[0] << 4 * 2) + (keyArray[1] << 4 * 1) + keyArray[2];
        __atomic_store_n(&currentStepSize, STEPSIZES[decode_to_idx(to_be_printed)], __ATOMIC_RELAXED);
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void displayUpdateTask(void *pvParameters) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime= xTaskGetTickCount();
    while (true) {
        //Update display
        u8g2.clearBuffer();         // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        uint16_t pressed_key_hex = (keyArray[0]<<4*2) + (keyArray[1]<<4*1) + keyArray[2];
        u8g2.setCursor(2,10);
        u8g2.print(pressed_key_hex, HEX);
        u8g2.drawStr(2,20, NOTES[decode_to_idx(pressed_key_hex)]);
        u8g2.sendBuffer();          // transfer internal memory to the display

        //Toggle LED
        digitalToggle(LED_BUILTIN);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
