#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <locale.h>

struct Obraz {

	char nazwa[30];
	char standard[3];
	int szerokosc_obrazu;
	int wysokosc_obrazu;
	int max_szarosc;
	int** tablica;
};


bool wczytaj(struct Obraz* obrazy, int ilosc_obrazow) {

	char nazwa[30];
	printf("Podaj nazwe pliku zawierajacego obraz:\n");
	scanf(" %s", nazwa);
	

	strcat(nazwa, "\0");
	strcat(nazwa, ".pgm");					//dodajemy na koncu nazwy pliku ".pgm" aby program wiedzial ktory plik otworzyc
	

	FILE* plik = fopen(nazwa, "r");			//"r" oznacza, ze plik do odczytu

	char tekst[3];
	int liczba;
	if (plik != NULL) {

		strcpy(obrazy[ilosc_obrazow].nazwa, nazwa);

		//pobieranie typu
		fscanf(plik, "%s", tekst);
		if (tekst[0] != 'P' && tekst[1] != '2') {
			printf("Nie jest to typ P2");
			return false;
		}
		else {
			strcpy(obrazy[ilosc_obrazow].standard, strcat(tekst, "\0"));
		}

		bool pobrano = false;
		//pobieranie szerokosci
		while (pobrano == false) {
			if (fscanf(plik, "%d", &liczba) == 0) {
				while (fgetc(plik) != '\n') {			//fgetc zwraca kolejny znak w pliku oraz przesuwa wskaznik w pliku o 1 w prawo
					
				}
				
			}
			else {
				obrazy[ilosc_obrazow].szerokosc_obrazu = liczba;
				pobrano = true;
			}
		}


		//pobieranie wysokosci
		fscanf(plik, "%d", &liczba);
		obrazy[ilosc_obrazow].wysokosc_obrazu = liczba;


		pobrano = false;
		//pobnieranie glebi szarosci
		while (pobrano == false) {
			if (fscanf(plik, "%d", &liczba) == 0) {
				while (fgetc(plik) != '\n') {
					
				}
			}
			else {
				obrazy[ilosc_obrazow].max_szarosc = liczba;
				pobrano = true;
			}
		}


		//alokacja pamieci
		obrazy[ilosc_obrazow].tablica = calloc(obrazy[ilosc_obrazow].wysokosc_obrazu, sizeof(int*));

		for(int i=0; i<obrazy[ilosc_obrazow].wysokosc_obrazu; i++){
			
			obrazy[ilosc_obrazow].tablica[i] = calloc(obrazy[ilosc_obrazow].szerokosc_obrazu, sizeof(int));

	
			if(obrazy[ilosc_obrazow].tablica[i] == NULL){			//zabezpieczenie przed wyciekiem w razie nieudanej alokacji
				for (int j = 0; j <= i; j++) {
					free(obrazy[ilosc_obrazow].tablica[j]);
				}
				free(obrazy[ilosc_obrazow].tablica);
				return false;
			}
		}

		int i = 0;
		int j = -1;
		bool liczby_w_linii = false;

		while (fgetc(plik) != EOF) {
			fseek(plik, -1, SEEK_CUR);								//cofam sie o 1 poniewaz fgetc() przesunelo go o 1 w prawo
			if (liczby_w_linii == true) {
				i++;
			}
			if (i == obrazy[ilosc_obrazow].wysokosc_obrazu) {
				break;
			}
			liczby_w_linii = false;
			j = -1;
			while (fgetc(plik) != '\n' && fgetc(plik) != EOF && i < obrazy[ilosc_obrazow].wysokosc_obrazu && j < obrazy[ilosc_obrazow].szerokosc_obrazu) {
				fseek(plik, -2, SEEK_CUR);
				j++;

				if (fscanf(plik, "%d", &liczba) != 0) {
					
					liczby_w_linii = true;
					obrazy[ilosc_obrazow].tablica[i][j] = liczba;

					//polykanie spacji gdy jest na koncu linii
					if (fgetc(plik) == ' ') {
						if (fgetc(plik) == '\n') {
							break;
						}
						else {
							fseek(plik, -2, SEEK_CUR);
						}
					}
					else{
						fseek(plik, -2, SEEK_CUR);
						if (fgetc(plik) == '\n') {
							//fseek(plik, -1, SEEK_CUR);
							break;
						}
					}
				}
				else {
					if (liczby_w_linii == false) {
						j = -1;
					}
					while (fgetc(plik) != '\n' && fgetc(plik) != EOF) {
						fseek(plik, -1, SEEK_CUR);
					}
					fseek(plik, -2, SEEK_CUR);
				}

			}
			
		}
		fclose(plik);

	}
	else {
		printf("Blad otwarcia pliku\n");
		return false;
	}

	return true;
}


void wyswietl(struct Obraz* obrazy, int* ilosc_obrazow) {
	
	printf("Lista obrazow w bazie: \n");
	for (int i = 0; i < (*ilosc_obrazow); i++) {
		printf("%d. %s\n", i+1, obrazy[i].nazwa);
	}
	printf("\n");
}

