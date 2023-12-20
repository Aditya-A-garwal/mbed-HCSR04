/**
 * @file                    HCSR04.h
 * @author                  Aditya Agarwal (aditya.agarwal@dumblebots.com)
 *
 * @brief                   Simple Library to use an HCSR04 ultrasonic sensor with MBed OS asynchronously
 *
 * @copyright               Copyright (c) 2023
 *
 */

#ifndef __HCSR04_H__
#define __HCSR04_H__

#include <utility>

#include "mbed.h"

/**
 * @brief                   Class that provides a simple interface to use an HCSR04 ultrasonic sensor asynchronously
 *
 */
class HCSR04 {

    /** Trigger Pin of the sensor */
    DigitalOut      trigPin;
    /** Echo Pin of the sensor */
    InterruptIn     echoPin;

    /** Microsecond timer to measure the duration of a pulse */
    Timer           pulseTimer;
    /** Distance calculated from the duration of the pulse */
    float           dist {0};

    /** Handle to thread used for periodically reading from the sensor */
    Thread          *threadHandle {nullptr};

    /** Queue to post measurement event on */
    EventQueue      queue;
    /** ID of the periodic event on the EventQueue (0 if no periodic event or failed allocation) */
    int32_t         periodicId {0};
    /** Number of non-periodic measurements pending in the queue */
    uint32_t        pendingMeasurementCount {0};

    /** Semaphore to indicate whether a complete pulse has been received or not */
    Semaphore       pulseBusyLock;
    /** Semaphore to block the queue dispatch thread and for graceful termination */
    Semaphore       shouldTerminate;

public:

    HCSR04() = delete;

    /**
     * @brief               Construct a new HCSR04 object
     *
     * @param   trig        Microcontroller Pin to which the Trig Pin of the sensor is connected
     * @param   echo        Microcontroller Pin to which the Echo Pin of the sensor is connected
     */
    HCSR04(PinName trig, PinName echo);

    /**
     * @brief               Initializes the object by allocating and starting a thread to dispatch callbacks on
     *
     * @remark              If the thread was already initialized, then the HCSR04::finalize() method must be called
     *                      before trying to initialize it again
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the thread was successfully allocated and started, false otherwise
     */
    bool        initialize();

    /**
     * @brief               Finalizes the object by stopping and freeing the thread on which callbacks are dispatched
     *
     * @remark              If the thread was not initialized or finalized before, then the HCSR04::initialize() method must be called
     *                      before trying to finalize it again
     * @remark              The object can not be finalized as long as there are pending non-periodic measurements
     * @remark              The object can not be finalized as long as there is a registered non-periodic measurement
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the thread was successfully stopped and freed, false otherwise
     */
    bool        finalize();

    /**
     * @brief               Checks if the object was initialized and callbacks can be dispatched correctly
     *
     * @attention           This function can be called from ISR context
     *
     * @return              true if the thread is running, false otherwise
     */
    bool        is_initialized() const;

    /**
     * @brief               Asynchronously starts a measurement from the sensor and returns immediately, calling the callback once the measurement is complete
     *
     * @remarks             As long as periodic measurement is started, this function will always return false
     * @remarks             As long as a measurement (enqueued using this method) is still pending, periodic measurement can not be started
     *
     * @attention           This function can be called from ISR context
     *
     * @param cb            Callback when the distance is calculated
     *                      , the first argument to the callback is a boolean value which is false if the sensor timed-out, true otherwise
     *                      , the second argument to the callback is the distance as floating point value
     *
     * @return              true if the request to start a measurement could successfully be enqueued, false otherwise
     */
    bool        do_measurement(const Callback<void(bool, float)> &cb);

    /**
     *
     * @return
     */
    uint32_t    get_pending_measurement_count() const;

    /**
     * @brief               Begins asynchronously, periodically measuring the distance forever
     *
     * @remarks             To stop periodic measurement, see the HCSR04::stop_measurement_periodic() function
     *
     * @attention           This function can be called from ISR context
     *
     * @param period        Time period between two measurements
     * @param cb            Callback when the distance is calculated
     *                      , the first argument to the callback is a boolean value which is false if the sensor timed-out, true otherwise
     *                      , the second argument to the callback is the distance as floating point value
     *
     * @return              true if the request to start a measurement could successfully be enqueued, false otherwise
     */
    bool        start_measurement_periodic(std::chrono::milliseconds period, const Callback<void(bool, float)> &cb);

    /**
     * @brief               Stops periodically measuring the distance
     *
     * @attention           Can not call from ISR context
     *
     * @remarks             If a measurement was happening while this function was called, the measurement (along with the callback) are completed first
     *                      , rather than instantly stopping the measurements
     *
     * @attention           This function can be called from ISR context
     */
    void        stop_measurement_periodic();

    /**
     * @brief           Checks if periodic measurement was started
     *
     * @attention       This function can be called from ISR context
     *
     * @return          true if periodic measurement is started, false otherwise
     */
    bool        is_periodic_started() const;

private:

    /**
     * @brief           Handler for the start of the returned pulse from the sensor
     *
     * @remarks         This is called whenever a rise interrupt is received on the Echo pin (start of a pulse)
     */
    void        pulse_start_handler();

    /**
     * @brief           Handler for the end of the returned pulse from the sensor
     *
     * @remarks         This is called whenever a fall interrupt is received on the Echo pin (end of a pulse)
     */
    void        pulse_end_handler();

    /**
     * @brief           Helper function to send a pulse to the sensor's Trig pin
     */
    __attribute__((always_inline))
    void        start_pulse();

    /**
     * @brief           Helper function to atomically increment the count of pending measurements
     *
     * @todo            Make this function atomic
     */
    __attribute__((always_inline))
    void        inc_pending_measurements();

    /**
     * @brief           Helper function to atomically decrement the count of pending measurements
     *
     * @todo            Make this function atomic
     */
    __attribute__((always_inline))
    void        dec_pending_measurements();

    /**
     * @brief           Function for queue to run callbacks on
     *
     * @remarks         Calling break_dispatch() on the queue without acquiring shouldTerminate will cancel
     *                  the registered periodic event (if at all) and dispatch the queue again
     *
     * @remarks         Calling break_dispatch() on the queue after acquiring shouldTerminate will cancel
     *                  the registered periodic event (if at all) and prepare the thread for graceful termination
     */
    void        dispatch_events();
};

#endif //__HCSR04_H__
