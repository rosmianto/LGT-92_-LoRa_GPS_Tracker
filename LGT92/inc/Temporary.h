#pragma once

#include <stdint.h>
#include <stdbool.h>

// TODO: I prefer enum class for type-safety.
typedef enum {
    DISABLED = 0,
    ON_MOVE = 1,
    ON_COLLIDE = 2,
    USER_DEFINED = 3
} MotionDetectionMode;

typedef enum {
    GPS_L70RL      = 0,
    GPS_L76L       = 1,
    GPS_UBLOX_MAX7 = 2,
    GPS_UBLOX_MAX8 = 3,
    GPS_L76K       = 4
} GPSModel;

typedef enum {
    SEARCHMODE_DEFAULT = 0,
    SEARCHMODE_GPS_GLONASS = 1,
    SEARCHMODE_GPS_BEIDOU = 2,
    SEARCHMODE_GPS_GALILEO = 3,
    SEARCHMODE_GPS_GLONASS_GALILEO = 4
} GPSSearchMode;

typedef enum {
    NAVMODE_DEFAULT  = 0,
    NAVMODE_FITNESS  = 1,
    NAVMODE_AVIATION = 2,
    NAVMODE_BALLOON  = 3,
    NAVMODE_STATIONARY = 4
} GPSNavMode;

extern MotionDetectionMode motionDetectMode; // Declared in at.c
extern GPSModel gpsModel;                    // Declared in at.c
extern GPSSearchMode searchMode;             // Declared in at.c
extern GPSNavMode navMode;                   // Declared in at.c