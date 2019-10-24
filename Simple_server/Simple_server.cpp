/*
Приложение-сервер. Пользователь пишет сообщение и отправляет его. Если сообщение от пользователя
непустое, приложение-клиент соединяется к сокету приложения-сервера и отправляет сообщение пользователя
на сервер. Сервер обрабатывает полученное сообщение (а именно, переводит символы из нижнего регистра в
верхний и наоборот) и отправляет обработанное сообщение обратно клиенту. На этом соединение между клиентом
и сервером обрывается. Пользователь получает сообщение и может отправить на сервер новое сообщение.
Чтобы выйти из приложения-клиента, нужно ввести пустое сообщение (нажать ENTER).
*/

#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#define DEFAULT_BUFLEN 500
#define DEFAULT_PORT 27015

// Функция переводит символы из нижнего регистра в верхний и наоборот
void upper(char* rec_buf, char* send_buf, int size)
{
	int i = 0;
	const int CODE_a = int('a');
	const int CODE_A = int('A');
	const int CODE_z = int('z');
	const int CODE_Z = int('Z');
	while (i < size)
	{
		if (CODE_a <= int(rec_buf[i]) && int(rec_buf[i]) <= CODE_z)
		{
			send_buf[i] = char(int(rec_buf[i]) + CODE_A - CODE_a);
		}
		else if (CODE_A <= int(rec_buf[i]) && int(rec_buf[i]) <= CODE_Z)
		{
			send_buf[i] = char(int(rec_buf[i]) + CODE_a - CODE_A);
		}
		else
		{
			send_buf[i] = rec_buf[i];
		}
		i++;
	}
}

int main(int argc, char** argv)
{
	// Инициализируем Winsock
	WSADATA wsaData = { 0 };
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	// Создаем серверный сокет для IPv4 с протоколом TCP
	SOCKET server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_s == INVALID_SOCKET)
	{
		printf("Socket function failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	// Объявим переменную для хранения адреса
	sockaddr_in s_address;
	// Тип адреса (TCP/IP)
	s_address.sin_family = AF_INET;
	// Порт; используем функцию htons для перевода номера порта из обычного числа в TCP/IP представление
	s_address.sin_port = htons(DEFAULT_PORT);
	// Адреса клиентов могут быть любыми
	s_address.sin_addr.s_addr = htonl(INADDR_ANY);
	// Привязываем адреса и порт к серверному сокету
	iResult = bind(server_s, (sockaddr*)&s_address, sizeof(s_address));
	if (iResult == SOCKET_ERROR)
	{
		printf("Bind failed with error = %d\n", WSAGetLastError());
		closesocket(server_s);
		WSACleanup();
		return 1;
	}
	// Переводим сокет в режим приема новых соединеий
	iResult = listen(server_s, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("Listen failed with error = %d\n", WSAGetLastError());
		closesocket(server_s);
		WSACleanup();
		return 1;
	}
	printf("Server is ready!\n");
	while (true)
	{
		// Новое соединение с сокетом клиента
		sockaddr_in client_s_addr;
		int addr_len = sizeof(client_s_addr);
		int client_s = accept(server_s, (sockaddr*)&client_s_addr, &addr_len);
		if (client_s == INVALID_SOCKET)
		{
			// Если при соединении произошла ошибка
			printf("Accept failed with error: %d\n", WSAGetLastError());
			closesocket(server_s);
			WSACleanup();
			return 1;
		}
		else
		{
			// Если соединение установлено
			printf("\nClient connected.\n");
			//printf("Client IP: %s\n", inet_ntop((in_addr)client_s_addr.sin_addr));
			printf("Client port: %d\n", ntohs(client_s_addr.sin_port));
		}
		// Получаем сообщение от клиента
		char rec_buffer[DEFAULT_BUFLEN]; // буфер для хранения полученных данных
		char send_buffer[DEFAULT_BUFLEN]; // буфер для хранения отправляемых данных
		int size = recv(client_s, rec_buffer, sizeof(rec_buffer), 0);
		if (size == SOCKET_ERROR)
		{
			// Если произошла ошибка при чтении сообщений
			printf("Receive failed with error: %d\n", WSAGetLastError());
			closesocket(client_s);
			closesocket(server_s);
			WSACleanup();
			return 1;
		}
		upper(rec_buffer, send_buffer, size); // обработка полученного сообщения
		send(client_s, send_buffer, size, 0); // отправка сообщения
		// Соединение с клиентом закрывается
		iResult = shutdown(client_s, SD_BOTH);
		if (iResult == SOCKET_ERROR)
		{
			printf("Shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(client_s);
			WSACleanup();
			return 1;
		}
		closesocket(client_s);
		// Полученное и отправленное сообщения выводятся на экран
		printf("Received message: ");
		int i = 0;
		while (i < size)
		{
			printf("%c", rec_buffer[i]);
			i++;
		}
		printf("\nSent message: ");
		i = 0;
		while (i < size)
		{
			printf("%c", send_buffer[i]);
			i++;
		}
		printf("\n");
	}
	closesocket(server_s);
	WSACleanup();
	return 0;
}