#pragma once

// TODO: I prefer enum class for type-safety.
typedef enum {
    DISABLED = 0,
    ON_MOVE = 1,
    ON_COLLIDE = 2,
    USER_DEFINED = 3
} MotionDetectionMode;

extern MotionDetectionMode motionDetectMode; // Declared in at.c