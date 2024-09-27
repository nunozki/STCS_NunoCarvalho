// ThermalControlApp.c : Defines the entry point for the application.

#include "ThermalControlApp.h"

int infoPipe[2];
int responsePipe[2];
bool simulateTemperatureActive = false;
bool thermalControlEnabled = false;
float currentTemperature = 25.0f;
float setpointTemperature = 20.0f;
PIDController pidController = { 1.0f, 0.1f, 0.01f, 0.0f, 0.0f }; // Exemplo de PID
pthread_t simulationThread;
pthread_t menuThread;

#define MAX_BUFFER_SIZE 256  // Tamanho máximo do buffer para mensagens

#define MIN_TEMPERATURE -25.0f
#define MAX_TEMPERATURE 25.0f

#define MIN_INTEGRAL_VALUE -10.0f   // Valor mínimo para a integral
#define MAX_INTEGRAL_VALUE 10.0f     // Valor máximo para a integral

#define MAX_OUTPUT 100.0f // Define o limite máximo da saída do PID
#define MIN_OUTPUT -100.0f // Define o limite mínimo da saída do PID

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

void adjustTemperature(float adjustment) {
	// Se o controlo térmico estiver ativado
	if (thermalControlEnabled) {
		// Aumenta a temperatura com base no ajuste
		currentTemperature += adjustment;

		// Exibir temperatura atual independentemente dos limites
		printf("Current Temperature: %.2f\n", currentTemperature);

		// Limitar a temperatura dentro dos limites
		if (currentTemperature > MAX_TEMPERATURE) {
			printf("Maximum Temperature Reached: %.2f. Decreasing temperature...\n", currentTemperature);
			currentTemperature -= 0.5f; // Diminuir um pouco a temperatura
		}
		else if (currentTemperature < MIN_TEMPERATURE) {
			printf("Minimum Temperature Reached: %.2f. Increasing temperature...\n", currentTemperature);
			currentTemperature += 0.5f; // Aumentar um pouco a temperatura
		}
	}
	else {
		// Se o controlo térmico não estiver ativado, diminui constantemente a temperatura
		currentTemperature -= 0.5f; // Ajusta para diminuir a temperatura

		// Limitar a temperatura dentro do limite inferior
		if (currentTemperature < MIN_TEMPERATURE) {
			currentTemperature = MIN_TEMPERATURE; // Limite inferior
		}
	}

	// Formata a mensagem para o pipe
	char message[MAX_BUFFER_SIZE];
	snprintf(message, sizeof(message), "THERM-01_TEMP-%.2f;", currentTemperature);
	writeToInfoPipe(message);

	// Mostre a temperatura no terminal
	printf("Adjusted Temperature: %.2f (Adjustment: %.2f)\n", currentTemperature, adjustment);
}

void* simulateTemperature(void* arg) {
	float dt = 1.0f; // Intervalo de tempo (em segundos) para simulação
	simulateTemperatureActive = true;
	clearTerminal();

	while (simulateTemperatureActive) {
		float controlOutput = 0.0f;
		float error = setpointTemperature - currentTemperature;

		// Armazenar a temperatura atual antes de qualquer ajuste
		float previousTemperature = currentTemperature;


		char buffer[MAX_BUFFER_SIZE]; // Buffer para armazenar a mensagem do pipe

		if (thermalControlEnabled) {
			// Cálculo do controle PID
			controlOutput = pidController.Kp * error +
				pidController.Ki * (pidController.integral) + // Integral
				pidController.Kd * (error - pidController.previousError); // Derivativo

			// Atualiza o estado anterior do erro e a integral
			pidController.previousError = error;
			pidController.integral += error;

			// Limitar a integral para evitar windup
			if (pidController.integral > MAX_INTEGRAL_VALUE) {
				pidController.integral = MAX_INTEGRAL_VALUE; // Um valor máximo
			}
			else if (pidController.integral < MIN_INTEGRAL_VALUE) {
				pidController.integral = MIN_INTEGRAL_VALUE; // Um valor mínimo
			}

			// Limitar a saída do controle
			if (controlOutput > MAX_OUTPUT) {
				controlOutput = MAX_OUTPUT;
			}
			else if (controlOutput < MIN_OUTPUT) {
				controlOutput = MIN_OUTPUT;
			}

			printf("Error: %.2f, Control Output Before Limits: %.2f\n", error, controlOutput);
			printf("Control Output After Limits: %.2f\n", controlOutput);

			// Ajusta a temperatura baseado na saída de controle
			adjustTemperature(controlOutput);
		}
		else {
			// Se o controlo térmico não estiver ativado, diminuir constantemente a temperatura
			if (currentTemperature > MIN_TEMPERATURE) {
				currentTemperature -= 0.5f; // Ajusta para diminuir a temperatura
				printf("Thermal Control Disabled. Decreasing Temperature: %.2f\n", currentTemperature);
			}
			else {
				printf("Minimum Temperature Reached: %.2f\n", currentTemperature);
			}
		}

		// Prepara a mensagem para o pipe
		snprintf(buffer, sizeof(buffer), "Current Temperature: %.2f, Control Output: %.2f", currentTemperature, controlOutput);
		writeToInfoPipe(buffer); // Escreve no pipe

		// Ajusta o intervalo da simulação
		usleep(500000);  // Simulação a cada 0.5 segundos
	}

	return NULL;
}

