#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>

//Maciej Nowak 384013

int main()
{
	//ustawiamy zmienny
	int socketServer; //deskryptor socketa servera
	int port = 5678; //port
	char buffer[1024]; //buffer otrzymanej wiadomosci
	char message[100]; //wysylana wiadomosc
  	struct sockaddr_in server; //struktura socketu dla servera
  	struct hostent * hp; //struktura hosta

	//ustawiamy deskryptor socketu dla servera
	socketServer = socket(PF_INET, SOCK_STREAM, 0);

	//ustawiamy dane servera
  	server.sin_family = AF_INET;
  	hp = gethostbyname("localhost");
  	bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length); //kopiowanie adresu hosta do adresu serwera	
  	server.sin_port = htons(port);

	//nawiazujemy polaczenie lub wyswietlamy info o bledzie
  	if(connect(socketServer, (struct sockaddr *)&server, sizeof(server)) < 0)
  	{
    		printf("Blad polaczenia");
    		return 0;
  	}

  	
	while(1)
	{
		//pobieramy wiadomosc
		scanf("%s", message);
		
		//wysylamy wiadomosc
  		write(socketServer, message, strlen(message));

		//pobieramy wiadomosc i jej dlugosc	
  		int n = read(socketServer, buffer, 1024);

		//wyswietlamy wiadomosc odpowiedzi z serwera
  		if(n > 0)
  		{
    			buffer[n] = '\0';
    			printf("Wiadomosc otrzymana z serwera: %s\n", buffer);
  		}
	}

	//zamykamy socket
  	close(socketServer);

return 0;
}

