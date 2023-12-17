#include "HCSR04Blocking.h"

// Constructors

HCSR04Blocking::HCSR04Blocking(PinName trigPin, PinName echoPin)
        : sensor(trigPin, echoPin)
        , measurementLock(0, 1)
{
}

// Public Methods

bool
HCSR04Blocking::initialize() {
    return sensor.initialize();
}

bool
HCSR04Blocking::finalize() {
    return sensor.finalize();
}

bool
HCSR04Blocking::is_initialized() const {
    return sensor.is_initialized();
}

bool
HCSR04Blocking::get_distance(float *distPtr) {

    // start the measurement and sleep till the HCSR04 releases the Semaphore in the callback

    sensor.do_measurement(callback(this, &HCSR04Blocking::distance_cb));
    measurementLock.acquire();

    if (!noTimeout) {
        return false;
    }

    *distPtr = measuredDist;
    return true;
}

// Private Methods

void
HCSR04Blocking::distance_cb(bool valid, float dist) {

    // store whether the sensor returned a valid distance (sensor working, no timeout)
    // store the distance (if valid)
    // release the measurementLock Semaphore to indicate that the measurement is complete and results available

    noTimeout = valid;
    if (valid) {
        measuredDist = dist;
    }

    measurementLock.release();
}
