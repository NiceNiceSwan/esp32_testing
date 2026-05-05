#include "rotation_stepper_controller.h"

Rotation_stepper_controller::Rotation_stepper_controller()
{
}

Rotation_stepper_controller::~Rotation_stepper_controller()
{
}

bool Rotation_stepper_controller::_calculate_direction(double current_angle, double target_angle)
{
    if (forced_direction != directions::NONE)
    {
        return forced_direction;
    }
    
    double delta = fmod(target_angle - current_angle + 360, 360) - 180;
    return delta <= 0;
    // if (delta <= 0)
    // {
    //     return directions::CLOCKWISE;
    // }
    // return directions::COUNTER_CLOCKWISE;
    // directions direction = delta <= 0;
}

void Rotation_stepper_controller::move_to_angle(double angle)
{
    bool direction = _calculate_direction(_current_angle, _target_angle);
	digitalWrite(_direction_pin, direction);
    angle = fmod(angle, 360);
    _target_angle = angle;
}

void Rotation_stepper_controller::move_by_angle(double angle)
{
    bool direction = _calculate_direction(_current_angle, _target_angle);
	digitalWrite(_direction_pin, direction);
    angle = fmod(angle, 360);
    _target_angle = angle + _current_angle;
}

void Rotation_stepper_controller::move_to_origin()
{
    bool direction = _calculate_direction(_current_angle, _target_angle);
	digitalWrite(_direction_pin, direction);
    _target_angle = 0;
}

bool Rotation_stepper_controller::take_serial_input(String input)
{
    if (input == SERVO_FORCE_DIRECTION_CLOCKWISE)
    {
        forced_direction = directions::CLOCKWISE;
        return true;
    }
    if (input == SERVO_FORCE_DIRECTION_COUNTER_CLOCKWISE)
    {
        forced_direction = directions::COUNTER_CLOCKWISE;
        return true;
    }
    if (input.startsWith(SERVO_MOVE_ABSOLUTE))
    {
        double target_angle = input.substring(2).toDouble();
        move_to_angle(target_angle);
        return true;
    }
    if (input.startsWith(SERVO_MOVE_RELATIVE))
    {
        double move_angle = input.substring(2).toDouble();
        move_by_angle(move_angle);
        return true;
    }
    if (input.startsWith(SERVO_MOVE_TO_ORIGIN))
    {
        move_to_origin();
        return true;
    }
    if (input.startsWith(SERVO_TEST_ROUTINE))
    {
        test_routine();
        return true;
    }
    return false;

}

void Rotation_stepper_controller::handle_movement()
{
    if (_current_angle == _target_angle)
    {
        _running = false;
        return;
    }
    
    _running = true;
	digitalWrite(_pulse_pin, HIGH);
	delayMicroseconds(500);
	digitalWrite(_pulse_pin, LOW);
}
