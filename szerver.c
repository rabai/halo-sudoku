#include <asm/byteorder.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

#define N 9
int sudoku[N][N];
int noDeleteX[81], noDeleteY[81];
char string[256];
char kuld[256];

void kiir(char string[256], char kuld[256]);

int kitolt(int x, int y, int szam);

int torol(int, int);

int isUnique(int tomb[]);

int sorKulomboz(int szam, int tabla[][9]);

int oszlopKulomboz(int szam, int tabla[][9]);

int haromKulomboz(int tabla[][9]);

int isKitoltve();

int isKesz(int sudoku[][9]);

int main(int argc, char *argv[])
{
	/*------------szerver----------------*/

	int szerverSocket, kliens1Socket, kliens2Socket;
	struct sockaddr_in szerver, kliens1, kliens2;
	int szerverMeret = sizeof(szerver), kliens1Meret = sizeof(kliens1),
			kliens2Meret = sizeof(kliens2);
	int portNo = 51716;

	if (argv[1] != NULL)
	{
		portNo = atoi(argv[1]);
	}

	
	szerverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (szerverSocket < 0)
		error("Hiba, nem sikerült megnyitni a portot!");

	//a serverAddr feltöltése
	szerver.sin_family = AF_INET;       
	szerver.sin_port = htons(portNo); 
	szerver.sin_addr.s_addr = INADDR_ANY; 

	if (bind(szerverSocket, (struct sockaddr *) &szerver, szerverMeret) < 0)
		error("Hiba, nem sikerült címhez kötni");


	listen(szerverSocket, 5);

	/*------------kliensek----------------*/
	int kliensSzam = 0;
	printf("első kliens csatlakozzon\n");
	kliens1Socket = accept(szerverSocket, (struct sockaddr*) &kliens1,
			&kliens1Meret);
	send(kliens1Socket, "siker;", 256, 0);
	++kliensSzam;

	printf("második kliens csatlakozzon\n");
	kliens2Socket = accept(szerverSocket, (struct sockaddr*) &kliens2,
			&kliens2Meret);
	send(kliens2Socket, "siker;", 256, 0);
	++kliensSzam;

	if (kliensSzam == 2)
	{
		//init
		char str[2];
		char ssudoku[999] = "";

		FILE *file;
		file = fopen("Start", "r");
		if (file)
		{
			while (fscanf(file, "%s", str) != EOF)
			{
				strcat(ssudoku, str);
			}
		}
		int i, j, z = 0;

		for (i = 0; i < N; i++)
		{
			for (j = 0; j < N; j++)
			{
				sudoku[i][j] = (int) ssudoku[z] - 48;
				z++;
			}
		}

		int x, y, it = 0;
		for (x = 0; x < 9; x++)
		{
			for (y = 0; y < 9; y++)
			{
				if (sudoku[x][y] != 0)
				{
					noDeleteX[it] = x;
					noDeleteY[it] = y;
					it++;
				}
			}
		}

		send(kliens1Socket, "transfer;", 256, 0);
		kiir(string, kuld);
		send(kliens1Socket, string, 256, 0);

		strcpy(string, "");
		strcpy(kuld, "");
		send(kliens2Socket, "transferElso;", 256, 0);
		kiir(string, kuld);
		send(kliens2Socket, string, 256, 0);

		if (isKitoltve() == 1)
		{
			if (isKesz(sudoku) == 1)
			{
				printf("Helyes kitöltés\n");
				send(kliens1Socket, "kesz;", 256, 0);
				send(kliens2Socket, "kesz;", 256, 0);
				close(kliens1Socket);
				close(kliens2Socket);
				close(szerverSocket);
				exit(0);
			}
			else
				printf("Helytelen kitöltés\n");
		}
		fclose(file);
	}

	char buffer[256];
	int k1Bezarva = 0, k2Bezarva = 0;

	while (k1Bezarva != 1 && k2Bezarva != 1)
	{
		char *token, *xChar, *yChar, *szamChar;
		char *nyit = "(";
		char *vesszo = ",";
		char *bezar = ")";
		char beKitolt[] = "kitolt";
		char beTorol[] = "torol";
		char bePassz[] = "passz";
		char beFelad[] = "feladom";
		int x = 0, y = 0, szam = 0;
		int x1, y1;
		char string[256];
		char kuld[256];

		PASSZ1: printf("első kliens:\n");
		LOOP1: recv(kliens1Socket, buffer, 1024, 0);

		token = strtok(buffer, nyit);
		if (strcmp(token, bePassz) == 0)
		{
			printf("következő játékos\n");
			send(kliens1Socket, "passz;", 256, 0);
			if (k2Bezarva == 1)
			{
				send(kliens1Socket, "transfer;", 256, 0);
				strcpy(string, "");
				strcpy(kuld, "");
				kiir(string, kuld);
				send(kliens1Socket, string, sizeof string, 0);
				goto PASSZ1;
			}
			else
			{
				send(kliens2Socket, "transfer;", 256, 0);
				strcpy(string, "");
				strcpy(kuld, "");
				kiir(string, kuld);
				send(kliens2Socket, string, sizeof string, 0);
				goto PASSZ2;
			}
		}
		else if (strcmp(token, beFelad) == 0)
		{
			printf("kliens1 bezárva\n");
			k1Bezarva = 1;
			send(kliens1Socket, "exit;", 256, 0);
			if (k2Bezarva == 1)
			{
				break;
			}
			else
				strcpy(string, "");
			strcpy(kuld, "");
			kiir(string, kuld);
			send(kliens2Socket, string, sizeof string, 0);
			close(kliens1Socket);
		}
		else if (strcmp(token, beKitolt) == 0)
		{
			token = strtok(NULL, vesszo);
			xChar = token;
			token = strtok(NULL, vesszo);
			yChar = token;
			token = strtok(NULL, bezar);
			szamChar = token;
			x = atoi(xChar);
			y = atoi(yChar);
			szam = atoi(szamChar);
			int fi = kitolt(x, y, szam);
			if (fi == 1)
			{
				printf(
						"Már volt itt szám! Innen csak törölni lehet. v. 1-9 közötti számot kell megadni!\n");
				send(kliens1Socket, "again;", 256, 0);
				send(kliens1Socket, "rosszKitolt;", 256, 0);
				goto LOOP1;
			}
			else if (x == 0 || y == 0 || szam == 0)
			{
				printf("Rossz üzenet, próbálkozz újra! \n");
				send(kliens1Socket, "again;", 256, 0);
				send(kliens1Socket, "rosszKitolt;", 256, 0);
				goto LOOP1;
			}
			else
			{
				kitolt(x, y, szam);
				int i, j;
				for (i = 0; i < 9; ++i)
				{
					for (j = 0; j < 9; ++j)
					{
						printf("%d ", sudoku[i][j]);
					}
					printf("\n");
				}

				if (isKitoltve() == 1)
				{
					if (isKesz(sudoku) == 1)
					{
						printf("Helyes kitöltés\n");
						send(kliens1Socket, "kesz;", 256, 0);
						send(kliens2Socket, "kesz;", 256, 0);
						close(kliens1Socket);
						close(kliens2Socket);
						close(szerverSocket);
						exit(0);
					}
					else
						printf("Helytelen kitöltés\n");
				}
				else
				{
					if (k2Bezarva == 1)
					{
						send(kliens1Socket, "transfer;", 256, 0);
						strcpy(string, "");
						strcpy(kuld, "");
						kiir(string, kuld);
						send(kliens1Socket, string, sizeof string, 0);
						goto PASSZ1;
					}
					else
					{
						send(kliens2Socket, "transfer;", 256, 0);
						strcpy(string, "");
						strcpy(kuld, "");
						kiir(string, kuld);
						send(kliens2Socket, string, sizeof string, 0);
						send(kliens1Socket, "var;", 256, 0);
					}
				}
			}
		}
		else if (strcmp(token, beTorol) == 0)
		{
			token = strtok(NULL, vesszo);
			xChar = token;
			token = strtok(NULL, bezar);
			yChar = token;
			int x = atoi(xChar);
			int y = atoi(yChar);
			int fi = torol(x, y);
			if (fi == 1)
			{
				printf(
						"Kezdő pontot nem lehet törölni.\n v. Ezen a mezőn nincs adat! Ide csak beszúrni lehet.\n");
				send(kliens1Socket, "again;", 256, 0);
				send(kliens1Socket, "rosszTorol;", 256, 0);
				goto LOOP1;
			}
			else if (x == 0 || y == 0)
			{
				printf("Rossz üzenet, próbálkozz újra! \n");
				send(kliens1Socket, "again;", 256, 0);
				send(kliens1Socket, "rosszTorol;", 256, 0);
				goto LOOP1;
			}
			else
			{
				torol(x, y);
				int i, j;
				for (i = 0; i < 9; ++i)
				{
					for (j = 0; j < 9; ++j)
					{
						printf("%d ", sudoku[i][j]);
					}
					printf("\n");
				}

				if (k2Bezarva == 1)
				{
					send(kliens1Socket, "transfer;", 256, 0);
					strcpy(string, "");
					strcpy(kuld, "");
					kiir(string, kuld);
					send(kliens1Socket, string, sizeof string, 0);
					goto PASSZ1;
				}
				else
				{
					//send(kliens2Socket, "var;", 256, 0);
					send(kliens2Socket, "transfer;", 256, 0);
					strcpy(string, "");
					strcpy(kuld, "");
					kiir(string, kuld);
					send(kliens2Socket, string, sizeof string, 0);
					send(kliens1Socket, "var;", 256, 0);
				}
			}
		}
		else
		{
			printf("Rossz üzenet, próbálkozz újra! \n");
			send(kliens1Socket, "again;", 256, 0);
			send(kliens1Socket, "rosszUzenet;", 256, 0);
			goto LOOP1;
		}

		printf("Kliens1 üzenete: %s\n", buffer);
		strcpy(buffer, "");

		if (k1Bezarva == 1 && k2Bezarva == 1)
			break;

		PASSZ2: printf("második kliens:\n");
		LOOP2: recv(kliens2Socket, buffer, 256, 0);

		token = strtok(buffer, nyit);
		if (strcmp(token, bePassz) == 0)
		{
			printf("következő játékos\n");
			send(kliens2Socket, "passz;", 256, 0);
			if (k1Bezarva == 1)
			{
				send(kliens2Socket, "transfer;", 256, 0);
				strcpy(string, "");
				strcpy(kuld, "");
				kiir(string, kuld);
				send(kliens2Socket, string, sizeof string, 0);
				goto PASSZ2;
			}
			else
			{
				send(kliens1Socket, "transfer;", 256, 0);
				strcpy(string, "");
				strcpy(kuld, "");
				kiir(string, kuld);
				send(kliens1Socket, string, sizeof string, 0);
				goto PASSZ1;
			}
		}
		else if (strcmp(token, beFelad) == 0)
		{
			printf("kliens2 bezárva\n");
			send(kliens2Socket, "exit;", 256, 0);
			k2Bezarva = 1;
			if (k1Bezarva == 1)
			{
				break;
			}
			else
				send(kliens1Socket, "transfer;", 256, 0);
			strcpy(string, "");
			strcpy(kuld, "");
			kiir(string, kuld);
			send(kliens1Socket, string, sizeof string, 0);
			close(kliens2Socket);
		}
		else if (strcmp(token, beKitolt) == 0)
		{
			token = strtok(NULL, vesszo);
			xChar = token;
			token = strtok(NULL, vesszo);
			yChar = token;
			token = strtok(NULL, bezar);
			szamChar = token;
			int x = atoi(xChar);
			int y = atoi(yChar);
			int szam = atoi(szamChar);
			int fi = kitolt(x, y, szam);
			if (fi == 1)
			{
				printf("Már volt itt szám! Innen csak törölni lehet.\n");
				send(kliens2Socket, "again;", 256, 0);
				send(kliens2Socket, "rosszKitolt;", 256, 0);
				goto LOOP2;
			}
			if (x == 0 || y == 0 || szam == 0)
			{
				printf("Rossz üzenet, próbálkozz újra! \n");
				send(kliens2Socket, "again;", 256, 0);
				send(kliens2Socket, "rosszKitolt;", 256, 0);
				goto LOOP2;
			}
			else
			{
				kitolt(x, y, szam);
				int i, j;
				for (i = 0; i < 9; ++i)
				{
					for (j = 0; j < 9; ++j)
					{
						printf("%d ", sudoku[i][j]);
					}
					printf("\n");
				}

				if (isKitoltve() == 1)
				{
					if (isKesz(sudoku) == 1)
					{
						printf("Helyes kitöltés\n");
						send(kliens1Socket, "kesz;", 256, 0);
						send(kliens2Socket, "kesz;", 256, 0);
						close(kliens1Socket);
						close(kliens2Socket);
						close(szerverSocket);
						exit(0);
					}
					else
						printf("Helytelen kitöltés\n");
				}
				else
				{
					if (k1Bezarva == 1)
					{
						send(kliens2Socket, "transfer;", 256, 0);
						strcpy(string, "");
						strcpy(kuld, "");
						kiir(string, kuld);
						send(kliens2Socket, string, sizeof string, 0);
						goto PASSZ2;
					}
					else
					{
						send(kliens1Socket, "transfer;", 256, 0);
						strcpy(string, "");
						strcpy(kuld, "");
						kiir(string, kuld);
						send(kliens1Socket, string, sizeof string, 0);
						send(kliens2Socket, "var;", 256, 0);
					}
				}
			}
		}
		else if (strcmp(token, beTorol) == 0)
		{
			token = strtok(NULL, vesszo);
			xChar = token;
			token = strtok(NULL, bezar);
			yChar = token;
			int x = atoi(xChar);
			int y = atoi(yChar);
			int fi = torol(x, y);
			if (fi == 1)
			{
				printf(
						"Kezdő pontot nem lehet törölni.\n v. Ezen a mezőn nincs adat! Ide csak beszúrni lehet.\n");
				send(kliens2Socket, "again;", 256, 0);
				send(kliens2Socket, "rosszTorol;", 256, 0);
				goto LOOP2;
			}
			if (x == 0 || y == 0)
			{
				printf("Rossz üzenet, próbálkozz újra! \n");
				send(kliens2Socket, "again;", 256, 0);
				send(kliens2Socket, "rosszTorol;", 256, 0);
				goto LOOP2;
			}
			else
			{
				torol(x, y);
				int i, j;
				for (i = 0; i < 9; ++i)
				{
					for (j = 0; j < 9; ++j)
					{
						printf("%d ", sudoku[i][j]);
					}
					printf("\n");
				}

				if (k1Bezarva == 1)
				{
					send(kliens1Socket, "transfer;", 256, 0);
					strcpy(string, "");
					strcpy(kuld, "");
					kiir(string, kuld);
					send(kliens1Socket, string, sizeof string, 0);
					goto PASSZ2;
				}
				else
				{
					//send(kliens1Socket, "var;", 256, 0);
					send(kliens1Socket, "transfer;", 256, 0);
					strcpy(string, "");
					strcpy(kuld, "");
					kiir(string, kuld);
					send(kliens1Socket, string, sizeof string, 0);
					send(kliens2Socket, "var;", 256, 0);
				}
			}
		}
		else
		{
			printf("Rossz üzenet, próbálkozz újra! \n");
			send(kliens2Socket, "again;", 256, 0);
			send(kliens2Socket, "rosszUzenet;", 256, 0);
			goto LOOP2;
		}

		printf("Kliens2 üzenete: %s\n", buffer);
		strcpy(buffer, "");
	}
	close(szerverSocket);
	exit(0);
}

