// - - - - -
// DMXSerial - A Arduino library for sending and receiving DMX using the builtin serial hardware port.
// DMXSerial.cpp: Library implementation file
//
// Copyright (c) 2011-2020 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
//
// Documentation and samples are available at http://www.mathertel.de/Arduino
// Changelog: See DMXSerial.h
// - - - - -

#include "Arduino.h"
#include "DMXSerial.h"


// ----- forwards -----

void _DMXStartSending();
void _DMXStartReceiving();

// register interrupt for receiving data and frameerrors that calls _DMXReceived()
void _DMXReceived(uint8_t data, uint8_t frameerror);
void _DMXTransmitted();


// These functions all exist in the processor specific implementations:
// void _DMX_init();
// void _DMX_setMode();
// void _DMX_writeByte(uint8_t data);
// void _DMX_flush();


// ----- Serial UART Modes -----

// There are 4 different modes required while receiving and sending DMX using the Serial

// State of receiving DMX Bytes
typedef enum {
  OFF = 0, // Turn off
  RONLY = 1, // Receive DMX data only
  RDATA = 2, // Receive DMX data + Interrupt
  TBREAK = 3, // Transmit DMX Break + Interrupt on Transmission completed
  TDATA = 4, // Transmit DMX Data + Interrupt on Register empty
  TDONE = 5 // Transmit DMX Data + Interrupt on Transmission completed
} __attribute__((packed)) DMXUARTMode;


// Baud rate for DMX protocol
#define DMXSPEED 250000L

// the break timing is 10 bits (start + 8 data + 1 (even) parity) of this speed
// the mark-after-break is 1 bit of this speed plus approx 6 usec
// 100000 bit/sec is good: gives 100 usec break and 16 usec MAB
// 1990 spec says transmitter must send >= 92 usec break and >= 12 usec MAB
// receiver must accept 88 us break and 8 us MAB
#define BREAKSPEED 100000L

#define BREAKFORMAT SERIAL_8E2
#define DMXFORMAT SERIAL_8N2
#define DMXREADFORMAT SERIAL_8N1


// ----- include processor specific definitions and functions.

#if defined(ARDUINO_ARCH_AVR)
#include "DMXSerial_avr.h"

#elif defined(ARDUINO_ARCH_MEGAAVR)
#include "DMXSerial_megaavr.h"

#endif

// ----- Enumerations -----

// State of receiving DMX Bytes
typedef enum {
  STARTUP = 1, // wait for any interrupt BEFORE starting analyzing the DMX protocoll.
  IDLE = 2, // wait for a BREAK condition.
  BREAK = 3, // BREAK was detected.
  DATA = 4, // DMX data.
  DONE = 5 // All channels received.
} __attribute__((packed)) DMXReceivingState;


// ----- DMXSerial Private variables -----
// These variables are not class members because they have to be reached by the interrupt implementations.
// don't use these variable from outside, use the appropriate methods.

DMXMode _dmxMode; // Mode of Operation
int _dmxModePin; // pin used for I/O direction.

uint8_t _dmxRecvState; // Current State of receiving DMX Bytes
int _dmxChannel; // the next channel byte to be sent.

volatile unsigned int _dmxMaxChannel = 32; // the last channel used for sending (1..32).
volatile unsigned long _dmxLastPacket = 0; // the last time (using the millis function) a packet was received.

bool _dmxUpdated = true; // is set to true when new data arrived.

// Array of DMX values (raw).
// Entry 0 will never be used for DMX data but will store the startbyte (0 for DMX mode).
uint8_t _dmxData[DMXSERIAL_MAX + 1];

// This pointer will point to the next byte in _dmxData;
uint8_t *_dmxDataPtr;

// This pointer will point to the last byte in _dmxData;
uint8_t *_dmxDataLastPtr;

// Create a single class instance. Multiple class instances (multiple simultaneous DMX ports) are not supported.
DMXSerialClass DMXSerial;



#define _MMIO_BYTE(mem_addr) (*(volatile uint8_t *)(mem_addr))

#define _SFR_MEM8(mem_addr) _MMIO_BYTE(mem_addr)


#define OCR1BL _SFR_MEM8(0x8A)
#define OCR1BL0 0
#define OCR1BL1 1
#define OCR1BL2 2
#define OCR1BL3 3
#define OCR1BL4 4
#define OCR1BL5 5
#define OCR1BL6 6
#define OCR1BL7 7

