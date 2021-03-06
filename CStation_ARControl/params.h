/* RTC and clock display pins & params */
#define TM_CLK 22
#define TM_DIO 23

#define DS1302_GND_PIN 24
#define DS1302_VCC_PIN 25
#define DS1302_RST_PIN 26
#define DS1302_DAT_PIN 28
#define DS1302_CLK_PIN 30

#define CLOCK_DELAY_MS 500
/* /RTC and clock display pins & params */


/* other pins */
#define IR_RECV_PIN 13

#define RFID_RST_PIN 5
#define RFID_SS_PIN 53
/* /other pins */


// Mega2560 interrupt pins: 2, 3, 18, 19, 20, 21
/* movement sensor */
#define HC_PIN 18
#define HC_INTERRUPT_MODE RISING
/* /movement sensor */

enum ElightControl {
  LIGHT_NOP, LIGHT_ON, LIGHT_OFF, LIGHT_B_DOWN, LIGHT_B_DOWN2, LIGHT_B_UP, LIGHT_B_UP2, 
  LIGHT_R, LIGHT_G, LIGHT_B, LIGHT_W, LIGHT_COL11, LIGHT_COL21, LIGHT_COL31, LIGHT_COL41, 
  LIGHT_COL12, LIGHT_COL22, LIGHT_COL32, LIGHT_COL42, LIGHT_COL13, LIGHT_COL23, LIGHT_COL33, LIGHT_COL43, 
  LIGHT_MODE_FLASH, LIGHT_MODE_STROBE, LIGHT_MODE_FADE, LIGHT_MODE_SMOOTH
};

/* IR Button codes */
#define CM_LIGHT_ON 0xFFB04F
#define CM_LIGHT_OFF 0xFFF807
#define CM_LIGHT_B_DOWN 0xFFB847
#define CM_LIGHT_B_DOWN2 0xA23C94BF
#define CM_LIGHT_B_UP 0xFF906F
#define CM_LIGHT_B_UP2 0xE5CFBD7F
#define CM_LIGHT_R 0xFF9867
#define CM_LIGHT_G 0xFFD827
#define CM_LIGHT_B 0xFF8877
#define CM_LIGHT_W 0xFFA857
#define CM_LIGHT_COL11 0xFFE817
#define CM_LIGHT_COL21 0xFF02FD
#define CM_LIGHT_COL31 0xFF50AF
#define CM_LIGHT_COL41 0xFF38C7
#define CM_LIGHT_COL12 0xFF48B7
#define CM_LIGHT_COL22 0xFF32CD
#define CM_LIGHT_COL32 0xFF7887
#define CM_LIGHT_COL42 0xFF28D7
#define CM_LIGHT_COL13 0xFF6897
#define CM_LIGHT_COL23 0xFF20DF
#define CM_LIGHT_COL33 0xFF708F
#define CM_LIGHT_COL43 0xFFF00F
#define CM_LIGHT_MODE_FLASH 0xFFB24D
#define CM_LIGHT_MODE_STROBE 0xFF00FF
#define CM_LIGHT_MODE_FADE 0xFF58A7
#define CM_LIGHT_MODE_SMOOTH 0xFF30CF

#define CM_REPEAT 0xFFFFFFFF
/* /IR Button codes */

/* OneWire codes */
#define POWER_SIGNAL_MAIN_PIN 8
#define ONEWIRE_CODE_NOOP 0x00
#define ONEWIRE_CODE_OFF 0b11011001
#define ONEWIRE_CODE_FAN_OFF 0b11100000
#define ONEWIRE_CODE_FAN_MID 0b10100001
#define ONEWIRE_CODE_FAN_ON 0b01100111
/* /OneWire codes */

/* Data Exchange Params */
#define CMD_CMD_TURNINGON 0x01
#define CMD_CMD_TURNOFFBEGIN 0x03
#define CMD_CMD_TURNOFFREADY 0x04
#define CMD_CMD_TURNOFF 0x05
#define CMD_CMD_SETMODESTATE 0x10
#define CMD_CMD_SETDEVICESTATE 0x15
#define CMD_CMD_SETRTCTIME 0x20
#define CMD_CMD_PRESENCE 0x30
#define CMD_CMD_MAGNETIC_REQUEST 0x40
#define CMD_CMD_MAGNETIC_SETX 0x41
#define CMD_CMD_MAGNETIC_SETY 0x42
#define CMD_CMD_MAGNETIC_SETZ 0x43
#define CMD_CMD_CARDFOUND 0x50
#define CMD_CMD_LIGHTPDUCMD 0x60
/* /Data Exchange Params */

