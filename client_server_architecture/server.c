#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

//Maciej Nowak 384013

int main()
{
	//wszystkie zmienne
	int port = 5678; //port
	char buffer[1024]; //buffer wiadomosci
	char *message = "OK"; //wiadomosc zwrotna dla klienta
	int socketServer, socketClient; //deskryptory dla socketow
	int messageLength; //dlugosc otrzymanej wiadomosci
	int clientLenght; //dlugosc klienta
	int clients[10]; //tablica przechowywania klientow
 	int clientsNumber = 0; //ilosc klientow
	int i; //zmienna pomocnicza
	struct sockaddr_in server, client; //struktury socketu dla servera i klienta
	fd_set fileDescriptor; //fd_set - tablica bitow reprezentujaca deskryptory (dla select)
	clientLenght = sizeof(client);

	//ustawiamy deskryptor socketu dla servera
	socketServer = socket(PF_INET, SOCK_STREAM, 0);
	
	//ustawiamy dane servera
 	server.sin_family = AF_INET;
 	server.sin_addr.s_addr = INADDR_ANY;
 	server.sin_port = htons(port);

	//bindujemy socket 
 	bind(socketServer, (struct sockaddr *)&server, (int)sizeof(server));

	//nasluchiwanie na sockecie, 10 oczekujacych klientow
 	listen(socketServer, 10); 
 	printf("Nasluchiwanie na porcie: %d\n", port);

 	while(1)
 	{	
		//obsluga timeoutu
   		struct timeval timeout;
   		timeout.tv_sec = 2;   
   		timeout.tv_usec = 200;  
   	
   		FD_ZERO(&fileDescriptor); //inicjalizacja pliku, 0 dla wszystkich deskryptorow
   		FD_SET(socketServer, &fileDescriptor); //ustawia bity dla socketServer w pliku

		//ustawiamy bity - deskryptor dla kazdego klienta w pliku
   		for(i=0; i<clientsNumber; i++) 
		{
    			FD_SET(clients[i], &fileDescriptor);
   		}

		//pobieramy ilosc polaczen z klientami
   		i = select(FD_SETSIZE, &fileDescriptor, NULL, NULL, &timeout);
   
		//dodajemy nowy deskryptor socketu dla klienta do tablicy klientow
   		if(FD_ISSET(socketServer, &fileDescriptor)) //sprawdzamy czy sa ustawione bity dla socketServer
   		{
     			socketClient = accept(socketServer, (struct sockaddr *)&client, &clientLenght);
     			clients[clientsNumber++] = socketClient;
   		}

		//dla wszystkich klientow
   		for(i=0; i<clientsNumber; i++) 
		{
			//pobieramy deskryptor dla klienta z tablicy
     			int fd = clients[i];
     			if(FD_ISSET(fd, &fileDescriptor)) 
			{
				//odbieramy od klienta o deskrypotrze fd, dlugosc wiadomosci i jej tresc
       				messageLength = recv(fd, buffer, 1024-1, 0);
				//poprawne pobranie wiadomosci, wyswietlenie i wyslanie komunikatu do klienta
      				if(messageLength > 0) 
				{
					buffer[messageLength] = '\0';
         				printf("Wiadomosc otrzymana z deskryptora %d: %s\n", fd, buffer);
         				send(fd, message, strlen(message), 0);
       				} 	
     			}
   		}
 	}

return 0;
}
