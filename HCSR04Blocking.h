/**
 * @file                    HCSR04Blocking.h
 * @author                  Aditya Agarwal (aditya.agarwal@dumblebots.com)
 *
 * @brief                   Simple Library to use an HCSR04 ultrasonic sensor with MBed OS
 *
 * @copyright               Copyright (c) 2023
 *
 */


#ifndef __HCSR04BLOCKING_H__
#define __HCSR04BLOCKING_H__

#include "mbed.h"
#include "HCSR04.h"

/**
 * @brief                   Class that provides a simple interface to use an HCSR04 ultrasonic sensor
 */
class HCSR04Blocking {

    /** Ultrasonic Sensor instance for internally use */
    HCSR04          sensor;
    /** Semaphore to indicate whether a measurement has been completed or not */
    Semaphore       measurementLock;

    /** Distance measured by the sensor */
    /** Whether the sensor did not time out */
    float           measuredDist;
    bool            noTimeout;

public:

    /**
     * @brief               Construct a new HCSR04Blocking object
     *
     * @param   trig        Microcontroller Pin to which the Trig Pin of the sensor is connected
     * @param   echo        Microcontroller Pin to which the Echo Pin of the sensor is connected
     */
    HCSR04Blocking(PinName trigPin, PinName echoPin);

    /**
     * @brief               Initializes the internal HCSR04 object (See HCSR04::initialize())
     *
     * @remarks             This method must be called before the distance can be measured using the HCSR04Blocking::get_distance() method
     * @remark              If the thread was already initialized, then the HCSR04Blocking::finalize() method must be called
     *                      before trying to initialize it again
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the object could be initialized, false otherwise
     */
    bool            initialize();

    /**
     * @brief               Finalizes the internal HCSR04 object (see HCSR04::finalize())
     *
     * @remarks             After calling this method, the distance can not be measured using the HCSR04Blocking::get_distance() method
     * @remark              If the thread was not initialized or previously finalized, then the HCSR04Blocking::initialize() method must be called
     *                      before trying to finalize it again
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the object could be finalized, false otherwise
     */
    bool            finalize();

    /**
     * @brief               Checks if the object was initialized and callbacks can be dispatched correctly
     *
     * @attention           This function can be called from ISR context
     *
     * @return              true if the thread is running, false otherwise
     */
    bool            is_initialized() const;

    /**
     * @brief               Reads the distance from the sensor
     *
     * @remarks             Calling this method before calling the HCSR04Blocking::initialize() or after HSCR04Blocking::finalize() will produce undefined behaviour
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @param   distPtr     Location to store the measured distance
     *
     * @return              true if the sensor did not time-out, false otherwise
     */
    bool            get_distance(float *distPtr);

private:

    /**
     * @brief               Callback used by the internal sensor object to report a completed measurement
     *
     * @param valid         Whether a valid reading is available (sensor is working and did not time-out)
     * @param dist          Distance measured by the sensor
     */
    void            distance_cb(bool valid, float dist);

};

#endif //__HCSR04BLOCKING_H__
