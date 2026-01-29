#pragma once

// Startup token from Slocum (Backseat Driver)
#define GLIDER_EXPECTED_STR   "$HI"
#define GLIDER_TIMEOUT_MS     3000

// $SD messages
#define GLIDER_SD_TAG         "SD"
#define GLIDER_SD_MODE_FIELD  5  // SD,3:<mode>

enum glider_mode {
    GLIDER_MODE_UNKNOWN = -1,
    GLIDER_MODE_DIVING  = 1,
    GLIDER_MODE_CLIMBING= 2,
    GLIDER_MODE_HOVER   = 3
};

