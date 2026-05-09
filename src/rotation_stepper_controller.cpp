#include "rotation_stepper_controller.h"

Rotation_stepper_controller::Rotation_stepper_controller()
{
}

Rotation_stepper_controller::Rotation_stepper_controller(uint8_t direction_pin, uint8_t pulse_pin)
{
    _direction_pin = direction_pin;
    _pulse_pin = pulse_pin;

	pinMode(_direction_pin, OUTPUT);
	pinMode(_pulse_pin, OUTPUT);
}

Rotation_stepper_controller::~Rotation_stepper_controller()
{
}

void Rotation_stepper_controller::attach_pins(uint8_t direction_pin, uint8_t pulse_pin)
{
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
    _direction = delta <= 0;
    digitalWrite(_direction_pin, _direction);
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
    int sign;
    if (angle >= 0)
    {
        sign = 1;
    }
    else
    {
        sign = -1;
    }
    angle = fmod(angle, 360);
    if (sign == 1)
    {
        _target_angle = angle;
    }
    else
    {
        _target_angle = 360 + angle;
    }
    _calculate_direction(_current_angle, _target_angle);
}

void Rotation_stepper_controller::move_by_angle(double angle)
{
    angle = fmod(angle, 360);
    _target_angle = angle + _current_angle;
    if (_target_angle < 0)
    {
        _target_angle += 360;
    }
    
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
    if (_testing_stage)
    {
        vTaskDelay(2500 / portTICK_PERIOD_MS);
    }
    
    switch (_testing_stage)
    {
    case 0:
        Serial.println("Starting test routine");
        Serial.println("Rotate to 90 degrees clockwise");
        move_to_angle(90);
        break;
    case 1:
        Serial.println("Rotate to position 0");
        move_to_origin();
        break;
    case 2:
        Serial.println("Rotate by 90 degrees counter clockwise");
        move_by_angle(-90);
        break;
    case 3:
        Serial.println("Rotate to 90 degrees clockwise");
        move_to_angle(90);
        break;
    case 4:
        Serial.println("Forcing clockwise rotation");
        _forced_direction = directions::CLOCKWISE;
        break;
    case 5:
        Serial.println("Rotate to 90 degrees counter clockwise");
        move_to_angle(-90);
        break;
    case 6:
        Serial.println("Rotate to position 0");
        move_to_origin();
        break;
    case 7:
        Serial.println("Forcing counter clockwise rotation");
        _forced_direction = directions::COUNTER_CLOCKWISE;
        break;
    case 8:
        Serial.println("Rotate to 90 degrees clockwise");
        move_to_angle(90);
        break;
    case 9:
        Serial.println("Rotate to position 0");
        move_to_origin();
        break;
    default:
        Serial.print("Test routine finished\n");
        _testing = false;
        _testing_stage = 0;
        return;
    }
    _testing_stage++;
}

Command Rotation_stepper_controller::take_serial_input(String input)
{
    double target_angle = 0;
    Command command;
    if (_testing)
    {
        return Command::TEST_ROUTINE;
    }
    

    if (isDigit(input.charAt(2)) || input.charAt(2) == '-')
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
        return command;
    case Command::FORCE_DIRECTION_COUNTER_CLOCKWISE:
        Serial.println("Force direction counter clockwise");
        _forced_direction = directions::COUNTER_CLOCKWISE;
        return command;
    case Command::FORCE_DIRECTION_NONE:
        Serial.println("Force direction none");
        _forced_direction = directions::NONE;
        return command;
    case Command::MOVE_ABSOLUTE:
        Serial.println("Move absolute");
        target_angle = input.substring(2).toDouble();
        move_to_angle(target_angle);
        return command;
    case Command::MOVE_RELATIVE:
        Serial.println("Move relative");
        target_angle = input.substring(2).toDouble();
        move_by_angle(target_angle);
        return command;
    case Command::MOVE_TO_ORIGIN:
        Serial.println("Move to origin");
        move_to_origin();
        return command;
    case Command::TEST_ROUTINE:
        Serial.println("Test routine");
        _testing = true;
        _testing_stage = 0;
        test_routine();
        return command;
    case Command::SET_HOME:
        Serial.println("Set home");
        set_home();
        return command;
    default:
        Serial.println("Wrong command");
        return command;
    }

    // just in case
    return command;
}

void Rotation_stepper_controller::handle_movement()
{
    
    double distance_to_target = abs(_current_angle - _target_angle);
    if (_direction == directions::COUNTER_CLOCKWISE && _target_angle > _current_angle)
    {
        distance_to_target = abs(_current_angle - (_target_angle - 360));
    }

    if (distance_to_target < MAX_ANGLE_RESOLUTION)
    {
        if (_running == true)
        {
            _forced_direction = directions::NONE;
        }
        
        if (_testing)
        {
            test_routine();
        }
        _running = false;
        return;
    }
    
    _running = true;
	digitalWrite(_pulse_pin, HIGH);
    vTaskDelay(1/portTICK_PERIOD_MS);
	digitalWrite(_pulse_pin, LOW);
    vTaskDelay(1/portTICK_PERIOD_MS);
}
