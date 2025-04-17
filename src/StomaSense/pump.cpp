#include <StomaSense/pump.h>

#include <StomaSense/pin_defs.h>
#include <StomaSense/datatypes.h>

#include <cstring>
#include <limits>

using namespace StomaSense;

static PicoStepper stepper;
static PicoPWM pump;
static PicoServo servo = {};

static Scale scales[STOMA_SENSE_N_SCALES];
static size_t n_scales = 0;

static Array<scale_t, uint8_t, STOMA_SENSE_WATERING_QUEUE_SIZE> watering_queue;

bool Pump::init()
{
    mutex_stepper_claim_blocking();
    pico_stepper_init(
        &stepper,
        STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4,
        PicoStepperStepType::PICO_STEPPER_STEP_TYPE_HALF, false);
    mutex_stepper_release();

    pico_pwm_init(&pump, PUMP_PIN, false);
    pico_pwm_set_freq_and_duty_u16(&pump, 100'000, 0);

    mutex_servo_claim_blocking();
    bool res = pico_servo_init(&servo, SERVO_PIN, true);
    mutex_servo_release();
    return res;
}

bool add_scale(const Scale *scale)
{
    if (n_scales >= STOMA_SENSE_N_SCALES)
    {
        STOMASENSE_WARN("can't add another scale: full\n");
        return false;
    }
    memcpy(&scales[n_scales], scale, sizeof(Scale));
    ++n_scales;
    return true;
}

bool overwrite_scale(const Scale *scale, scale_t scale_n)
{
    if (scale_n >= n_scales)
    {
        STOMASENSE_WARN("can't overwrite scale in array position %u (the array is %lu in size)\n", scale_n, n_scales);
        return false;
    }
    memcpy(&scales[scale_n], scale, sizeof(Scale));
    return true;
}

bool add_scale_to_watering_queue(scale_t scale_n)
{
    mutex_watering_queue_access_claim_blocking();
    if (watering_queue.full())
    {
        mutex_watering_queue_access_release();
        STOMASENSE_DEBUG("can't add scale to watering target: it's already full\n");
        return false;
    }

    watering_queue.append(scale_n);
    mutex_watering_queue_access_release();
    return true;
}

bool pop_next_target(scale_t *scale)
{
    mutex_watering_queue_access_claim_blocking();
    if (watering_queue.empty())
    {
        mutex_watering_queue_access_release();
        STOMASENSE_DEBUG("can't pop next target because there are no targets in queue\n");
        return false;
    }
    watering_queue.sort_distance([](scale_t a, scale_t b)
                                 {
        if (a > b) return b - a;
        return a - b; });
    bool res = watering_queue.pop_front(scale);
    mutex_watering_queue_access_release();
    if (!res)
    {
        STOMASENSE_ERROR("internal watering_queue Array error\n");
        return false;
    }
    return true;
}

static inline void set_pump_to_duty(uint16_t duty)
{
    pico_pwm_set_duty_u16(&pump, duty);
    pico_pwm_hw_enable(&pump, duty > 0);
}

const Scale *water_next_target()
{
    // get next scale_n
    scale_t scale_n;
    bool available_targets = pop_next_target(&scale_n);
    if (!available_targets)
    {
        return nullptr;
    }

    // filter sccale with right scale_n
    Scale *scale = nullptr;
    for (uint8_t i = 0; i < n_scales; ++i)
    {
        if (scales[i].scale_n == scale_n)
        {
            scale = &scales[i];
            break;
        }
    }
    if (!scale)
    {
        STOMASENSE_ERROR("scale with scale_n == %u not found!\n", scale_n);
        return nullptr;
    }

    // lock servo to 90
    mutex_servo_claim_blocking();
    if (!pico_servo_attach(&servo))
    {
        STOMASENSE_ERROR("couldn't attach servo");
        mutex_servo_release();
        return nullptr;
    }
    pico_servo_set_angle(&servo, 90);
    mutex_servo_release();

    // move stepper
    mutex_stepper_claim_blocking();
    STOMASENSE_DEBUG("moving stepper to pos %li\n", scale->pos.stepper);
    pico_stepper_move_to_pos(&stepper, scale->pos.stepper);
    mutex_stepper_release();

    // move and lock servo
    mutex_servo_claim_blocking();
    if (!pico_servo_sweep(&servo, scale->pos.servo, 10, 10))
    {
        STOMASENSE_ERROR("couldn't attach servo");
        mutex_servo_release();
        return nullptr;
    }
    mutex_stepper_release();

    // pump
    WateringIntensity wi;
    scale->pos.watering_data.interpolate(&wi, 0.2);
    pump_intensity_t intensity = static_cast<pump_intensity_t>(
        std::numeric_limits<uint16_t>::max() * intensity / 100
    );
    set_pump_to_duty(intensity);
    delay_ms(wi.time_ms);
    set_pump_to_duty(0);

    // release servo
    mutex_servo_claim_blocking();
    if (!pico_servo_sweep(&servo, 90, 10, 10))
    {
        STOMASENSE_ERROR("couldn't attach servo");
        mutex_servo_release();
        return nullptr;
    }
    pico_servo_release(&servo);
    mutex_stepper_release();

    return scale;
}