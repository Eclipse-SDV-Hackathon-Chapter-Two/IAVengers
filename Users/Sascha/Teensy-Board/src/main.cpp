/*
 * Demo sketch for use with:
 * http://skpang.co.uk/catalog/teensy-40-can-fd-board-with-240x240-ips-lcd-and-usd-holder-p-1584.html
 *
 * Classic CAN demo
 * Nominal baudrate 500kbps
 * CAN FD data baudrate 2000kbps
 *
 * www.skpang.co.uk Jan 2020
 *
 *
 * */

#include "Arduino.h"
#include "can_fd_training.h"
#include <FlexCAN_T4.h>

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
TRAINING t(1);

IntervalTimer timer;

const uint32_t ESC_01_ID = 0x100;
const uint32_t ASG_01_ID = 0x101;
const uint32_t BMS_01_ID = 0x102;

uint8_t pot1;
uint8_t pot2;
uint8_t pot3;
uint8_t pot4;

uint8_t sw1;
uint8_t sw2;
uint8_t sw3;
uint8_t sw4;

void sendframe();
void loop();
void canSniff(const CAN_message_t &msg);

void setup(void)
{
  t.begin(); // Initialise board
  t.flash_led();

  Serial.begin(115200);

  Serial.println("Teensy 4.0 CAN Training board demo. Classic CAN test. www.skpang.co.uk Nov 2020");

  can3.begin();
  can3.setBaudRate(500000); // Data rate 500kbps
  can3.setMaxMB(16);
  can3.enableFIFO();
  can3.enableFIFOInterrupt();
  can3.onReceive(canSniff);
  can3.mailboxStatus();

  timer.begin(sendframe, 1000); // Send frame every 500ms
}

uint8_t counter_0 = 0;

bool led_on = false;
uint32_t timer_1s_last = 0;
uint32_t ESC_01_last = 0;
uint8_t ESC_01_Counter = 0;
bool ESC_01_Signal_Error = false;

uint32_t ASG_01_last = 0;
uint8_t ASG_01_Counter = 0;
bool ASG_01_Signal_Error = false;

uint32_t BMS_01_last = 0;
uint8_t BMS_01_Counter = 0;
bool BMS_01_Signal_Error = false;

void sendframe()
{
  CAN_message_t msg;
  msg.seq = 1;

  // msg.id = 0x7df;

  // msg.buf[0] = pot1;
  // msg.buf[1] = pot2;
  // msg.buf[2] = pot3;
  // msg.buf[3] = pot4;
  // msg.buf[4] = 0x00;
  // msg.buf[5] = 0x00;
  // msg.buf[6] = counter_0;
  // msg.buf[7] = 0xff;

  // msg.len = 8;
  // msg.seq = 1;
  // can3.write(MB15, msg);

  if (millis() - ESC_01_last > 100)
  {
    ESC_01_last = millis();

    // ESC_01
    msg.id = ESC_01_ID;
    msg.len = 8;
    msg.buf[0] = pot1;
    msg.buf[1] = ESC_01_Signal_Error;
    msg.buf[2] = 0x00;
    msg.buf[3] = 0x00;
    msg.buf[4] = 0x00;
    msg.buf[5] = 0x00;
    msg.buf[6] = 0x00;
    msg.buf[7] = ESC_01_Counter;

    can3.write(MB15, msg);

    ESC_01_Counter++;
    if (ESC_01_Counter > 15)
    {
      ESC_01_Counter = 0;
    }
  }

  if (millis() - ASG_01_last > (uint32_t)pot4 + 100)
  {
    ASG_01_last = millis();

    // ASG_01
    msg.id = ASG_01_ID;
    msg.len = 8;
    msg.buf[0] = pot2;
    msg.buf[1] = ASG_01_Signal_Error;
    msg.buf[2] = 0x00;
    msg.buf[3] = 0x00;
    msg.buf[4] = 0x00;
    msg.buf[5] = 0x00;
    msg.buf[6] = 0x00;
    msg.buf[7] = ASG_01_Counter;

    can3.write(MB15, msg);
    ASG_01_Counter++;

    if (ASG_01_Counter > 15)
    {
      ASG_01_Counter = 0;
    }
  }

  if (millis() - BMS_01_last > (uint32_t)pot4 + 100)
  {
    BMS_01_last = millis();

    // BMS_01
    msg.id = BMS_01_ID;
    msg.len = 8;
    msg.buf[0] = pot3;
    msg.buf[1] = BMS_01_Signal_Error;
    msg.buf[2] = 0x00;
    msg.buf[3] = 0x00;
    msg.buf[4] = 0x00;
    msg.buf[5] = 0x00;
    msg.buf[6] = 0x00;
    msg.buf[7] = BMS_01_Counter;

    can3.write(MB15, msg);

    BMS_01_Counter++;
    if (BMS_01_Counter > 15)
    {
      BMS_01_Counter = 0;
    }
  }
}

