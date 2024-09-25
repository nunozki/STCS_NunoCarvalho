// ThermalControlApp.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <unistd.h>  // Para pipe(), fork(), read(), write()
#include <cstring>   // Para strlen()
#include <cstdlib>   // Para rand(), srand()
#include <ctime>     // Para time()
#include <sys/types.h>
#include <sys/wait.h>

int fd[2]; // Descritores da pipe
void createPipe(); // Método para criar a pipe
void readFromPipe(); // Método para ler da pipe
void writeToPipe(const char* message); // Método para escrever na pipe
void simulateTemperature(); // Método para simular a temperatura
float currentTemperature; // Variável para armazenar a temperatura atual

// TODO: Reference additional headers your program requires here.
