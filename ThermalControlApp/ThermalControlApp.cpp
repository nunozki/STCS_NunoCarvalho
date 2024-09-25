// ThermalControlApp.cpp : Defines the entry point for the application.
//

#include "ThermalControlApp.h"

using namespace std;

void simulateTemperature() {
	// Simula a temperatura em um intervalo, por exemplo, entre 20 e 30 graus Celsius
	currentTemperature = 20.0 + (float)rand() / (float)(RAND_MAX / (30.0 - 20.0));
}

void writeToPipe(const char* message) {
	write(fd[1], message, strlen(message) + 1); // Escreve dados na pipe
}

void readFromPipe() {
	char buffer[100];
	ssize_t bytesRead = read(fd[0], buffer, sizeof(buffer));
	if (bytesRead > 0) {
		printf("Dados lidos da pipe: %s\n", buffer);
	}
	else {
		printf("Erro ao ler da pipe ou pipe vazia.\n");
	}
}

void createPipe() {
	if (pipe(fd) == -1) {
		printf("Erro ao criar pipe\n");
		return;
	}

	pid_t pid = fork();
	if (pid < 0) {
		printf("Erro ao criar processo filho\n");
		return;
	}
	else if (pid == 0) { // Processo filho
		close(fd[1]); // Fecha o lado de escrita da pipe
		readFromPipe(); // Lê da pipe
		close(fd[0]); // Fecha o lado de leitura da pipe
		exit(0);
	}
	else { // Processo pai
		close(fd[0]); // Fecha o lado de leitura da pipe
		simulateTemperature(); // Simula a temperatura
		char message[100];
		snprintf(message, sizeof(message), "Temperatura: %.2f°C", currentTemperature);
		writeToPipe(message); // Escreve a temperatura na pipe
		close(fd[1]); // Fecha o lado de escrita da pipe
		wait(NULL); // Espera o filho terminar
	}
}

int main() {
	printf("Hello CMake.\n");
	createPipe(); // Cria a pipe ao inicializar o aplicativo
	srand((unsigned int)time(NULL)); // Inicializa o gerador de números aleatórios
	return 0;
}