void kiir(char string[256], char kuld[256])
{
	int x1, y1;

	strcpy(string, "");
	strcpy(kuld, "");
	for (x1 = 0; x1 < 9; ++x1)
	{
		for (y1 = 0; y1 < 9; ++y1)
		{
			snprintf(kuld, sizeof kuld, "%d", sudoku[x1][y1]);
			strcat(string, kuld);
			strcat(string, ";");
		}
	}
}

int isUnique(int tomb[])
{
	int ki = 0;
	int i, j;
	for (i = 0; i < 9; i++)
	{
		for (j = i + 1; j < 9; j++)
		{
			if (tomb[i] != tomb[j])
			{
				ki = 1;
			}
			else
			{
				ki = 0;
				return ki;
			}
		}
	}
	return ki;
}

int kitolt(int x, int y, int szam)
{
	int ki = 0;
	if (szam > 0 && szam < 10)
	{
		int sor = x - 1, oszlop = y - 1;
		if (sudoku[sor][oszlop] == 0)
			sudoku[sor][oszlop] = szam;
		else
		{
			//printf("Már volt itt szám! Innen csak törölni lehet.\n");
			ki = 1;
		}
	}
	else
	{
		//printf("1-9 közötti számot kell megadni!\n");
		ki = 1;
	}
	return ki;
}

