// ThermalControlApp.h : Header file for the thermal control application.

#ifndef THERMAL_CONTROL_APP_H
#define THERMAL_CONTROL_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>


// Estrutura PIDController
typedef struct {
	float Kp;
	float Ki;
	float Kd;
	float previousError;
	float integral;
} PIDController;

// Funções do aplicativo
void createPipes();
void clearTerminal();
void adjustTemperature(float adjustment);
void* simulateTemperature(void* arg);
void writeToInfoPipe(const char* message);
void readFromInfoPipe(char* buffer, size_t bufferSize);
void writeToResponsePipe(const char* message);
void readFromResponsePipe(char* buffer, size_t bufferSize);
void setPIDParameters(float kp, float ki, float kd);
float calculatePIDControl(float error);
void setSetpoint(float value);
void setCurrentTemperature(float value);
void* menuInput(void* arg);
void reads();

#endif // THERMAL_CONTROL_APP_H