void usun(struct Obraz* obrazy, int* ilosc_obrazow, int* index) {

	for (int i = 0; i < obrazy[*index].wysokosc_obrazu; i++)
	{
		free(obrazy[*index].tablica[i]);
	}
	free(obrazy[*index].tablica);
	for (int i = *index; i < (*ilosc_obrazow)-1; i++)
	{
		obrazy[i] = obrazy[i + 1];
	}
	*ilosc_obrazow -= 1;
	*index = -1;
}

void zapisz(struct Obraz* obrazy, int ilosc_obrazow, int index) {

	char nazwa_pliku[30];
	printf("Podaj nazwe pliku do ktorego zapisac obraz: ");
	scanf("%s", nazwa_pliku);
	strcat(nazwa_pliku, ".pgm");

	FILE* plik = fopen(nazwa_pliku, "w");					//"w" otwarcie do zapisu

	fprintf(plik, "%s\n", obrazy[index].standard);
	fprintf(plik, "%d %d\n", obrazy[index].szerokosc_obrazu, obrazy[index].wysokosc_obrazu);
	fprintf(plik, "%d\n", obrazy[index].max_szarosc);


	for (int i = 0; i < obrazy[index].wysokosc_obrazu; i++) {
		for (int j = 0; j < obrazy[index].szerokosc_obrazu; j++) {
			fprintf(plik, "%d ", obrazy[index].tablica[i][j]);
		}
		if (i != obrazy[index].wysokosc_obrazu - 1) {
			fprintf(plik, "\n");
		}
		
	}

	fclose(plik);
}


void aktywny_obraz(int ilosc_obrazow, int* index) {

	int tmp;
	
	printf("Podaj numer obrazu ktory ma byc aktywny: ");
	scanf("%d", &tmp);
	if (tmp < 1 || tmp > ilosc_obrazow) {
		printf("Wybrano zly obraz\n");
	}
	else {
		*index = tmp - 1;
	}
}

void negatyw(struct Obraz* obrazy, int index) {


	for (int i = 0; i < obrazy[index].wysokosc_obrazu; i++) {
		for (int j = 0; j < obrazy[index].szerokosc_obrazu; j++) {
			obrazy[index].tablica[i][j] = obrazy[index].max_szarosc - obrazy[index].tablica[i][j];
		}
	}

}

void szum(struct Obraz* obrazy, int index) {

	int ilosc_pikseli = obrazy[index].szerokosc_obrazu * obrazy[index].wysokosc_obrazu / 10;
	srand(time(NULL));

	for (int n = 0; n < ilosc_pikseli; n++) {
		
		int i = rand() % obrazy[index].wysokosc_obrazu;
		int j = rand() % obrazy[index].szerokosc_obrazu;

		if (n % 2 == 0) {												//co drugi wylosowany piksel zmieniam na czarno, co drugi na bialo
			obrazy[index].tablica[i][j] = 0;
		}
		else {
			obrazy[index].tablica[i][j] = obrazy[index].max_szarosc;
		}
	}

}

void histogram(struct Obraz* obrazy, int index) {

	int* tablica_wystapien = calloc(obrazy[index].max_szarosc + 1, sizeof(int));
	int tmp = 0;

	for (int i = 0; i < obrazy[index].wysokosc_obrazu; i++) {
		for (int j = 0; j < obrazy[index].szerokosc_obrazu; j++) {
			tmp = obrazy[index].tablica[i][j];
			tablica_wystapien[tmp]++;
		}
	}

	char nazwa[30];
	printf("Podaj nazwe pliku do ktorego zapisac histogram: ");
	scanf("%s", nazwa);
	strcat(nazwa, ".CSV");
	FILE* plik = fopen(nazwa, "w");								//"w" otwarcie pliku do zapisu

	for (int i = 0; i < obrazy[index].max_szarosc + 1; i++) {
		fprintf(plik, "%d;", tablica_wystapien[i]);
	}

	fclose(plik);

	free(tablica_wystapien);
}