int torol(int x, int y)
{
	int benneVan = 0;
	int i;
	for (i = 0; i < 9; i++)
	{
		if ((x - 1 == noDeleteX[i]) && (y - 1 == noDeleteY[i]))
		{
			//printf("Kezdő pontot nem lehet törölni.\n");
			benneVan = 1;
		}
	}
	if (benneVan != 1)
	{
		int sor = x - 1, oszlop = y - 1;
		if (sudoku[sor][oszlop] != 0)
		{
			sudoku[sor][oszlop] = 0;
			printf("Törölve\n");
		}
		else
		{
			//printf("Ezen a mezőn nincs adat! Ide csak beszúrni lehet.\n");
			benneVan = 1;
		}
	}
	return benneVan;
}

int sorKulomboz(int szam, int tabla[][9])
{
	int ki = 0;
	int i, j;
	for (i = 0; i < 9; i++)
	{
		if (i == 8)
			break;
		for (j = i + 1; j < 9; j++)
		{
			if (tabla[szam][i] != tabla[szam][j])
				ki = 1;
			else
			{
				ki = 0;
				return ki;
			}
		}
	}
	return ki;
}

int oszlopKulomboz(int szam, int tabla[][9])
{
	int ki = 0;
	int i, j;
	for (i = 0; i < 9; i++)
	{
		for (j = i + 1; j < 9; j++)
		{
			if (tabla[i][szam] != tabla[j][szam])
				ki = 1;
			else
			{
				ki = 0;
				return ki;
			}
		}
	}
	return ki;
}