// Função para escrever na infoPipe
void writeToInfoPipe(const char* message) {
	if (write(infoPipe[1], message, strlen(message) + 1) == -1) {
		perror("Failed to write to infoPipe");
	}
}

// Função para ler da infoPipe
void readFromInfoPipe(char* buffer, size_t bufferSize) {
	if (read(infoPipe[0], buffer, bufferSize) == -1) {
		perror("Failed to read from infoPipe");
	}
	snprintf(buffer, bufferSize, "Simulated data from pipe");
}

// Função para escrever na responsePipe
void writeToResponsePipe(const char* message) {
	if (write(responsePipe[1], message, strlen(message) + 1) == -1) {
		perror("Failed to write to responsePipe");
	}
}

// Função para ler da responsePipe
void readFromResponsePipe(char* buffer, size_t bufferSize) {
	if (read(responsePipe[0], buffer, bufferSize) == -1) {
		perror("Failed to read from responsePipe");
	}
}

// Função para configurar os parâmetros PID
void setPIDParameters(float kp, float ki, float kd) {
	if (kp < 0 || ki < 0 || kd < 0) {
		printf("PID parameters must be non-negative.\n");
		return;
	}

	pidController.Kp = kp;
	pidController.Ki = ki;
	pidController.Kd = kd;
	pidController.previousError = 0.0;
	pidController.integral = 0.0;
	printf("PID parameters set: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", kp, ki, kd);
}

float calculatePIDControl(float error) {
	// Calcular a derivada
	float derivative = error - pidController.previousError;

	// Atualizar o erro anterior
	pidController.previousError = error;

	// Acumular a integral do erro
	// Limitar a integral para evitar windup
	if (currentTemperature >= MAX_TEMPERATURE && error > 0) {
		// Prevenir acumulação da integral se a temperatura atual estiver saturada
		pidController.integral = fmaxf(0.0f, pidController.integral);
	}
	else if (currentTemperature <= MIN_TEMPERATURE && error < 0) {
		// Prevenir acumulação da integral se a temperatura atual estiver saturada
		pidController.integral = fminf(0.0f, pidController.integral);
	}
	else {
		// Se a temperatura não estiver saturada, permita a acumulação
		pidController.integral += error; // Acumula o erro
	}

	// Calcular a saída do PID
	float output = pidController.Kp * error +
		pidController.Ki * pidController.integral +
		pidController.Kd * derivative;

	// Limitar a saída a uma faixa específica
	if (output > MAX_OUTPUT) {
		output = MAX_OUTPUT; // Limitar a saída ao máximo permitido
	}
	else if (output < MIN_OUTPUT) {
		output = MIN_OUTPUT; // Limitar a saída ao mínimo permitido
	}

	return output; // Retornar a saída calculada
}

// Função para definir o setpoint de temperatura
void setSetpoint(float value) {
	float newSetpoint;
	while (1) {
		printf("Enter new setpoint temperature (%.2f to %.2f): ", MIN_TEMPERATURE, MAX_TEMPERATURE);

		if (scanf("%f", &newSetpoint) == 1) {
			if (newSetpoint >= MIN_TEMPERATURE && newSetpoint <= MAX_TEMPERATURE) {
				setpointTemperature = newSetpoint;
				printf("Setpoint temperature set to: %.2f\n", setpointTemperature);
				break; // Sai do loop quando a entrada é válida
			}
			else {
				printf("Setpoint value must be between %.2f and %.2f.\n", MIN_TEMPERATURE, MAX_TEMPERATURE);
			}
		}
		else {
			printf("Invalid input. Please enter a numeric value.\n");
			while (getchar() != '\n'); // Limpa o buffer de entrada
		}
	}
}

// Função para definir a temperatura atual
void setCurrentTemperature(float value) {
	if (value >= MIN_TEMPERATURE && value <= MAX_TEMPERATURE) {
		currentTemperature = value;
		printf("Current temperature set to: %.2f\n", currentTemperature);
	}
	else {
		printf("Error: Current temperature must be between %.2f and %.2f\n", MIN_TEMPERATURE, MAX_TEMPERATURE);
	}
}

void reads()
{
	while (true)
	{
		printf("Reading temperature from infoPipe...\n");
		char buffer[MAX_BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));  // Limpar o buffer
		readFromInfoPipe(buffer, sizeof(buffer));  // Lê a mensagem do pipe

		if (strlen(buffer) > 0) {
			printf("Received: %s\n", buffer);  // Exibe a mensagem recebida
		}
		else {
			printf("No data received from pipe.\n");
		}

		// Exibir temperatura atual e parâmetros do PID
		printf("Current Temperature: %.2f, Setpoint: %.2f\n", currentTemperature, setpointTemperature);
		printf("PID Controller Values:\n");
		printf(" Kp: %.2f, Ki: %.2f, Kd: %.2f\n", pidController.Kp, pidController.Ki, pidController.Kd);
		printf(" Previous Error: %.2f, Integral: %.2f\n", pidController.previousError, pidController.integral);

		// Verifica se a tecla ESC foi pressionada
		if (getchar() == 27) { // 27 é o código ASCII para ESC
			printf("ESC pressed, exiting...\n");
			break;
		}

		usleep(500000);  // Simulação a cada 0.5 segundos
	}
	return;
}

