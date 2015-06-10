#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void kiir(char buffer[256], char *token2);

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	char egzit[] = "exit";
	char *token, *token2;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];

	char sudoku[9][9] =
	{ 0 };

	if (argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		fprintf(stderr, "Hiba, nincs ilyen host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("Hiba a kapcsolódásnál!");
	else
		while (1)
		{
			R1: bzero(buffer, 256);
			n = recv(sockfd, buffer, 256, 0);
			token = strtok(buffer, ";");
			if (n < 0)
				error("Hiba socketről olvasáskor!");
			if (strcmp(token, "siker") == 0)
			{
				goto R1;
			}
			//R1-be küld
			if (strcmp(token, "transferElso") == 0)
			{
				bzero(buffer, 256);
				recv(sockfd, buffer, 256, 0);
				strcpy(token2, "");
				kiir(buffer, token2);
				goto R1;
			}
			//R2-be küld
			if (strcmp(token, "transfer") == 0)
			{
				bzero(buffer, 256);
				recv(sockfd, buffer, 256, 0);
				strcpy(token2, "");
				kiir(buffer, token2);
				goto R2;
			}
			if (strcmp(token, "var") == 0)
			{
				printf("bejutott a varbe\n");
				goto R1;
			}
			if (strcmp(token, "again") == 0)
			{
				goto R2;
			}
			if (strcmp(token, "rosszKitolt") == 0)
			{
				printf(
						"\nMár volt itt szám! Innen csak törölni lehet.\n v. 1-9 közötti számot kell megadni!\n");
				goto R2;
			}
			if (strcmp(token, "rosszTorol") == 0)
			{
				printf(
						"\nKezdő pontot nem lehet törölni.\n v. Ezen a mezőn nincs adat! Ide csak beszúrni lehet.\n");
				goto R2;
			}
			if (strcmp(token, "rosszUzenet") == 0)
			{
				printf("\nRossz üzenet, próbálkozz újra!\n");
				goto R2;
			}
			if (strcmp(token, "passz") == 0)
			{
				goto R1;
			}
			if (strcmp(token, "kesz") == 0)
			{
				printf("Helyes kitöltés! Szép munka!\n");
				exit(0);
			}
			if (strcmp(token, egzit) == 0)
				exit(0);
			else
				goto R2;
			R2: bzero(buffer, 256);
			printf(
					"\nMegadható üzenetek:\n 1. kitolt(x,y,szam) 2. torol(x,y) 3. passz( 4. feladom(\n");
			printf("Add meg az üzenetet: ");
			bzero(buffer, 256);
			fgets(buffer, 255, stdin);
			n = write(sockfd, buffer, strlen(buffer));
			if (n < 0)
				error("Hiba a socketre írásnál");
			bzero(buffer, 256);
			recv(sockfd, buffer, 256, 0);
			token = strtok(buffer, ";");
			if (strcmp(token, "kesz") == 0)
			{
				printf("Helyes kitöltés! Szép munka!\n");
				exit(0);
			}
			if (strcmp(token, egzit) == 0)
				exit(0);
			else if (strcmp(token, "transfer") == 0)
			{
				bzero(buffer, 256);

				recv(sockfd, buffer, 256, 0);
				strcpy(token2, "");
				kiir(buffer, token2);
				goto R2;
			}
			goto R1;
		}
	close(sockfd);
	return 0;
}

void kiir(char buffer[256], char *token2)
{
	int x = 0, y = 0;
	token2 = strtok(buffer, ";");
	printf("\n");
	while (token2 != NULL)
	{
		printf("%s ", token2);
		y++;
		if (y == 9)
		{
			y = 0;
			printf("\n");
			x++;
		}
		token2 = strtok(NULL, ";");
	}
}
