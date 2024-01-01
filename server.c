/* 1.Tạo các #include cần thiết */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
/* dành riêng cho AF_INET */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h> // Include for threading support
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int count_nam = 0;

void removeSpaces(char *input) {
    int i, j = 0;
    for (i = 0; input[i] != '\0'; i++) {
        if (input[i] != ' ') {
            input[j++] = input[i];
        }
    }
    input[j] = '\0';
}

int calculate(int *numbers, char *operators, int length) {
    // Handle multiplication and division first
	int i = 0, j = 0;
	for (i = 0; i < length; i++) {
        if (operators[i] == '*' || operators[i] == '/') {
            int result;
            switch (operators[i]) {
                case '*':
                    result = numbers[i] * numbers[i + 1];
                    break;
                case '/':
                    // Check if division by zero
                    if (numbers[i + 1] != 0) {
                        result = numbers[i] / numbers[i + 1];
                    } else {
                        printf("Error: Division by zero\n");
                        return 0;
                    }
                    break;
            }
            // Adjust numbers and operators arrays after calculation
			
			for (j = i; j < length - 1; j++) {
                numbers[j] = numbers[j + 1];
                operators[j] = operators[j + 1];
            }
            numbers[i] = result;
            length -= 1;
            i -= 1; // Go back one step to re-evaluate from the current index
        }
    }

    // Then handle addition and subtraction
    for (i = 0; i < length - 1; i++) {
        switch (operators[i]) {
            case '+':
                numbers[i + 1] = numbers[i] + numbers[i + 1];
                break;
            case '-':
                numbers[i + 1] = numbers[i] - numbers[i + 1];
                break;
            default:
                // Handle invalid operator
                printf("Error: Invalid operator\n");
                return 0;
        }
    }
    return numbers[length - 1];
}

int evaluateInParentheses(char *input, int start, int end) {
    int numbers[50]; // Assuming maximum 50 numbers
    char operators[50]; // Assuming maximum 50 operators
    char temp[20]; // Temporary storage for a number
    int numCount = 0, opCount = 0;
    int i = start;

    while(i<end){
        if (input[i] == '(') {
            int j = i + 1;
            int stack = 1;

            while (stack != 0) {
                if (input[j] == '(') {
                    stack++;
                } else if (input[j] == ')') {
                    stack--;
                }
                j++;
            }

            numbers[numCount++] = evaluateInParentheses(input, i + 1, j - 1);
            i = j;
        }
        else if (input[i] == '+' || input[i] == '-'|| input[i] == '*'|| input[i] == '/') {
            operators[opCount++] = input[i++];
        } 
        else if (input[i] >= '0' && input[i] <= '9') {
            int index = 0;
            temp[index++] = input[i++];
            while (input[i] >= '0' && input[i] <= '9') {
                temp[index++] = input[i++];
            }
            temp[index++] = '\0';
            numbers[numCount++] = atoi(temp);
        }
        else {
            i++;
        }
        
    }
    operators[opCount] = '\0';
    return calculate(numbers, operators, opCount + 1);
}

void splitInput(char *input, int *numbers, char *operators) {
    removeSpaces(input);

    int numCount = 0, opCount = 0;
    char temp[20]; // Temporary storage for a number
    int i = 0;

    while (input[i] != '\0') {
        if (input[i] == '(') {
            int j = i + 1;
            int stack = 1;

            while (stack != 0) {
                if (input[j] == '(') {
                    stack++;
                } else if (input[j] == ')') {
                    stack--;
                }
                j++;
            }
            int ducnam_temp = evaluateInParentheses(input, i + 1, j - 1);
            // printf("%d",ducnam_temp);
            numbers[numCount++] = ducnam_temp;
            i = j;
        } else if (input[i] == '+' || input[i] == '-'|| input[i] == '*'|| input[i] == '/') {
            operators[opCount++] = input[i++];
        } else if (input[i] >= '0' && input[i] <= '9') {
            int index = 0;
            temp[index++] = input[i++];
            while (input[i] >= '0' && input[i] <= '9') {
                temp[index++] = input[i++];
            }
            temp[index++] = '\0';
            numbers[numCount++] = atoi(temp);
        } else {
            i++;
        }
    }
    operators[opCount] = '\0';
}

void *handle_client(void *arg) {
    int client_sockfd = *((int *)arg);
    char input[100];
    int numbers[50];
    char operators[50];
    int result;

    printf("Handling client...%d\n",++count_nam);

    // Receive data from the client
    read(client_sockfd, input, sizeof(input));
    input[strcspn(input, "\n")] = 0; // Removing the newline character
    splitInput(input, numbers, operators);

    // Calculate the result
    result = calculate(numbers, operators, strlen(operators) + 1);

    // Send the result back to the client
    write(client_sockfd, &result, sizeof(result));

    // Close the client connection
    close(client_sockfd);
    pthread_exit(NULL);
}

int main()
{
	int server_sockfd, client_sockfd;
	// int server_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	
	/* 2. Thực hiện khởi tạo socket mới cho trình chủ */
	server_sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	
	/* 3. Đặt tên và gán địa chỉ kết nối cho socket theo giao thức Internet */
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	server_address.sin_port = htons( 9734 );
	server_len = sizeof( server_address );
	
	/* 4. Ràng buộc tên với socket */
	bind( server_sockfd, (struct sockaddr *)&server_address, server_len );
	
	/* 5. Mở hàng đợi nhận kết nối - cho phép đặt hàng vào hàng đợi tối đa 5 kết nối */
	listen( server_sockfd, 5 );

	pthread_t thread_id;
	int client_fds[5]; // Assuming handling up to 5 clients
    int client_count = 0;
	
	/* 6. Lặp vĩnh viễn để chờ và xử lý kết nối của trình khách */
	while (1) {
        printf("server waiting...\n");

        // Accept incoming connections
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        if (client_sockfd < 0) {
            perror("Error accepting connection");
            continue;
        }

        if (client_count < 5) { // Limit to 5 clients
            client_fds[client_count++] = client_sockfd;

            // Create a new thread to handle the client
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, handle_client, &client_fds[client_count - 1]);

            // Detach the thread to allow it to terminate independently
            pthread_detach(thread_id);
        } else {
            printf("Server full. Try again later.\n");
            close(client_sockfd); // Close the connection if the server is full
        }
    }

    // Close the server socket (if needed)
    close(server_sockfd);
    return 0;
}