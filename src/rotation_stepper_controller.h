#ifndef ROTATION_STEPPER_CONTROLLER
#define ROTATION_STEPPER_CONTROLLER

#include <Arduino.h>
#include <math.h>

#define DIRECTION_CLOCKWISE HIGH
#define DIRECTION_COUNTER_CLOCKWISE LOW

/// moves the servo to a set position from origin
#define SERVO_MOVE_ABSOLUTE "SA"
/// moves the servo to a position relative to it's current one
#define SERVO_MOVE_RELATIVE "SR"
/// returns the servo to the origin point
#define SERVO_MOVE_TO_ORIGIN "SH"
/// performs a test routine
#define SERVO_TEST_ROUTINE "ST"
/// forces the next move of the stepper to be clockwise
#define SERVO_FORCE_DIRECTION_CLOCKWISE "SFR"
/// forces the next move of the stepper to be counter-clockwise
#define SERVO_FORCE_DIRECTION_COUNTER_CLOCKWISE "SFL"

enum directions
{
    NONE = -1,
    COUNTER_CLOCKWISE = 0,
    CLOCKWISE = 1,
};

class Rotation_stepper_controller
{
private:
    directions forced_direction = directions::NONE;
    bool _running = false;
    int _enable_pin;
    int _direction_pin;
    int _pulse_pin;
    double _current_angle;
    double _target_angle;

    /// @brief given the current and target angles, returns the direction in which the stepper should be turning
    /// @param current_angle our angle in range [0, 360)
    /// @param target_angle angle to which we want to turn. Must be in range [0, 360)
    /// @return TRUE if direction is clockwise, FALSE if direction is counter clockwise. if current_angle == target_angle, returns TRUE
    bool _calculate_direction(double current_angle, double target_angle);
public:
    Rotation_stepper_controller(/* args */);
    ~Rotation_stepper_controller();

    // setters
    // const double& because it's TINY TINSY BIT FASTER (because we don't have to copy 
    // the double value and instead just use the one it's being pointed to)
    // look it probably doesn't matter but I think it's neat
    void current_angle(const double& current_angle) { _current_angle = current_angle; };

    void move_to_angle(double angle);
    void move_by_angle(double angle);
    void move_to_origin();
    void test_routine();
    bool take_serial_input(String input);
    void handle_movement();
};

#endif