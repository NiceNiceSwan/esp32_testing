#include <Arduino.h>
#include "rotation_stepper_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// red is VCC, black is ground
// green cable
#define ENCODER_A 34
// white cable
#define ENCODER_B 35
#define PUL_PLUS 33
#define DIR_PLUS 25

volatile long encoderCount = 0;
volatile uint8_t lastEncoded = 0;
double angle = 0.0;
SemaphoreHandle_t angle_mutex = NULL;
SemaphoreHandle_t encoder_mutex = NULL;
Rotation_stepper_controller rotation_stepper(DIR_PLUS, PUL_PLUS);

void updateEncoder();

void task_1(void* parameters);
void task_2(void* parameters);

void setup()
{
	Serial.begin(9600);
	Serial.println("Starting program.");

	pinMode(ENCODER_A, INPUT_PULLUP);
	pinMode(ENCODER_B, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(ENCODER_A), updateEncoder, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENCODER_B), updateEncoder, CHANGE);

	angle_mutex = xSemaphoreCreateMutex();
	encoder_mutex = xSemaphoreCreateMutex();

	xTaskCreate(
		task_1,
		"task 1",
		1000,
		NULL,
		1,
		NULL
	);
	xTaskCreate(
		task_2,
		"task 2",
		4096,
		NULL,
		1,
		NULL
	);
}

void loop()
{
}

void task_1(void* parameters)
{
	while (true)
	{
	
	static long lastCount = 0;
	if (lastCount != encoderCount)
	{

		long count;
		noInterrupts();
		if (xSemaphoreTake(encoder_mutex, 1 / portTICK_PERIOD_MS))
		{
			count = encoderCount;
			xSemaphoreGive(encoder_mutex);
		}
		interrupts();
		count %= 4096;


		double revolutions = count / 4096.0;
		double _angle = revolutions * 360.0;
		if (_angle < 0)
		{
			_angle += 360;
		}
		
		if (xSemaphoreTake(angle_mutex, 1 / portTICK_PERIOD_MS))
		{
			angle = _angle;
			xSemaphoreGive(angle_mutex);
		}

		lastCount = count;
	}
	vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}

void task_2(void* parameters)
{
	while (true)
	{
		double _angle = 0;
		if (xSemaphoreTake(angle_mutex, 1 / portTICK_PERIOD_MS))
		{
			_angle = angle;
			xSemaphoreGive(angle_mutex);
		}
		
		rotation_stepper.handle_movement();
		rotation_stepper.current_angle(_angle);
		if (Serial.available()) {
			String input = Serial.readStringUntil('\n');
			input.trim();
		
			// this is fucking retarded
			// but we basically have to do this because I spaghettified myself.
			// basically, when we issue a set home command to the stepper, it needs to tell the encoder
			// that are now on position one no matter what
			Command command = rotation_stepper.take_serial_input(input);
			if (command == Command::SET_HOME)
			{
				if (xSemaphoreTake(encoder_mutex, 1 / portTICK_PERIOD_MS))
				{
					encoderCount = 0;
					lastEncoded = 0;
					xSemaphoreGive(encoder_mutex);
				}
			}
		}
		vTaskDelay(1/portTICK_PERIOD_MS);
	}

}

void updateEncoder()
{

	uint8_t MSB = digitalRead(ENCODER_B);
	uint8_t LSB = digitalRead(ENCODER_A);

	uint8_t encoded = (MSB << 1) | LSB;
	uint8_t sum = (lastEncoded << 2) | encoded;

	if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderCount++;
	if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderCount--;

	lastEncoded = encoded;
}