void* menuInput(void* arg) {
	char command[MAX_BUFFER_SIZE];
	int firstOption = -1;  // Armazena a primeira escolha
	int secondOption = -1; // Armazena a segunda escolha

	int option = 0;

	do {
		clearTerminal(); // Limpa o terminal a cada iteração
		printf("Thermal Control Application\n");

		if (firstOption == -1) {
			// Se nenhuma opção foi escolhida ainda
			printf("1. Enable Thermal Control\n");
			printf("2. Disable Thermal Control\n");
		}
		else if (secondOption == -1) {
			// Se uma opção já foi escolhida, remove a possibilidade de repetição
			if (firstOption == 1) {
				printf("2. Disable Thermal Control\n");
			}
			else {
				printf("1. Enable Thermal Control\n");
			}
		}

		// Exibe as outras opções
		printf("3. Set PID Parameters\n");
		printf("4. Set Setpoint Temperature\n");
		printf("5. Set Current Temperature\n");
		printf("6. Read from Pipe\n");
		printf("7. Exit\n");

		printf("Choose an option: ");

		if (fgets(command, sizeof(command), stdin) != NULL) {
			option = atoi(command);

			if (firstOption == -1) {
				// Primeira escolha: ou ativa ou desativa o controlo térmico
				if (option == 1 || option == 2) {
					firstOption = option;
					if (option == 1) {
						thermalControlEnabled = true;
						printf("Thermal Control Enabled\n");
					}
					else if (option == 2) {
						thermalControlEnabled = false;
						printf("Thermal Control Disabled\n");
					}
				}
				else if (option == 7) {
					thermalControlEnabled = false;
					simulateTemperatureActive = false;
					pthread_join(simulationThread, NULL); // Aguardar a conclusão da simulação;
					printf("Exiting program...\n");
					exit(0);
					continue;
				}
				else {
					printf("Invalid option. Please choose 1 or 2.\n");
					continue;
				}
			}
			else if (secondOption == -1) {
				// Segunda escolha: a outra opção deve ser selecionada
				if ((firstOption == 1 && option == 2) || (firstOption == 2 && option == 1)) {
					secondOption = option;
					if (option == 1) {
						thermalControlEnabled = true;
						printf("Thermal Control Enabled\n");
					}
					else if (option == 2) {
						thermalControlEnabled = false;
						printf("Thermal Control Disabled\n");
					}
				}
				else if (option == 7) {
					thermalControlEnabled = false;
					simulateTemperatureActive = false;
					pthread_join(simulationThread, NULL); // Aguardar a conclusão da simulação;
					printf("Exiting program...\n");
					exit(0);
					continue;
				}
				else {
					printf("Invalid option. Please choose the remaining option (1 or 2).\n");
					continue;
				}
			}
			else {
				// Permitir apenas ajustes normais após as duas escolhas
				switch (option) {
				case 3: {
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
				case 4:
					setSetpoint(setpointTemperature);
					break;
				case 5:
					float currentTemp;
					printf("Enter Current Temperature %.2f and %.2f:\n", MIN_TEMPERATURE, MAX_TEMPERATURE);
					scanf("%f", &currentTemp);
					setCurrentTemperature(currentTemp);
					break;
				case 6:
					reads();
					break;
				case 7:
					thermalControlEnabled = false;
					simulateTemperatureActive = false;
					pthread_join(simulationThread, NULL); // Aguardar a conclusão da simulação;
					printf("Exiting program...\n");
					exit(0);
					break;
				default:
					printf("Invalid option. Please try again.\n");
					break;
				}
			}
		}
	} while (option != 7);
}

int main() {
	createPipes(); // Cria os pipes
	signal(SIGINT, SIG_IGN); // Ignora o sinal de interrupção

	// Certifique-se de que currentTemperature está dentro dos limites ao iniciar
	if (currentTemperature > MAX_TEMPERATURE) {
		currentTemperature = MAX_TEMPERATURE;
	}
	else if (currentTemperature < MIN_TEMPERATURE) {
		currentTemperature = MIN_TEMPERATURE;
	}

	if (pthread_create(&simulationThread, NULL, simulateTemperature, NULL) != 0) {
		perror("Failed to create simulation thread");
		return EXIT_FAILURE;
	}

	if (pthread_create(&menuThread, NULL, menuInput, NULL) != 0) {
		perror("Failed to create menu thread");
		return EXIT_FAILURE;
	}

	// Aguarda a finalização do menu thread
	pthread_join(menuThread, NULL);

	// Aqui você pode adicionar a limpeza de recursos, se necessário.

	return EXIT_SUCCESS;
}