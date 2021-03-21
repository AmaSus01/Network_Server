#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>

extern errno; 	//глобальная переменная, которая хранит код последней ошибки


//проверка, задан ли шаблон INADDR_NONE, который обозначает сразу все доступные сетевые интерфейсы
//на некоторых платформах, он может быть не задан.
#ifndef INADDR_NONE
#define INADDR_NONE 0xfffffffff
#endif


//функция создания и связывания сокета. объявление
//аргументы:
//port - порт, с которым связывается сервер
//transport - протокол, по которому будет работать сервер (tcp или udp)
//q - длина  очереди
int sock(const char *port, const char *transport, int q);


int sock(const char *port, const char *transport, int qlen)
{
	struct protoent *ppe;		
	struct sockaddr_in sin;
	int s, type;
	//обнуляем структуру адреса
	memset(&sin, 0, sizeof(sin));
	//указываем тип адреса - IPv4, для IPv6 необходимо указать AF_INET6
	sin.sin_family = AF_INET;
	//указываем, в качестве адреса, шаблон INADDR_ANY - все сетевые интерфейсы
	sin.sin_addr.s_addr = INADDR_ANY;
	//задаем порт
	sin.sin_port = htons((unsigned short)atoi(port));
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
	//создаем сокет
	s = socket(PF_INET, type, ppe->p_proto);
	if(s < 0)
		{
			printf("Ошибка создания сокета: %s\n", strerror(errno));	//в случае неудачи выводим сообщение ошибки
			return -1;
		}
	//привязка сокета с проверкой результата
	if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		{
			printf("Ошибка связывания сокета: %s\n", strerror(errno));	//в случае неудачи выводим сообщение ошибки
			return -1;
		}
	//запуск прослушивания с проверкой результата
	if(type == SOCK_STREAM && listen(s, qlen) <0)
		{
			printf("Ошибка прослушивания сокета: %s\n", strerror(errno));	//в случае неудачи выводим сообщение ошибки
			return -1;
		}
	return s;	//возвращаем дескриптор сокета
}

int main()
{
	int msock, csock;				//дескрипторы сокетов
	struct sockaddr_in  remaddr;			//структура IP-адреса клиента
	unsigned int remaddrs = sizeof(remaddr);	//размер структуры адреса
	char msg[21];					//буфер сообщения
	
	msock = sock("1231", "tcp", 5);	//создаем tcp сокет и привязываем его к порту 3123, задав очередь 5
	if(msock < 0)			//проверяем значение дескриптора сокета
		return -1;		//завершаем программу

	while(1)	//бесконечный цикл
	{
		csock = accept(msock, (struct sockaddr*) &remaddr, &remaddrs);	//принимаем входящее подключение, адрес клиента в remaddr
		if(csock < 0)		//проверяем результат
			printf("Ошибка принятия подключения: %s\n", strerror(errno)); //сообщение об ошибке
		else			//если все нормально - начинаем обмен данными с клиентом
			{
				if(read(csock, &msg, sizeof(msg)) >0 )		//пробуем читать данные от клиента
				{
					if(strstr(msg, "hello"))			//если получено "hello"
					{
						memset(&msg, 0, sizeof(msg));				//обнуляем буфер
						strcpy(msg, "Hello Ivan, ");						//формируем строку ответа
						strcat(msg, inet_ntoa(remaddr.sin_addr));	//преобразовываем адрес клиента в строку
						strcat(msg, " !!!\n\0");					//завершаем строку ответа
						write(csock, msg, sizeof(msg));			//отсылаем ответ
					}
				}
				close(csock);		//закрываем сокет клиента
			}
	}	
	close(msock);		//закрываем сокет сервера
	return 0;
}