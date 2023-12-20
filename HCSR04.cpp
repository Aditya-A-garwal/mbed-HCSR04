#include "HCSR04.h"

/** Maximum Distance the sensor should be able to measure before readings are considered invalid/too far awat */
constexpr auto MAX_DISTANCE = 300;
/** Timeout of the sensor based on the maximum distance it can measure */
constexpr auto SENSOR_TIMEOUT = MAX_DISTANCE * 20'000ms / 343;

// Constructors

HCSR04::HCSR04(PinName trig, PinName echo)
        : trigPin(trig)
        , echoPin(echo)
        , pulseBusyLock(0, 1)
        , shouldTerminate(1, 1)
{

    echoPin.rise(callback(this, &HCSR04::pulse_start_handler));
    echoPin.fall(callback(this, &HCSR04::pulse_end_handler));
}

// Public Methods

bool
HCSR04::initialize() {

    // return if the thread was already initialized; move forward otherwise
    // allocate the thread and return if the allocation fails; move forward otherwise
    // try to start the thread and return if successful
    // in case of failure, delete the allocated thread and return

    if (is_initialized()) {
        return false;
    }

    threadHandle = new (std::nothrow) Thread(osPriorityRealtime);
    if (threadHandle == nullptr) {
        return false;
    }

    auto status = threadHandle->start(callback(this, &HCSR04::dispatch_events));
    if (status != osOK) {

        delete threadHandle;
        threadHandle = nullptr;

        return false;
    }

    return true;
}

bool
HCSR04::finalize() {

    // return if the thread was already initialized, there are pending non-periodic measurements or a periodic measurement; move forward otherwise
    // allocate the thread and return if the allocation fails; move forward otherwise
    // try to start the thread and return if successful
    // in case of failure, delete the allocated thread and return

    if (!is_initialized() || is_periodic_started() || get_pending_measurement_count() > 0) {
        return false;
    }

    shouldTerminate.acquire();
    queue.break_dispatch();

    threadHandle->join();
    // auto status = threadHandle->terminate();
    // if (status != osOK) {
    //     return false;
    // }
    shouldTerminate.release();

    delete threadHandle;
    threadHandle = nullptr;
    return true;
}

bool
HCSR04::is_initialized() const {

    // if threadHandle is NULL, then the object was not initialized/finalized before
    // otherwise it is still in the initialized state

    return threadHandle != nullptr;
}

bool
HCSR04::do_measurement(const Callback<void(bool, float)> &cb) {

    // return if a periodic event is already registered; move forward otherwise
    // otherwise, post a non-periodic event to the queue, where the distance is measured and the callback called
    // increment the pending measurement count if the event was successfully posted

    if (is_periodic_started()) {
        return false;
    }

    auto id = queue.call([this, cb]() {

        // start a pulse and sleep on the lock while the pulse does not return
        // the lock is released in HCSR04::pulse_end_handler() when the pulse is completely received
        // if the pulse takes too long (faulty sensor or object too far away), then wake-up anyway

        start_pulse();

        pulseBusyLock.try_acquire_for(SENSOR_TIMEOUT)
        ? cb(true, dist)
        : cb(false, 0.0f);

        dec_pending_measurements();
    });

    if (id == 0) {
        return false;
    }

    inc_pending_measurements();
    return true;
}

uint32_t
HCSR04::get_pending_measurement_count() const {

    return pendingMeasurementCount;
}

bool
HCSR04::start_measurement_periodic(std::chrono::milliseconds period, const Callback<void(bool, float)> &cb) {

    // return if a periodic measurement is already started, or if there are pending non-periodic measurements; move forward otherwise
    // otherwise, post a periodic event to the queue, where the distance is measured and the callback called

    if (is_periodic_started() || get_pending_measurement_count() > 0) {
        return false;
    }

    auto id = queue.call_every(period, [this, cb] {

        // start a pulse and sleep on the lock while the pulse does not return
        // the lock is released in HCSR04::pulse_end_handler() when the pulse is completely received
        // if the pulse takes too long (faulty sensor or object too far away), then wake-up anyways

        start_pulse();

        pulseBusyLock.try_acquire_for(SENSOR_TIMEOUT)
        ? cb(true, dist)
        : cb(false, 0.0f);
    });

    periodicId = id;
    return periodicId != 0;
}

void
HCSR04::stop_measurement_periodic() {

    // return if no periodic measurement was registered; move forward otherwise
    // if the object was not initialized, directly cancel the event and move forward
    // otherwise, break the dispatch without acquiring shouldTerminate to cancel the event in the other thread

    // because periodic and non-periodic measurements are exclusive, this method can never break pending non-periodic measurements

    if (!is_periodic_started()) {
        return;
    }

    if (!is_initialized()) {

        queue.cancel(periodicId);
        periodicId = 0;

        return;
    }

    queue.break_dispatch();
}

bool
HCSR04::is_periodic_started() const {

    // if a periodic event was started, then it will have non-zero ID in the queue

    return (periodicId != 0);
}

// Private methods

void
HCSR04::pulse_start_handler() {

    // start the high-resolution timer

    pulseTimer.start();
}

void
HCSR04::pulse_end_handler() {

    // stop the high-resolution timer, get its measured value and calculate the distance using the formula
    // finally, reset the timer for the next use and release the pulseBusyLock to indicate that the pulse has been entirely received and processed

    uint32_t pulse;

    pulseTimer.stop();
    pulse   = chrono::duration_cast<chrono::microseconds>(pulseTimer.elapsed_time()).count();
    dist    = ((float)pulse * 343) / (10'000 * 2);

    pulseTimer.reset();
    pulseBusyLock.release();
}

__attribute__((always_inline))
void
HCSR04::start_pulse() {

    // send a short 10ms pulse on the sensor

    ThisThread::sleep_for(2ms);
    trigPin = 1;
    ThisThread::sleep_for(10ms);
    trigPin = 0;
}

__attribute__((always_inline))
void
HCSR04::inc_pending_measurements() {
    ++pendingMeasurementCount;
}

__attribute__((always_inline))
void
HCSR04::dec_pending_measurements() {
    --pendingMeasurementCount;
}


void
HCSR04::dispatch_events() {

    // keep dispatching the queue in an infinite loop
    //
    // if break_dispatch is called without acquiring shouldTerminate,
    // the periodic event is cancelled (if registered) and the queue is dispatched again
    //
    // if break_dispatch is called after acquiring shouldTerminate,
    // the periodic event is cancelled (if registered) and the thread is prepared for graceful termination

    for (;;) {

        queue.dispatch_forever();

        if (periodicId != 0) {

            queue.cancel(periodicId);
            periodicId = 0;
        }

        if (!shouldTerminate.try_acquire()) {
            break;
        }
        shouldTerminate.release();
    }
}