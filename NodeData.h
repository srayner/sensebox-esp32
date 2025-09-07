#ifndef NODEDATA_H
#define NODEDATA_H

#include <time.h>
#include <Arduino.h>

#define MAX_READINGS 20   // max readings per sensor

// --- Data structures ---

struct SensorData {
    time_t timestamp;
    float value;
    String unit;
};

struct Sensor {
    String name;
    String type;
    String model;
    SensorData readings[MAX_READINGS];
    uint8_t readingCount = 0;  // how many valid readings
    uint8_t writeIndex = 0;    // index where the next reading will be written
};

struct Node {
    String name;
    String location;
    Sensor* sensors[2];
};

void addReading(Sensor &sensor, time_t timestamp, float value, String unit) {
    // Store the reading at the current writeIndex
    sensor.readings[sensor.writeIndex] = { timestamp, value, unit };

    // Move writeIndex forward, wrap around if needed
    sensor.writeIndex = (sensor.writeIndex + 1) % MAX_READINGS;

    // Update readingCount if not full yet
    if (sensor.readingCount < MAX_READINGS) {
        sensor.readingCount++;
    }
}

uint8_t getBufferPercentFull(const Sensor &sensor) {
    return (sensor.readingCount * 100) / MAX_READINGS;
}

#endif