#define OCR1BH _SFR_MEM8(0x8B)
#define OCR1BH0 0
#define OCR1BH1 1
#define OCR1BH2 2
#define OCR1BH3 3
#define OCR1BH4 4
#define OCR1BH5 5
#define OCR1BH6 6
#define OCR1BH7 7

#define TCCR2A _SFR_MEM8(0xB0)
#define WGM20 0
#define WGM21 1
#define COM2B0 4
#define COM2B1 5
#define COM2A0 6
#define COM2A1 7

#define TCCR2B _SFR_MEM8(0xB1)
#define CS20 0
#define CS21 1
#define CS22 2
#define WGM22 3
#define FOC2B 6
#define FOC2A 7

#define TCNT2 _SFR_MEM8(0xB2)
#define TCNT2_0 0
#define TCNT2_1 1
#define TCNT2_2 2
#define TCNT2_3 3
#define TCNT2_4 4
#define TCNT2_5 5
#define TCNT2_6 6
#define TCNT2_7 7

#define OCR2A _SFR_MEM8(0xB3)
#define OCR2_0 0
#define OCR2_1 1
#define OCR2_2 2
#define OCR2_3 3
#define OCR2_4 4
#define OCR2_5 5
#define OCR2_6 6
#define OCR2_7 7

#define OCR2B _SFR_MEM8(0xB4)
#define OCR2_0 0
#define OCR2_1 1
#define OCR2_2 2
#define OCR2_3 3
#define OCR2_4 4
#define OCR2_5 5
#define OCR2_6 6
#define OCR2_7 7

#define ASSR _SFR_MEM8(0xB6)
#define TCR2BUB 0
#define TCR2AUB 1
#define OCR2BUB 2
#define OCR2AUB 3
#define TCN2UB 4
#define AS2 5
#define EXCLK 6

#define TWBR _SFR_MEM8(0xB8)
#define TWBR0 0
#define TWBR1 1
#define TWBR2 2
#define TWBR3 3
#define TWBR4 4
#define TWBR5 5
#define TWBR6 6
#define TWBR7 7

#define TWSR _SFR_MEM8(0xB9)
#define TWPS0 0
#define TWPS1 1
#define TWS3 3
#define TWS4 4
#define TWS5 5
#define TWS6 6
#define TWS7 7

#define TWAR _SFR_MEM8(0xBA)
#define TWGCE 0
#define TWA0 1
#define TWA1 2
#define TWA2 3
#define TWA3 4
#define TWA4 5
#define TWA5 6
#define TWA6 7

#define TWDR _SFR_MEM8(0xBB)
#define TWD0 0
#define TWD1 1
#define TWD2 2
#define TWD3 3
#define TWD4 4
#define TWD5 5
#define TWD6 6
#define TWD7 7

#define TWCR _SFR_MEM8(0xBC)
#define TWIE 0
#define TWEN 2
#define TWWC 3
#define TWSTO 4
#define TWSTA 5
#define TWEA 6
#define TWINT 7

#define TWAMR _SFR_MEM8(0xBD)
#define TWAM0 0
#define TWAM1 1
#define TWAM2 2
#define TWAM3 3
#define TWAM4 4
#define TWAM5 5
#define TWAM6 6

#define UCSR0A _SFR_MEM8(0xC0)
#define MPCM0 0
#define U2X0 1
#define UPE0 2
#define DOR0 3
#define FE0 4
#define UDRE0 5
#define TXC0 6
#define RXC0 7

#define UCSR0B _SFR_MEM8(0xC1)
#define TXB80 0
#define RXB80 1
#define UCSZ02 2
#define TXEN0 3
#define RXEN0 4
#define UDRIE0 5
#define TXCIE0 6
#define RXCIE0 7

#define UCSR0C _SFR_MEM8(0xC2)
#define UCPOL0 0
#define UCSZ00 1
#define UCPHA0 1
#define UCSZ01 2
#define UDORD0 2
#define USBS0 3
#define UPM00 4
#define UPM01 5
#define UMSEL00 6
#define UMSEL01 7

#define UBRR0L _SFR_MEM8(0xC4)
#define UBRR0_0 0
#define UBRR0_1 1
#define UBRR0_2 2
#define UBRR0_3 3
#define UBRR0_4 4
#define UBRR0_5 5
#define UBRR0_6 6
#define UBRR0_7 7

#define UBRR0H _SFR_MEM8(0xC5)
#define UBRR0_8 0
#define UBRR0_9 1
#define UBRR0_10 2
#define UBRR0_11 3

