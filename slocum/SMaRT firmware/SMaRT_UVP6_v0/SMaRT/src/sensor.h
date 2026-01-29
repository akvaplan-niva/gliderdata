#pragma once

// Sensor startup token & timeout
#define SENSOR_EXPECTED_STR   "HW_CONF"
#define SENSOR_TIMEOUT_MS     3000

// Start/Stop command formatting (no EOL inside BODY)
#define SENSOR_START_PREFIX   "$"
#define SENSOR_START_BODY     "start:ACQ_CSCS_002H,20250404,120000;"
#define SENSOR_STOP_CMD       "$stop;"

// Pick the EOL that your sensor expects (you said LF earlier, but many use CRLF)
//#define SENSOR_LINE_ENDING_CRLF  "\r\n"
#define SENSOR_LINE_ENDING_LF   "\n"
// #define SENSOR_LINE_ENDING_CR   "\r"

#define SENSOR_RESTART_DELAY_MS 1000

