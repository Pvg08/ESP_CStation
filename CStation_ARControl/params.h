
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


/* control pins */
#define CTRL_LEDOFF_PIN 34
#define CTRL_LOCKOPEN_PIN 36
#define CTRL_CONTROLSBLOCKED_PIN 38
#define CTRL_CAMERA_PIN 40
#define CTRL_UVLAMP_PIN 42
/* /control pins */

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



/* IR Button codes */
#define CM_ON 0xFFB04F
#define CM_OFF 0xFFF807
#define CM_REPEAT 0xFFFFFFFF
/* /IR Button codes */

/* OneWire codes */
#define POWER_SIGNAL_MAIN_PIN 8
#define ONEWIRE_CODE_NOOP 0x00
#define ONEWIRE_CODE_OFF 0b11011001
#define ONEWIRE_CODE_SILENT_MODE_OFF 0b11100000
#define ONEWIRE_CODE_SILENT_MODE_MID 0b10100001
#define ONEWIRE_CODE_SILENT_MODE_ON 0b01100111
/* /OneWire codes */

/* Data Exchange Params */
#define CMD_CMD_TURNINGON 0x01
#define CMD_CMD_TURNOFFBEGIN 0x03
#define CMD_CMD_TURNOFFREADY 0x04
#define CMD_CMD_TURNOFF 0x05
#define CMD_CMD_SETMODESTATE 0x10
#define CMD_CMD_SETRTCTIME 0x20
#define CMD_CMD_PRESENCE 0x30
#define CMD_CMD_MAGNETIC_REQUEST 0x40
#define CMD_CMD_MAGNETIC_SETX 0x41
#define CMD_CMD_MAGNETIC_SETY 0x42
#define CMD_CMD_MAGNETIC_SETZ 0x43
#define CMD_CMD_CARDFOUND 0x50

#define CMD_MODE_TRACKING 0x11
#define CMD_MODE_INDICATION 0x12
#define CMD_MODE_SILENCE 0x13
#define CMD_MODE_CONTROL 0x14
#define CMD_MODE_SECURITY 0x15
#define CMD_MODE_AUTOANIMATOR 0x16
/* /Data Exchange Params */

enum TrackingModeState {TRACKING_ON, TRACKING_SENSORONLY, TRACKING_OFF};
enum IndicationModeState {INDICATION_ON, INDICATION_LOW, INDICATION_SCREENOFF, INDICATION_LEDOFF, INDICATION_OFF};
enum SilenceModeState {SILENCE_NO, SILENCE_MEDIUM, SILENCE_MAX};
enum ControlModeState {CONTROL_ON, CONTROL_OFF};
enum SecurityModeState {SECURITY_LOCKED, SECURITY_UNLOCKED};
enum AutoAnimatorModeState {AA_OFF, AA_ON};