#define UDR0 _SFR_MEM8(0xC6)
#define UDR0_0 0
#define UDR0_1 1
#define UDR0_2 2
#define UDR0_3 3
#define UDR0_4 4
#define UDR0_5 5
#define UDR0_6 6
#define UDR0_7 7

#define UCSRnA UCSR0A
#define RXCn RXC0
#define TXCn TXC0
#define UCSRnB UCSR0B
#define RXCIEn RXCIE0
#define TXCIEn TXCIE0
#define UDRIEn UDRIE0
#define RXENn RXEN0
#define TXENn TXEN0
#define UCSRnC UCSR0C
#define USBSn USBS0
#define UCSZn0 UCSZ00
#define UPMn0 UPM00
#define UBRRnH UBRR0H
#define UBRRnL UBRR0L
#define UDRn UDR0
#define UDREn UDRE0
#define FEn FE0
#define USARTn_RX_vect USART_RX_vect
#define USARTn_TX_vect USART_TX_vect
#define USARTn_UDRE_vect USART_UDRE_vect

#define CalcPreScale(B) (((((F_CPU) / 8) / (B)) - 1) / 2)

const int32_t _DMX_dmxPreScale = CalcPreScale(DMXSPEED); // BAUD prescale factor for DMX speed.
const int32_t _DMX_breakPreScale = CalcPreScale(BREAKSPEED); // BAUD prescale factor for BREAK speed.

// initialize mode independent registers.
void _DMX_init()
{
  // 04.06.2012: use normal speed operation
  UCSRnA = 0;
} // _DMX_init()

/// Initialize the Hardware UART serial port registers to the required mode.
void _DMX_setMode(DMXUARTMode mode)
{
  if (mode == DMXUARTMode::OFF) {
    UCSRnB = 0;

  } else if (mode == DMXUARTMode::RONLY) {
    // assign the baud_setting to the USART Baud Rate Register
    UBRRnH = _DMX_dmxPreScale >> 8;
    UBRRnL = _DMX_dmxPreScale;
    // enable USART functions RX, TX, Interrupts
    UCSRnB = (1 << RXENn);
    // stop bits and character size
    UCSRnC = DMXREADFORMAT; // accept data packets after first stop bit

  } else if (mode == DMXUARTMode::RDATA) {
    UBRRnH = _DMX_dmxPreScale >> 8;
    UBRRnL = _DMX_dmxPreScale;
    UCSRnB = (1 << RXENn) | (1 << RXCIEn);
    UCSRnC = DMXREADFORMAT; // accept data packets after first stop bit

  } else if (mode == DMXUARTMode::TBREAK) {
    UBRRnH = _DMX_breakPreScale >> 8;
    UBRRnL = _DMX_breakPreScale;
    UCSRnB = ((1 << TXENn) | (1 << TXCIEn));
    UCSRnC = BREAKFORMAT;

  } else if (mode == DMXUARTMode::TDATA) {
    UBRRnH = _DMX_dmxPreScale >> 8;
    UBRRnL = _DMX_dmxPreScale;
    UCSRnB = ((1 << TXENn) | (1 << UDRIEn));
    UCSRnC = DMXFORMAT; // send with 2 stop bits for compatibility

  } else if (mode == DMXUARTMode::TDONE) {
    UBRRnH = _DMX_dmxPreScale >> 8;
    UBRRnL = _DMX_dmxPreScale;
    UCSRnB = ((1 << TXENn) | (1 << TXCIEn));
    UCSRnC = DMXFORMAT; // send with 2 stop bits for compatibility
  } // if
} // _DMX_setMode()

inline void _DMX_writeByte(uint8_t data)
{
  // putting data into buffer sends the data
  UDRn = data;
} // _DMX_writeByte

// flush all incomming data packets in the queue
void _DMX_flush()
{
  uint8_t voiddata;
  while (UCSRnA & (1 << RXCn)) {
    voiddata = UDRn; // get data
  }
}

// ----- Class implementation -----

// Initialize the specified mode.
void DMXSerialClass::init(int mode)
{
  init(mode, DMXMODEPIN);
}

