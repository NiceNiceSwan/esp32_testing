#include "rotation_stepper_controller.h"

Rotation_stepper_controller::Rotation_stepper_controller()
{
}

Rotation_stepper_controller::~Rotation_stepper_controller()
{
}

void Rotation_stepper_controller::_calculate_direction(double current_angle, double target_angle)
{
    if (_forced_direction != directions::NONE)
    {
        digitalWrite(_direction_pin, _forced_direction);
        _forced_direction = directions::NONE;
        return;
    }
    
    double delta = fmod(target_angle - current_angle + 360, 360) - 180;
    bool direction = delta <= 0;
    digitalWrite(_direction_pin, direction);
}

command Rotation_stepper_controller::_input_to_command(const std::string& input)
{
    static const std::unordered_map<std::string, command> map = {
        {SERVO_MOVE_ABSOLUTE, command::MOVE_ABSOLUTE},
        {SERVO_MOVE_RELATIVE, command::MOVE_RELATIVE},
        {SERVO_MOVE_TO_ORIGIN, command::MOVE_TO_ORIGIN},
        {SERVO_SET_HOME, command::SET_HOME},
        {SERVO_TEST_ROUTINE, command::TEST_ROUTINE},
        {SERVO_FORCE_DIRECTION_CLOCKWISE, command::FORCE_DIRECTION_CLOCKWISE},
        {SERVO_FORCE_DIRECTION_COUNTER_CLOCKWISE, command::FORCE_DIRECTION_COUNTER_CLOCKWISE},
        {SERVO_FORCE_DIRECTION_NONE, command::FORCE_DIRECTION_NONE},
    };

    auto it = map.find(input);
    return it != map.end() ? it->second : command::ERROR;
}

void Rotation_stepper_controller::move_to_angle(double angle)
{
    angle = fmod(angle, 360);
    _target_angle = angle;
    _calculate_direction(_current_angle, _target_angle);
}

void Rotation_stepper_controller::move_by_angle(double angle)
{
    angle = fmod(angle, 360);
    _target_angle = angle + _current_angle;
    _calculate_direction(_current_angle, _target_angle);
}

void Rotation_stepper_controller::move_to_origin()
{
    _target_angle = 0;
    _calculate_direction(_current_angle, _target_angle);
}

void Rotation_stepper_controller::set_home()
{
    _current_angle = 0;
    _target_angle = 0;
    _forced_direction = directions::NONE;
    _running = false;
}

bool Rotation_stepper_controller::take_serial_input(String input)
{
    if (input == SERVO_FORCE_DIRECTION_CLOCKWISE)
    {
        _forced_direction = directions::CLOCKWISE;
        return true;
    }
    if (input == SERVO_FORCE_DIRECTION_COUNTER_CLOCKWISE)
    {
        _forced_direction = directions::COUNTER_CLOCKWISE;
        return true;
    }
    if (input == SERVO_FORCE_DIRECTION_NONE)
    {
        _forced_direction = directions::NONE;
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
    double distance_to_target = abs(_current_angle - _target_angle);
    if (distance_to_target < MAX_ANGLE_RESOLUTION)
    {
        _forced_direction = directions::NONE;
        _running = false;
        return;
    }
    
    _running = true;
	digitalWrite(_pulse_pin, HIGH);
	delayMicroseconds(500);
	digitalWrite(_pulse_pin, LOW);
}
