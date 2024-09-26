// ThermalControlApp.c : Defines the entry point for the application.

#include "ThermalControlApp.h"

int infoPipe[2];
int responsePipe[2];
bool simulateTemperatureActive = false;
bool thermalControlEnabled = false;
float currentTemperature = 0.0;
float setpointTemperature = 0.0; // Setpoint inicial
PIDController pidController; // Controlador PID

// Função para criar pipes
void createPipes() {
	if (pipe(infoPipe) == -1) {
		perror("Failed to create infoPipe");
		exit(EXIT_FAILURE);
	}
	if (pipe(responsePipe) == -1) {
		perror("Failed to create responsePipe");
		exit(EXIT_FAILURE);
	}
}

// Função para limpar o terminal
void clearTerminal() {
#ifdef _WIN32
	system("cls"); // Limpa o terminal no Windows
#else
	system("clear"); // Limpa o terminal em sistemas Unix-like
#endif
}

// Função para simular a temperatura
void* simulateTemperature(void* arg) {
	simulateTemperatureActive = true;

	clearTerminal();

	while (simulateTemperatureActive) {
		currentTemperature = generateRandomTemperature();
		char message[MAX_BUFFER_SIZE];
		snprintf(message, sizeof(message), "THERM-01_TEMP-%.2f-HTR-01_PWR-0;THERM-02_TEMP-%.2f-HTR-02_PWR-0;", currentTemperature, currentTemperature + 1.0);
		writeToInfoPipe(message);
		printf("Simulated temperature: THERM-01_TEMP-%.2f; THERM-02_TEMP-%.2f;\n", currentTemperature, currentTemperature + 1.0);
		sleep(2);  // Ajusta o intervalo da simulação
	}

	return NULL;
}

// Função para escrever na infoPipe
void writeToInfoPipe(const char* message) {
	write(infoPipe[1], message, strlen(message) + 1);
}

// Função para ler da infoPipe
void readFromInfoPipe(char* buffer, size_t bufferSize) {
	read(infoPipe[0], buffer, bufferSize);
}

// Função para escrever na responsePipe
void writeToResponsePipe(const char* message) {
	write(responsePipe[1], message, strlen(message) + 1);
}

// Função para ler da responsePipe
void readFromResponsePipe(char* buffer, size_t bufferSize) {
	read(responsePipe[0], buffer, bufferSize);
}

// Função para gerar temperatura aleatória
float generateRandomTemperature() {
	return (float)(rand() % 401 - 200) / 10.0; // Gera temperatura entre -20.0 e 20.0
}

// Função para configurar os parâmetros PID
void setPIDParameters(float kp, float ki, float kd) {
	pidController.kp = kp;
	pidController.ki = ki;
	pidController.kd = kd;
	pidController.previousError = 0.0;
	pidController.integral = 0.0;
	printf("PID parameters set: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", kp, ki, kd);
}

// Função para definir o setpoint
void setSetpoint(float value) {
	if (value >= -20.0 && value <= 20.0) {
		setpointTemperature = value;
		printf("Setpoint temperature set to %.2f\n", setpointTemperature);
	}
	else {
		printf("Setpoint value must be between -20 and 20.\n");
	}
}

// Função para ativar o controlo térmico
void enableThermalControl() {
	thermalControlEnabled = true;
	printf("Thermal Control Enabled.\n");
}

// Função para desativar o controlo térmico
void disableThermalControl() {
	thermalControlEnabled = false;
	printf("Thermal Control Disabled.\n");
}

// Função principal
int main() {
	int choice;
	srand(time(NULL)); // Inicializa o gerador de números aleatórios
	createPipes(); // Cria os pipes

	// Interface do utilizador
	do {
		clearTerminal(); // Limpa o terminal antes de mostrar o menu

		printf("1. Set PID Parameters\n");
		printf("2. Set Setpoint\n");
		printf("3. Enable Thermal Control\n");
		printf("4. Disable Thermal Control\n");
		printf("5. Simulate Temperature\n");
		printf("6. Stop Temperature Simulation\n");
		printf("7. Exit\n");
		printf("Enter your choice: ");

		scanf("%d", &choice);

		switch (choice) {
		case 1: {
			float kp, ki, kd;
			printf("Enter Kp: ");
			scanf("%f", &kp);
			printf("Enter Ki: ");
			scanf("%f", &ki);
			printf("Enter Kd: ");
			scanf("%f", &kd);
			setPIDParameters(kp, ki, kd);
			break;
		}
		case 2: {
			float setpoint;
			printf("Enter setpoint temperature (-20 to 20): ");
			scanf("%f", &setpoint);
			setSetpoint(setpoint);
			break;
		}
		case 3:
			enableThermalControl();
			break;
		case 4:
			disableThermalControl();
			break;
		case 5: {
			pthread_t simulationThread;
			pthread_create(&simulationThread, NULL, simulateTemperature, NULL);
			break;
		}
		case 6:
			simulateTemperatureActive = false; // Para a simulação
			printf("Temperature simulation stopped.\n");
			break;
		case 7:
			printf("Exiting...\n");
			exit(0);
		default:
			printf("Invalid choice. Please try again.\n");
		}
	} while (choice != 7);

	return 0;
}
