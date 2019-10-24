/*
Приложение-клиент. Пользователь пишет сообщение и отправляет его. Если сообщение от пользователя
непустое, приложение-клиент соединяется к сокету приложения-сервера и отправляет сообщение пользователя
на сервер. Сервер обрабатывает полученное сообщение (а именно, переводит символы из нижнего регистра в
верхний и наоборот) и отправляет обработанное сообщение обратно клиенту. На этом соединение между клиентом
и сервером обрывается. Пользователь получает сообщение и может отправить на сервер новое сообщение.
Чтобы выйти из приложения-клиента, нужно ввести пустое сообщение (нажать ENTER).
*/
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#define DEFAULT_BUFLEN 500
#define DEFAULT_PORT 27015

int main(int argc, char** argv)
{
	// Инизиализируем Winsock
	WSADATA wsaData = { 0 };
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	// Объявим переменную для хранения адреса для связи с сервером
	sockaddr_in s_address;
	// Тип адреса (TCP/IP)
	s_address.sin_family = AF_INET;
	// Порт; используем функцию htons для перевода номера порта из обычного числа в TCP/IP представление
	s_address.sin_port = htons(DEFAULT_PORT);
	// Адрес сервера
	s_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	//
	while (true)
	{
		// Пользователю предлагается ввести сообщение
		std::string send_message; // строка для набранного пользователем сообщения
		std::string rec_message; // строка для полученного сообщения
		char rec_buffer[DEFAULT_BUFLEN]; // буфер для принятых данных от сервера
		std::cout << "Enter message to server or enter 'ENTER' to exit: ";
		send_message.clear();
		std::getline(std::cin, send_message);
		if (send_message.empty())
		{
			// Если пользователь хочет завершить работу
			break;
		}
		// Создаем клиентский сокет для IPv4 с протоколом TCP
		SOCKET client_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (client_s == INVALID_SOCKET)
		{
			printf("Socket function failed with error = %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		// Соединяем клиентский сокет к серверному сокету
		iResult = connect(client_s, (sockaddr*)&s_address, sizeof(s_address));
		if (iResult == SOCKET_ERROR)
		{
			// Если при соединении произошла ошибка, сообщаем об этом
			printf("Connect failed with error: %d\n", WSAGetLastError());
			closesocket(client_s);
			WSACleanup();
			return 1;
		}
		// Отправляем сообщение на сервер
		int size = send(client_s, send_message.data(), send_message.size(), 0);
		if (size != send_message.size())
		{
			printf("Send failed. %d bytes were sent instead of %d bytes.\n", size, send_message.size());
		}
		// Получаем сообщения от сервера
		do
		{
			size = recv(client_s, rec_buffer, DEFAULT_BUFLEN, 0);
			if (size == SOCKET_ERROR)
			{
				printf("Receive failed with error: %d\n", WSAGetLastError());
				closesocket(client_s);
				WSACleanup();
				return 1;
			}
			rec_message.append(rec_buffer, size);
		} while (size > 0);
		printf("Answer from server: %s\n", rec_message.data());
		iResult = shutdown(client_s, SD_BOTH);
		if (iResult == SOCKET_ERROR)
		{
			printf("Shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(client_s);
			WSACleanup();
			return 1;
		}
		closesocket(client_s);
	}
	WSACleanup();
	return 0;
}