uint32_t sw1_value = 0;
uint32_t sw1_last_value = 0;
uint32_t sw1_last_time = 0;

uint32_t sw2_value = 0;
uint32_t sw2_last_value = 0;
uint32_t sw2_last_time = 0;

uint32_t sw3_value = 0;
uint32_t sw3_last_value = 0;
uint32_t sw3_last_time = 0;

uint32_t sw4_value = 0;
uint32_t sw4_last_value = 0;
uint32_t sw4_last_time = 0;

void loop()
{

  can3.events();

  pot1 = t.read_an1();
  pot2 = t.read_an2();
  pot3 = t.read_an3();
  pot4 = t.read_an4();

  uint32_t sw1_last_time_diff = millis() - sw1_last_time;
  sw1_value = t.read_sw1();

  if (sw1_last_time_diff > 500 && sw1_value != sw1_last_value)
  {
    sw1_last_value = sw1_value;

    if (sw1_value == LOW)
    {
      Serial.println(" SW1: Push");
      ESC_01_Signal_Error = !ESC_01_Signal_Error;
    }
  }

  uint32_t sw2_last_time_diff = millis() - sw2_last_time;
  sw2_value = t.read_sw2();

  if (sw2_last_time_diff > 500 && sw2_value != sw2_last_value)
  {
    sw2_last_value = sw2_value;

    if (sw2_value == LOW)
    {
      Serial.println(" sw2: Push");
      ASG_01_Signal_Error = !ASG_01_Signal_Error;
      Serial.println(ASG_01_Signal_Error);
    }
  }

  uint32_t sw3_last_time_diff = millis() - sw3_last_time;
  sw3_value = t.read_sw3();

  if (sw3_last_time_diff > 500 && sw3_value != sw3_last_value)
  {
    sw3_last_value = sw3_value;

    if (sw3_value == LOW)
    {
      Serial.println(" sw3: Push");
      BMS_01_Signal_Error = !BMS_01_Signal_Error;
    }
  }

  uint32_t sw4_last_time_diff = millis() - sw4_last_time;
  sw4_value = t.read_sw4();

  if (sw4_last_time_diff > 500 && sw4_value != sw4_last_value)
  {
    sw4_last_value = sw4_value;

    if (sw4_value == LOW)
    {
      Serial.println(" sw4: Push");
    }
  }

  // Serial.print("Pot values:  ");
  // Serial.print(pot1);
  // Serial.print("  ");

  // Serial.print(pot2);
  // Serial.print("  ");

  // Serial.print(pot3);
  // Serial.print("  ");

  // Serial.print(pot4);
  // Serial.print("   Switch values ");

  if (millis() - timer_1s_last > 1000)
  {
    timer_1s_last = millis();

    if (led_on)
    {
      t.set_rgb(GREEN, HIGH);
    }
    else
    {

      t.set_rgb(GREEN, LOW);
    }

    led_on = !led_on;
  }
}

void canSniff(const CAN_message_t &msg)
{
  Serial.print("MB ");
  Serial.print(msg.mb);
  Serial.print("  OVERRUN: ");
  Serial.print(msg.flags.overrun);
  Serial.print("  LEN: ");
  Serial.print(msg.len);
  Serial.print(" EXT: ");
  Serial.print(msg.flags.extended);
  Serial.print(" TS: ");
  Serial.print(msg.timestamp);
  Serial.print(" ID: ");
  Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for (uint8_t i = 0; i < msg.len; i++)
  {
    Serial.print(msg.buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}