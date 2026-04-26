#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ENCODER_A 35
#define ENCODER_B 34

volatile long encoderCount = 0;
volatile uint8_t lastEncoded = 0;

bool encoder_updated = false;

void IRAM_ATTR updateEncoder();

int count_1 = 0;
int count_2 = 0;

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
		1000,
		NULL,
		1,
		NULL
	);
}

void loop()
{
	Serial.println("loop test");
	delay(1000 / portTICK_PERIOD_MS);
}

void task_1(void* parameters)
{
	
	// while (true)
	// {
	// 	Serial.print("Task 1 counter: ");
	// 	Serial.println(count_1++);
	// 	vTaskDelay(1000 / portTICK_PERIOD_MS);
	// }
	
}

void task_2(void* parameters)
{
	while (true)
	{
		Serial.print("Task 2 counter: ");
		Serial.println(count_2++);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}

}

void IRAM_ATTR updateEncoder()
{

	uint8_t MSB = digitalRead(ENCODER_A);
	uint8_t LSB = digitalRead(ENCODER_B);

	uint8_t encoded = (MSB << 1) | LSB;
	uint8_t sum = (lastEncoded << 2) | encoded;

	if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderCount++;
	if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderCount--;

	lastEncoded = encoded;
	encoder_updated = true;
}
