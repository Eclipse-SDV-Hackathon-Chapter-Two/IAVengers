/*
 * Teensy Board GamePad
 * */

#include "Arduino.h"
#include "can_fd_training.h"
#include <FlexCAN_T4.h>

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can3;
TRAINING t(1);

IntervalTimer timer;

const uint32_t CFG_CAN_MSG_Teensy_01_ID = 0x100;
const uint32_t CFG_CAN_MSG_Teensy_01_CYCLE = 100;
const uint32_t CFG_CAN_MSG_Teensy_01_CYCLE_MIN = 50;

const uint32_t CFG_SW_DEBOUNCE_DELAY = 50;

uint8_t input_pot1;
uint8_t input_pot2;
uint8_t input_pot3;
uint8_t input_pot4;

uint8_t input_sw1;
uint8_t input_sw2;
uint8_t input_sw3;
uint8_t input_sw4;

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

void setup();
void loop();
void can_send();
void can_print_msg(const CAN_message_t &msg);

void setup()
{
  t.begin();
  t.flash_led();

  Serial.begin(115200);

  Serial.println("Teensy 4.0 CAN Training board demo. Classic CAN test. www.skpang.co.uk Nov 2020");

  can3.begin();
  can3.setBaudRate(500000);
  can3.setMaxMB(16);
  can3.enableFIFO();
  can3.enableFIFOInterrupt();
  can3.onReceive(can_print_msg);
  can3.mailboxStatus();

  // Send frame every 500ms
  timer.begin(can_send, 100);
}

bool led_on = false;
uint32_t timer_1s_last = 0;
uint32_t can_msg_Teensy_01_last = 0;
uint8_t can_msg_Teensy_01_counter = 0;
bool can_msg_Teensy_01_event = false;

void can_send()
{

  bool min_cycle_time_elapsed = millis() - can_msg_Teensy_01_last > CFG_CAN_MSG_Teensy_01_CYCLE_MIN;
  bool cycle_time_elapsed = millis() - can_msg_Teensy_01_last > CFG_CAN_MSG_Teensy_01_CYCLE;
  if (can_msg_Teensy_01_event || (min_cycle_time_elapsed && cycle_time_elapsed))
  {
    can_msg_Teensy_01_last = millis();
    can_msg_Teensy_01_event = false;

    CAN_message_t msg;
    msg.seq = 1;
    msg.id = CFG_CAN_MSG_Teensy_01_ID;
    msg.len = 8;

    msg.buf[0] = input_pot1;
    msg.buf[1] = input_pot2;
    msg.buf[2] = input_pot3;
    msg.buf[3] = input_pot4;

    msg.buf[4] = sw1_last_value<<4 | sw2_last_value;
    msg.buf[5] = sw3_last_value<<4 | sw4_last_value;

    msg.buf[6] = 0;

    // msg.buf[4] = sw1_last_value;
    // msg.buf[5] = sw2_last_value;
    // msg.buf[6] = sw3_last_value;
    // msg.buf[7] = sw4_last_value;

    msg.buf[7] = can_msg_Teensy_01_counter;

    can3.write(MB15, msg);

    t.set_rgb(GREEN, HIGH);
    timer_1s_last = millis();

    can_msg_Teensy_01_counter++;
    if (can_msg_Teensy_01_counter > 15)
    {
      can_msg_Teensy_01_counter = 0;
    }
  }
}

void loop()
{

  can3.events();

  uint8_t pot1 = t.read_an1();
  uint8_t pot2 = t.read_an2();
  uint8_t pot3 = t.read_an3();
  uint8_t pot4 = t.read_an4();

  sw1_value = t.read_sw1();
  sw2_value = t.read_sw2();
  sw3_value = t.read_sw3();

  uint32_t sw1_last_time_diff = millis() - sw1_last_time;
  uint32_t sw2_last_time_diff = millis() - sw2_last_time;
  uint32_t sw3_last_time_diff = millis() - sw3_last_time;

  if (sw1_last_time_diff > CFG_SW_DEBOUNCE_DELAY && sw1_value != sw1_last_value)
  {
    sw1_last_value = sw1_value;
    can_msg_Teensy_01_event = true;

    if (sw1_value == LOW)
    {
      Serial.println(" SW1: Push");
    }
  }

  if (sw2_last_time_diff > 50 && sw2_value != sw2_last_value)
  {
    sw2_last_value = sw2_value;
    can_msg_Teensy_01_event = true;

    if (sw2_value == LOW)
    {
      Serial.println(" sw2: Push");
    }
  }

  if (sw3_last_time_diff > CFG_SW_DEBOUNCE_DELAY && sw3_value != sw3_last_value)
  {
    sw3_last_value = sw3_value;
    can_msg_Teensy_01_event = true;

    if (sw3_value == LOW)
    {
      Serial.println(" sw3: Push");
    }
  }


  input_pot1 = pot1;
  input_pot2 = pot2;
  input_pot3 = pot3;
  input_pot4 = pot4;

  uint32_t sw4_last_time_diff = millis() - sw4_last_time;
  sw4_value = t.read_sw4();

  if (sw4_last_time_diff > CFG_SW_DEBOUNCE_DELAY && sw4_value != sw4_last_value)
  {
    sw4_last_value = sw4_value;
    can_msg_Teensy_01_event = true;

    if (sw4_value == LOW)
    {
      Serial.println(" sw4: Push");
    }
  }

  if (millis() - timer_1s_last > 10)
  {
    timer_1s_last = millis();
    t.set_rgb(GREEN, LOW);
  }
}

void can_print_msg(const CAN_message_t &msg)
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