// (Re)Initialize the specified mode.
// The mode parameter should be a value from enum DMXMode.
void DMXSerialClass::init(int mode, int dmxModePin)
{
  // initialize global variables
  _dmxMode = DMXNone;
  _dmxModePin = dmxModePin;
  _dmxRecvState = STARTUP; // initial state
  _dmxChannel = 0;
  _dmxDataPtr = _dmxData;
  _dmxLastPacket = millis(); // remember current (relative) time in msecs.

  _dmxMaxChannel = DMXSERIAL_MAX; // The default in Receiver mode is reading all possible 512 channels.
  _dmxDataLastPtr = _dmxData + _dmxMaxChannel;

  // initialize the DMX buffer
  //  memset(_dmxData, 0, sizeof(_dmxData));
  for (int n = 0; n < DMXSERIAL_MAX + 1; n++)
    _dmxData[n] = 0;

  // now start
  _dmxMode = (DMXMode)mode;

  if ((_dmxMode == DMXController) || (_dmxMode == DMXReceiver) || (_dmxMode == DMXProbe)) {
    // a valid mode was given
    // Setup external mode signal
    _DMX_init();

    pinMode(_dmxModePin, OUTPUT); // enables the pin for output to control data direction
    digitalWrite(_dmxModePin, DmxModeIn); // data in direction, to avoid problems on the DMX line for now.

    if (_dmxMode == DMXController) {
      digitalWrite(_dmxModePin, DmxModeOut); // data Out direction
      _dmxMaxChannel = 32; // The default in Controller mode is sending 32 channels.
      _DMXStartSending();

    } else if (_dmxMode == DMXReceiver) {
      // Setup Hardware
      _DMXStartReceiving();

      // } else if (_dmxMode == DMXProbe) {
      //   // don't setup the Hardware now

    } // if
  } // if
} // init()


// Set the maximum used channel.
// This method can be called any time before or after the init() method.
void DMXSerialClass::maxChannel(int channel)
{
  if (channel < 1)
    channel = 1;
  if (channel > DMXSERIAL_MAX)
    channel = DMXSERIAL_MAX;
  _dmxMaxChannel = channel;
  _dmxDataLastPtr = _dmxData + channel;
} // maxChannel


// Read the current value of a channel.
uint8_t DMXSerialClass::read(int channel)
{
  // adjust parameter
  if (channel < 1)
    channel = 1;
  if (channel > DMXSERIAL_MAX)
    channel = DMXSERIAL_MAX;
  // read value from buffer
  return (_dmxData[channel]);
} // read()


// Write the value into the channel.
// The value is just stored in the sending buffer and will be picked up
// by the DMX sending interrupt routine.
void DMXSerialClass::write(int channel, uint8_t value)
{
  // adjust parameters
  if (channel < 1)
    channel = 1;
  if (channel > DMXSERIAL_MAX)
    channel = DMXSERIAL_MAX;
  if (value < 0)
    value = 0;
  if (value > 255)
    value = 255;

  // store value for later sending
  _dmxData[channel] = value;

  // Make sure we transmit enough channels for the ones used
  if (channel > _dmxMaxChannel) {
    _dmxMaxChannel = channel;
    _dmxDataLastPtr = _dmxData + _dmxMaxChannel;
  } // if
} // write()


// Return the DMX buffer for un-save direct but faster access
uint8_t *DMXSerialClass::getBuffer()
{
  return (_dmxData);
} // getBuffer()


// Calculate how long no data packet was received
unsigned long DMXSerialClass::noDataSince()
{
  unsigned long now = millis();
  return (now - _dmxLastPacket);
} // noDataSince()


// return true when some DMX data was updated.
bool DMXSerialClass::dataUpdated()
{
  return (_dmxUpdated);
}


// reset DMX data update flag.
void DMXSerialClass::resetUpdated()
{
  _dmxUpdated = false;
}


// When mode is DMXProbe this function reads one DMX buffer and then returns.
// wait a meaningful time on a packet.
// return true when a packet has been received.
bool DMXSerialClass::receive()
{
  return (receive(DMXPROBE_RECV_MAX));
} // receive()


// When mode is DMXProbe this function reads one DMX buffer and then returns.
// wait approximately gives the number of msecs for waiting on a packet.
// return true when a packet has been received.
bool DMXSerialClass::receive(uint8_t wait)
{
  bool ret = false;

  if (_dmxMode == DMXProbe) {
    _DMXStartReceiving();
    // UCSRnA
    while ((wait > 0) && (_dmxRecvState != DONE)) {
      delay(1);
      wait--;
    } // while

    if (_dmxRecvState == DONE) {
      ret = true;
    } else {
      _DMX_setMode(DMXUARTMode::RONLY);
    } // if
  } // if

  return (ret);
} // receive(wait)


