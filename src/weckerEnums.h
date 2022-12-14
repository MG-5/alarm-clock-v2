#ifndef WECKERENUMS_H_
#define WECKERENUMS_H_

enum class States
{
    STANDBY,
    CLOCK,
    DISPLAY_ALARM1,
    DISPLAY_ALARM2,
    CHANGE_ALARM1_HOUR,
    CHANGE_ALARM1_MINUTE,
    CHANGE_ALARM2_HOUR,
    CHANGE_ALARM2_MINUTE,
    TEMP,
    DISPLAY_ALARM_STATUS,
    TEST,
    LED_PERCENT,
    DEBUGMODE
};

enum class AlarmStates
{
    OFF,
    ALARM1_LED,
    ALARM2_LED,
    ALARM1_VIBR,
    ALARM2_VIBR
};

enum class AlarmModes
{
    INACTIVE,
    ALARM1_ACTIVE,
    ALARM2_ACTIVE,
    ALARM1_ALARM2_ACTIVE
};

enum class LED_States
{
    NORMAL,
    AUTO,
    FLASH,
    JUMP3,
    JUMP7,
    FADE3,
    FADE7
};

enum class Color_States
{
    RED,
    GREEN,
    BLUE,
    WHITE,
    YELLOW,
    TUERKIS,
    PURPLE
};

enum class Switching_States
{
    NO,
    TURNING_ON,
    TURNING_OFF,
    FADING
};

#endif /* WECKERENUMS_H_ */
