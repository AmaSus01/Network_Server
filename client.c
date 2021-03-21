#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern errno; 	//глобальная переменная, которая хранит код последней ошибки

//функция подключения к серверу. объявление
//аргументы:
//host - адрес (имя) сервера
//port - порт сервера
//transport - протокол tcp или udp
int connectsock(const char *host, const char *port, const char *transport);



//функция подключения к серверу. реализация
int connectsock(const char *host, const char *port, const char *transport)
{
	struct hostent *phe; //указатель на запись с информацией о хосте
	struct servent *pse; //указатель на запись с информацией о службе
	struct protoent *ppe; //указатель на запись с информацией о протоколе
	struct sockaddr_in sin; //структура IP-адреса оконечной точки 
	int s, type; //дескриптор сокета и тип сокета

	//обнуляем структуру адреса
	memset(&sin, 0, sizeof(sin));
	//указываем тип адреса (IPv4) 
	sin.sin_family = AF_INET;
	//задаем порт
	sin.sin_port = htons((unsigned short)atoi(port)); 	
	//преобразовываем имя хоста в IP-адрес, предусмотрев возможность представить его
	//в точечном десятичном формате
	if(phe = gethostbyname(host))
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	//преобразовываем имя транспортного протокола в номер протокола
	if((ppe = getprotobyname(transport)) == 0)
		{
			printf("Ошибка преобразования имени транспортного протокола: %s\n", strerror(errno));	//в случае неудачи выводим сообщение ошибки
			return -1;			
		}	
	//используем имя протокола для определения типа сокета 	 
	if(strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;			
	//создание сокета
	s = socket(PF_INET, type, ppe->p_proto);
	if(s < 0)
		{
			printf("Ошибка создания сокета: %s\n", strerror(errno));	//в случае неудачи выводим сообщение ошибки
			return -1;
		}
	//попытка подключить сокет
	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		{
			printf("Не удалось подключится к серверу: %s\n", strerror(errno));	//в случае неудачи выводим сообщение ошибки
			return -1;			
		}
	//возвращаем дескриптор подключенного сокета
	return s;
}



//главная функция
int main(int argc, char **argv)
{
	int sock; 	//сокет
	char msg[22];	//буфер сообщения

	if(argc == 3) 	//проверяем количество переданных аргументов.
	{
		//подключаем сокет, в качестве хоста - первый аргумент программы, в качестве порта - второй аргумент программы
		//напомню, что в argv[0] хранится имя самого исполняемого файла программы, поэтому его опускаем.
		sock = connectsock(argv[1], argv[2], "tcp");
		if(sock < 0)	//проверяем дескриптор сокета
			return -1;
		else 		//подключились
			{
				printf("Установлено соединение с %s:%s\n", argv[1], argv[2]);
				strcpy(msg, "hello\0");		//подготавливаем строку сообщения
				if(write(sock, msg, sizeof(msg)) < 0)		//отсылаем серверу
					{
						printf("Не удалось отправить данные серверу: %s\n", strerror(errno));
						return -1;
					}
				printf("Серверу отправлен \"hello\"\n");	//читаем ответ сервера	
				memset(&msg, 0, sizeof(msg));
				if(read(sock, msg, sizeof(msg)) < 0)
					{
						printf("Не удалось отправить данные серверу: %s\n", strerror(errno));
						return -1;
					}
				else				//выводим ответ сервера
					printf("От сервера получено: %s\n", msg);
			close(sock);	//закрываем сокет
			}

	} else	//иначе
		printf("Использование: server \"server\" \"port\"\n");	//выводим подсказку по использованию.
	return 0;
}