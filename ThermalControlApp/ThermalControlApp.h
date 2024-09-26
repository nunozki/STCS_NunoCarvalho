// ThermalControlApp.h : Include file for standard system include files,
// or project specific include files.

#ifndef THERMAL_CONTROL_APP_H
#define THERMAL_CONTROL_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

#define MAX_BUFFER_SIZE 256  // Tamanho máximo do buffer para mensagens

// Declaração de pipes para comunicação
extern int infoPipe[2];      // Pipe para ler as informações de temperatura
extern int responsePipe[2];  // Pipe para responder com o estado dos aquecedores

// Estrutura do controlador PID
typedef struct {
	float kp;                // Ganho proporcional
	float ki;                // Ganho integral
	float kd;                // Ganho derivativo
	float previousError;     // Erro anterior para cálculo do PID
	float integral;          // Soma dos erros para o cálculo do PID
} PIDController;

// Funções para gerenciamento de pipes
void createPipes();                         // Cria os pipes de comunicação
void writeToInfoPipe(const char* message); // Escreve mensagem no infoPipe
void readFromInfoPipe(char* buffer, size_t bufferSize); // Lê mensagem do infoPipe
void writeToResponsePipe(const char* message); // Escreve mensagem no responsePipe
void readFromResponsePipe(char* buffer, size_t bufferSize); // Lê mensagem do responsePipe

// Função para simulação de temperatura
void* simulateTemperature(void* arg); // Função que simula a leitura de temperatura
float generateRandomTemperature(); // Gera uma temperatura aleatória

#endif // THERMAL_CONTROL_APP_H
