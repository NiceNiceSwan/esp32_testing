#include "rotation_stepper_controller.h"

Rotation_stepper_controller::Rotation_stepper_controller()
{
}

Rotation_stepper_controller::Rotation_stepper_controller(uint8_t enable_pin, uint8_t direction_pin, uint8_t pulse_pin)
{
    _enable_pin = enable_pin;
    _direction_pin = direction_pin;
    _pulse_pin = pulse_pin;

    
	pinMode(_enable_pin, OUTPUT);
	pinMode(_direction_pin, OUTPUT);
	pinMode(_pulse_pin, OUTPUT);
    digitalWrite(_enable_pin, HIGH);
}

Rotation_stepper_controller::~Rotation_stepper_controller()
{
}

void Rotation_stepper_controller::attach_pins(uint8_t enable_pin, uint8_t direction_pin, uint8_t pulse_pin)
{
    _enable_pin = enable_pin;
    _direction_pin = direction_pin;
    _pulse_pin = pulse_pin;
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
    digitalWrite(_direction_pin, HIGH);
}

Command Rotation_stepper_controller::_input_to_command(const std::string& input)
{
    static const std::unordered_map<std::string, Command> map = {
        {SERVO_MOVE_ABSOLUTE, Command::MOVE_ABSOLUTE},
        {SERVO_MOVE_RELATIVE, Command::MOVE_RELATIVE},
        {SERVO_MOVE_TO_ORIGIN, Command::MOVE_TO_ORIGIN},
        {SERVO_SET_HOME, Command::SET_HOME},
        {SERVO_TEST_ROUTINE, Command::TEST_ROUTINE},
        {SERVO_FORCE_DIRECTION_CLOCKWISE, Command::FORCE_DIRECTION_CLOCKWISE},
        {SERVO_FORCE_DIRECTION_COUNTER_CLOCKWISE, Command::FORCE_DIRECTION_COUNTER_CLOCKWISE},
        {SERVO_FORCE_DIRECTION_NONE, Command::FORCE_DIRECTION_NONE},
    };

    auto iterator = map.find(input);
    return iterator != map.end() ? iterator->second : Command::ERROR;
}

int Rotation_stepper_controller::_clamp(int value, const int &min_, const int &max_)
{
    value = min(value, max_);
    value = max(value, min_);
    return value;
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

void Rotation_stepper_controller::test_routine()
{

}

bool Rotation_stepper_controller::take_serial_input(String input)
{
    double target_angle = 0;
    Command command;

    if (isDigit(input.charAt(2)))
    {
        command = _input_to_command(input.substring(0, 2).c_str());
    }
    else
    {
        command = _input_to_command(input.c_str());
    }
    
    switch (command)
    {
    case Command::FORCE_DIRECTION_CLOCKWISE:
        Serial.println("Force direction clockwise");
        _forced_direction = directions::CLOCKWISE;
        return true;
    case Command::FORCE_DIRECTION_COUNTER_CLOCKWISE:
        Serial.println("Force direction counter clockwise");
        _forced_direction = directions::COUNTER_CLOCKWISE;
        return true;
    case Command::FORCE_DIRECTION_NONE:
        Serial.println("Force direction none");
        _forced_direction = directions::NONE;
        return true;
    case Command::MOVE_ABSOLUTE:
        Serial.println("Move absolute");
        target_angle = input.substring(2).toDouble();
        move_to_angle(target_angle);
        return true;
    case Command::MOVE_RELATIVE:
        Serial.println("Move relative");
        target_angle = input.substring(2).toDouble();
        move_by_angle(target_angle);
        return true;
    case Command::MOVE_TO_ORIGIN:
        Serial.println("Move to origin");
        move_to_origin();
        return true;
    case Command::TEST_ROUTINE:
        Serial.println("Test routine");
        test_routine();
        return true;
    case Command::SET_HOME:
        Serial.println("Set home");
        set_home();
        return true;
    default:
        Serial.println("Wrong command");
        return false;
    }

    // just in case
    return false;
}

void Rotation_stepper_controller::handle_movement()
{
    // Serial.println(_current_angle);
    // Serial.println(_target_angle);
    double distance_to_target = abs(_current_angle - _target_angle);
    Serial.println(distance_to_target);
    if (distance_to_target < MAX_ANGLE_RESOLUTION)
    {
        _forced_direction = directions::NONE;
        _running = false;
        return;
    }
    
    _running = true;
	digitalWrite(_pulse_pin, HIGH);
    vTaskDelay(50/portTICK_PERIOD_MS);
	// delayMicroseconds(500);
	digitalWrite(_pulse_pin, LOW);
    vTaskDelay(50/portTICK_PERIOD_MS);
}
