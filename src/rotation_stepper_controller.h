#ifndef ROTATION_STEPPER_CONTROLLER
#define ROTATION_STEPPER_CONTROLLER

#include <Arduino.h>
#include <math.h>
#include <unordered_map>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DIRECTION_CLOCKWISE HIGH
#define DIRECTION_COUNTER_CLOCKWISE LOW

#define MAX_ANGLE_RESOLUTION 0.35

// control commands

/// moves the servo to a set position from origin. 
/// Should be followed by a number immediately after, eg. SA300 or SA-100
#define SERVO_MOVE_ABSOLUTE "SA"
/// moves the servo to a position relative to it's current one. 
/// Should be followed by a number immediately after, eg. SR300 or SR-100
#define SERVO_MOVE_RELATIVE "SR"
/// returns the servo to the origin point.
#define SERVO_MOVE_TO_ORIGIN "SH"
/// performs a test routine
#define SERVO_TEST_ROUTINE "ST"
/// sets the current point to be the 0 point
#define SERVO_SET_HOME "SSH"
/// forces the next move of the stepper to be clockwise
#define SERVO_FORCE_DIRECTION_CLOCKWISE "SFR"
/// forces the next move of the stepper to be counter-clockwise
#define SERVO_FORCE_DIRECTION_COUNTER_CLOCKWISE "SFL"
/// resets the forced direction
#define SERVO_FORCE_DIRECTION_NONE "SFN"

enum Command
{
    MOVE_ABSOLUTE,
    MOVE_RELATIVE,
    MOVE_TO_ORIGIN,
    TEST_ROUTINE,
    SET_HOME,
    FORCE_DIRECTION_CLOCKWISE,
    FORCE_DIRECTION_COUNTER_CLOCKWISE,
    FORCE_DIRECTION_NONE,
    ERROR,
};

enum directions
{
    NONE = -1,
    COUNTER_CLOCKWISE = 0,
    CLOCKWISE = 1,
};

class Rotation_stepper_controller
{
private:
    uint8_t _enable_pin = -1;
    uint8_t _direction_pin = -1;
    uint8_t _pulse_pin = -1;

    directions _forced_direction = directions::NONE;
    bool _running = false;
    double _current_angle = 0;
    double _target_angle = 0;

    /// @brief given the current and target angles, returns the direction in which the stepper should be turning
    /// @param current_angle our angle in range [0, 360)
    /// @param target_angle angle to which we want to turn. Must be in range [0, 360)
    void _calculate_direction(double current_angle, double target_angle);
    Command _input_to_command(const std::string& input);
    /// @brief clamps the value
    /// @param value value which you want clamped
    /// @param min minimum value this can be
    /// @param max maximum value this can be
    /// @return value clamped to range min and max, including min and max
    int _clamp(int value, const int &min, const int &max);
public:
    Rotation_stepper_controller();
    Rotation_stepper_controller(uint8_t enable_pin, uint8_t direction_pin, uint8_t pulse_pin);
    ~Rotation_stepper_controller();

    void attach_pins(uint8_t enable_pin, uint8_t direction_pin, uint8_t pulse_pin);

    // setters

    // const double& because it's TINY TINSY BIT FASTER (because we don't have to copy 
    // the double value and instead just use the one it's being pointed to)
    // look it probably doesn't matter but I think it's neat
    void current_angle(const double& current_angle) { _current_angle = current_angle; };
    void attach_pulse_pin(uint8_t pulse_pin) { _pulse_pin = pulse_pin; };
    void attach_direction_pin(uint8_t direction_pin) {_direction_pin = direction_pin; };
    void attach_enable_pin(uint8_t enable_pin) { _enable_pin = enable_pin; };

    // getters

    // technically unnecessary since in our use case it can be read directly from main. But who cares.
    double current_angle() { return _current_angle; };
    double target_angle() { return _target_angle; };
    directions forced_direction() { return _forced_direction; };
    bool running() { return _running; };

    /// @brief absolute angle to which we want to move the motor from the 0 position
    /// @param angle angle in degrees. Can be any real number
    void move_to_angle(double angle);
    /// @brief relative move by the amount specified from the current position
    /// @param angle angle in degrees. Can be any real number
    void move_by_angle(double angle);
    /// @brief returns the servo to the 0 point
    void move_to_origin();
    /// @brief sets the current position as the 0 point. Also stops the servo and resets the forced direction
    void set_home();
    void test_routine();
    bool take_serial_input(String input);
    void handle_movement();
};

#endif