// Terminate operation
void DMXSerialClass::term(void)
{
  // Disable all USART Features, including Interrupts
  _DMX_setMode(DMXUARTMode::OFF);
} // term()


// ----- internal functions and interrupt implementations -----


// Setup Hardware for Sending
void _DMXStartSending()
{
  // Start sending a BREAK and send more bytes in UDRE ISR
  // Enable transmitter and interrupt
  _DMX_setMode(DMXUARTMode::TBREAK);
  _DMX_writeByte((uint8_t)0);
} // _DMXStartSending()


// Setup Hardware for Receiving
void _DMXStartReceiving()
{
  uint8_t voiddata;

  // Enable receiver and Receive interrupt
  _dmxDataPtr = _dmxData;
  _dmxRecvState = STARTUP;

  _DMX_setMode(DMXUARTMode::RDATA);
  _DMX_flush();
} // _DMXStartReceiving()


// This function is called by the Interrupt Service Routine when a byte or frame error was received.
// In DMXController mode this interrupt is disabled and will not occur.
// In DMXReceiver mode when a byte was received it is stored to the dmxData buffer.
void _DMXReceived(uint8_t data, uint8_t frameerror)
{
  uint8_t DmxState = _dmxRecvState; //just load once from SRAM to increase speed

  if (DmxState == STARTUP) {
    // just ignore any first frame comming in
    _dmxRecvState = IDLE;
    return;
  }

  if (frameerror) { //check for break
    // break condition detected.
    _dmxRecvState = BREAK;
    _dmxDataPtr = _dmxData;

  } else if (DmxState == BREAK) {
    // first byte after a break was read.
    if (data == 0) {
      // normal DMX start code (0) detected
      _dmxRecvState = DATA;
      _dmxLastPacket = millis(); // remember current (relative) time in msecs.
      _dmxDataPtr++; // start saving data with channel # 1

    } else {
      // This might be a RDM or customer DMX command -> not implemented so wait for next BREAK !
      _dmxRecvState = DONE;
    } // if

  } else if (DmxState == DATA) {
    // check for new data
    if (*_dmxDataPtr != data) {
      _dmxUpdated = true;
      // store received data into dmx data buffer.
      *_dmxDataPtr = data;
    } // if
    _dmxDataPtr++;

    if (_dmxDataPtr > _dmxDataLastPtr) {
      // all channels received.
      _dmxRecvState = DONE;
    } // if
  } // if

  if (_dmxRecvState == DONE) {
    if (_dmxMode == DMXProbe) {
      // stop creating interrupts on the serial port for now.
      _DMX_setMode(DMXUARTMode::RONLY);

    } else {
      // continue on DMXReceiver mode.
      _dmxRecvState = IDLE; // wait for next break
    }
  } // if

} // _DMXReceived()


// This function is called by the Transmission complete or Data Register empty interrupt routine.
// When changing speed (after sending BREAK) we use TX finished interrupt that occurs shortly after the last stop bit is sent
// When staying at the same speed (sending data bytes) we use data register empty interrupt that occurs shortly after the start bit of the *previous* byte
// When sending a DMX sequence it just takes the next channel byte and sends it out.
// In DMXController mode when the buffer was sent completely the DMX sequence will resent, starting with a BREAK pattern.
// In DMXReceiver mode this interrupt is disabled and will not occur.
void _DMXTransmitted()
{
  if ((_dmxMode == DMXController) && (_dmxChannel == -1)) {
    // this occurs after the stop bits of the last data byte
    // start sending a BREAK and loop forever in ISR
    _DMX_setMode(DMXUARTMode::TBREAK);
    _DMX_writeByte((uint8_t)0);
    _dmxChannel = 0; // next time send start byte

  } else if (_dmxChannel == 0) {
    // this occurs after the stop bits of the break byte
    // now back to DMX speed: 250000baud
    // take next interrupt when data register empty (early)
    _DMX_setMode(DMXUARTMode::TDATA);

    // write start code
    _DMX_writeByte((uint8_t)0);
    _dmxChannel = 1;

  } else {
    if (_dmxChannel < _dmxMaxChannel) {
      // just send the next data
      _DMX_writeByte(_dmxData[_dmxChannel++]);
    } else {
      // last data
      _DMX_setMode(DMXUARTMode::TDONE);
      _DMX_writeByte(_dmxData[_dmxChannel]);
      _dmxChannel = -1; // this series is done. Next time: restart with break.
    } // if

  } // if
} // _DMXTransmitted


// The End