void obrot(struct Obraz* obrazy, int index) {

	int nowa_wysokosc = obrazy[index].szerokosc_obrazu;
	int nowa_szerokosc = obrazy[index].wysokosc_obrazu;

	//alokacja nowej tablicy
	int** nowa_tablica = calloc(nowa_wysokosc, sizeof(int*));

	for (int i = 0; i < nowa_wysokosc; i++) {
		nowa_tablica[i] = calloc(nowa_szerokosc, sizeof(int));
		if (nowa_tablica[i] == NULL) {
			for (int j = 0; j <= i; j++) {
				free(nowa_tablica[j]);
			}
			free(nowa_tablica);

			return;
		}
	}



	for (int i = 0; i < nowa_wysokosc; i++) {						//transponowanie macierzy
		for (int j = 0; j < nowa_szerokosc; j++) {
			 nowa_tablica[i][j] = obrazy[index].tablica[j][i];
		}
	}

	//zwalnianie pamieci
	for (int i = 0; i < obrazy[index].wysokosc_obrazu; i++)
	{
		free(obrazy[index].tablica[i]);
	}
	free(obrazy[index].tablica);

	obrazy[index].szerokosc_obrazu = nowa_szerokosc;
	obrazy[index].wysokosc_obrazu = nowa_wysokosc;

	//alokacja nowego romiaru pamieci
	obrazy[index].tablica = calloc(nowa_wysokosc, sizeof(int*));
	for (int i = 0; i < nowa_wysokosc; i++) {
		obrazy[index].tablica[i] = calloc(nowa_szerokosc, sizeof(int));
		if (obrazy[index].tablica[i] == NULL) {
			for (int j = 0; j <= i; j++) {
				free(obrazy[index].tablica[j]);
			}
			free(obrazy[index].tablica);

			return false;
		}
	}

	for (int i = 0; i < nowa_wysokosc; i++) {
		for (int j = 0; j < nowa_szerokosc; j++) {
			obrazy[index].tablica[i][j] = nowa_tablica[i][nowa_szerokosc - j - 1];
		}
	}


	for (int i = 0; i < nowa_wysokosc; i++)
	{
		free(nowa_tablica[i]);
	}
	free(nowa_tablica);

}

int main() {

	int ilosc_obrazow = 0;
	int index = -1;
	int kat = 0;
	struct Obraz* obrazy = NULL;
	

	do {

		int opcja = 0;

		printf("%s", "Wybierz akcje: \n"
			"1. Wczytaj obraz\n"
			"2. Pokaz liste obrazow\n"
			"3. Wybierz aktywny obraz\n"
			"4. Usun aktywny obraz z listy\n"
			"5. Obroc aktywny obraz\n"
			"6. Negatyw aktywnego obrazu\n"
			"7. Szum pieprz i sol\n"
			"8. Histogram\n"
			"9. Zapisz aktywny obraz\n");
		if (index >= 0 && index < ilosc_obrazow) {
			printf("Aktywny obraz: %d\n", index  + 1);
		}
		else {
			printf("Aktywny obraz: brak\n");
		}
		
		scanf("%d", &opcja);
		switch (opcja) {
		case 1:

			ilosc_obrazow = ilosc_obrazow+1;
			obrazy = realloc(obrazy, ilosc_obrazow * sizeof(struct Obraz));
			
			if (obrazy == NULL) {
				printf("Blad alokacji, przerwanie dzialania");
				return 1;
			}
			
			if (wczytaj(obrazy, ilosc_obrazow - 1) == false) {
				ilosc_obrazow = ilosc_obrazow - 1;
				obrazy = realloc(obrazy, ilosc_obrazow * sizeof(struct Obraz));
				printf("Nie udalo sie wczytac obrazu\n");
			}
			break;
		case 2:

			wyswietl(obrazy, &ilosc_obrazow);
			break;

		case 3: 
			
			aktywny_obraz(ilosc_obrazow, &index);
			break;
		case 4:

			if (index >= 0 && index < ilosc_obrazow) {
				usun(obrazy, &ilosc_obrazow, &index);
				obrazy = realloc(obrazy, ilosc_obrazow * sizeof(struct Obraz));
				if (obrazy == NULL && ilosc_obrazow != 0) {
					printf("Blad alokacji, przerwanie dzialania");
					return 1;
				}
			}
			else {
				printf("Nie ma aktywnego obrazu\n");
			}
			break;

		case 5:

			printf("O ile stopni obrocic obraz w prawo? (k*90): ");
			scanf("%d", &kat);
			if (index >= 0 && index < ilosc_obrazow) {
				for (int i = 0; i < kat / 90; i++) {
					obrot(obrazy, index);
				}
			}
			else {
				printf("Nie ma aktywnego obrazu\n");
			}
			break;

		case 6:
			if (index >= 0 && index < ilosc_obrazow) {
				negatyw(obrazy, index);
			}
			else {
				printf("Nie ma aktywnego obrazu\n");
			}
			break;

		case 7:
			if (index >= 0 && index < ilosc_obrazow) {
				szum(obrazy, index);
			}
			else {
				printf("Nie ma aktywnego obrazu\n");
			}
			break;
		case 8:
			if (index >= 0 && index < ilosc_obrazow) {
				histogram(obrazy, index);
			}
			else {
				printf("Nie ma aktywnego obrazu\n");
			}
			break;
		case 9:

			if (index >= 0 && index < ilosc_obrazow) {
				zapisz(obrazy, ilosc_obrazow, index);
			}
			else {
				printf("Nie ma aktywnego obrazu\n");
			}
			break;

		default:
			printf("Koniec programu");
			for (int i = 0; i < ilosc_obrazow; i++) {
				usun(obrazy, &ilosc_obrazow, &i);
			}
			free(obrazy);
			return 1;
		}

	} while (1);

}