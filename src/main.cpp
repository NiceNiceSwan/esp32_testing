#include <Arduino.h>
#include "rotation_stepper_controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ENCODER_A 35
#define ENCODER_B 34
#define PUL_MINUS 25
#define ENA_MINUS 27
#define DIR_MINUS 35

volatile long encoderCount = 0;
volatile uint8_t lastEncoded = 0;
double angle = 0.0;
SemaphoreHandle_t angle_mutex = NULL;
Rotation_stepper_controller rotation_stepper(ENA_MINUS, DIR_MINUS, PUL_MINUS);

void updateEncoder();

void task_1(void* parameters);
void task_2(void* parameters);

void setup()
{
	// pinMode(ENA_MINUS, OUTPUT);
	// pinMode(DIR_MINUS, OUTPUT);
	// pinMode(PUL_MINUS, OUTPUT);
	// digitalWrite(ENA_MINUS, HIGH);
	// digitalWrite(DIR_MINUS, HIGH);
	Serial.begin(9600);
	Serial.println("Starting program.");

	pinMode(ENCODER_A, INPUT_PULLUP);
	pinMode(ENCODER_B, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(ENCODER_A), updateEncoder, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENCODER_B), updateEncoder, CHANGE);

	angle_mutex = xSemaphoreCreateMutex();

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
	// digitalWrite(PUL_MINUS, HIGH);
	// delayMicroseconds(500);
	// digitalWrite(PUL_MINUS, LOW);
	// delayMicroseconds(500);
	// Serial.println("looping");
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
		count = encoderCount;
		interrupts();
		count %= 4096;


		double revolutions = count / 4096.0;
		double _angle = revolutions * 360.0;
		if (xSemaphoreTake(angle_mutex, 1 / portTICK_PERIOD_MS))
		{
			angle = _angle;
			xSemaphoreGive(angle_mutex);
		}

		lastCount = count;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void task_2(void* parameters)
{
	while (true)
	{
		// Serial.println("looping");
		// Serial.print("Task 2 angle: ");
		double _angle = 0;
		if (xSemaphoreTake(angle_mutex, 1 / portTICK_PERIOD_MS))
		{
			_angle = angle;
			xSemaphoreGive(angle_mutex);
		}
		
		// Serial.println(_angle);
		// // analogWrite(PUL_MINUS, 255);
		// digitalWrite(PUL_MINUS, HIGH);
		// vTaskDelay(200 / portTICK_PERIOD_MS);
		// digitalWrite(PUL_MINUS, LOW);
		// // analogWrite(PUL_MINUS, 0);
		
		rotation_stepper.handle_movement();
		if (Serial.available()) {
			String input = Serial.readStringUntil('\n');
			input.trim();
			rotation_stepper.take_serial_input(input);
		}
	}

}

void updateEncoder()
{

	uint8_t MSB = digitalRead(ENCODER_A);
	uint8_t LSB = digitalRead(ENCODER_B);

	uint8_t encoded = (MSB << 1) | LSB;
	uint8_t sum = (lastEncoded << 2) | encoded;

	if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderCount++;
	if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderCount--;

	lastEncoded = encoded;
}