int haromKulomboz(int tabla[][9])
{
	int ki = 0;
	int t1[9], t2[9], t3[9], k = 0, l = 0, h = 0, i, j, x;
	for (i = 0; i < 9; i++)
	{
		for (j = 0; j < 9; j++)
		{
			if (j < 3 && i < 3)
			{
				t1[k] = tabla[i][j];
				k++;
			}
			else if (j < 6 && i < 3)
			{
				t2[l] = tabla[i][j];
				l++;
			}
			else if (j < 9 && i < 3)
			{
				t3[h] = tabla[i][j];
				h++;
			}
			else if (j < 3 && i < 6)
			{
				t1[k] = tabla[i][j];
				k++;
			}
			else if (j < 6 && i < 6)
			{
				t2[l] = tabla[i][j];
				l++;
			}
			else if (j < 9 && i < 6)
			{
				t3[h] = tabla[i][j];
				h++;
			}
			else if (j < 3 && i < 9)
			{
				t1[k] = tabla[i][j];
				k++;
			}
			else if (j < 6 && i < 9)
			{
				t2[l] = tabla[i][j];
				l++;
			}
			else if (j < 9 && i < 9)
			{
				t3[h] = tabla[i][j];
				h++;
			}
			if ((i == 2 && j == 8) || (i == 5 && j == 8) || (i == 8 && j == 8))
			{
				if (isUnique(t1) == 1 && isUnique(t2) == 1 && isUnique(t3) == 1)
					ki = 1;
				else
				{
					ki = 0;
					return ki;
				}
				k = 0;
				l = 0;
				h = 0;
				break;
			}
		}
	}

	return ki;
}

int isKitoltve()
{
	int ki = 0;
	int i, j;
	for (i = 0; i < 9; ++i)
	{
		for (j = 0; j < 9; ++j)
		{
			if (sudoku[i][j] != 0)
			{
				ki = 1;
			}
			else if (sudoku[i][j] == 0)
			{
				ki = 0;
				return ki;
				break;
			}
		}
	}
	return ki;
}

int isKesz(int tabla[][9])
{
	int ki = 0;
	int i, j;
	for (i = 0; i < 9; ++i)
	{
		if (sorKulomboz(i, tabla) == 1 && oszlopKulomboz(i, tabla) == 1
				&& haromKulomboz(tabla) == 1)
			ki = 1;
		else
		{
			ki = 0;
			return ki;
			break;
		}
	}

	return ki;
}

