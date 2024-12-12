#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#define NAME_LENGTH 100

struct image {
    char name[NAME_LENGTH];
    int width;
    int height;
    int max_gray;
    int** pixels; // Dynamiczna tablica dwuwymiarowa na piksele
};

// Funkcje pomocnicze
void allocate_image(struct image* img);
void free_image(struct image* img);
void copy_image(struct image* dest, struct image* src);
int validate_image(const struct image* img);

// Funkcje operacji na obrazach
bool read_image(struct image* img);
void save_image(const char* filename_base, struct image* img);
void rotate_image_90(struct image* img);
void add_salt_and_pepper_noise(struct image* img, double noise_level);
void gauss_filter(struct image* img);
void generate_histogram(struct image* img, const char* filename);
void negative_image(struct image* img);

// Funkcje bazy obrazów
void add_image(struct image** base, int* size, struct image* new_img);
void remove_image(struct image** base, int* size, int index);
void list_images(struct image* base, int size);
int select_image(int size);
void free_base(struct image** base, int size);

// Alokacja pamięci dla obrazu
void allocate_image(struct image* img) {

    if (img->width <= 0 || img->height <= 0) {
        printf("Nie mozna zaalokowac pamieci dla obrazu o wymiarach %dx%d.\n", img->width, img->height);
        return;
    }


    img->pixels = (int**)malloc(img->height * sizeof(int*));
    if (img->pixels == NULL) {
        fprintf(stderr, "Blad alokacji pamieci dla wierszy pikseli.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < img->height; i++) {
        img->pixels[i] = (int*)malloc(img->width * sizeof(int));
        if (img->pixels[i] == NULL) {
            fprintf(stderr, "Blad alokacji pamieci dla pikseli w wierszu %d.\n", i);
            exit(EXIT_FAILURE);
        }
    }
}

// Zwalnianie pamięci dla obrazu
void free_image(struct image* img) {
    if (img->pixels != NULL) {
        for (int i = 0; i < img->height; i++) {
            free(img->pixels[i]);
        }
        free(img->pixels);
        img->pixels = NULL;
    }
}

// Walidacja obrazu
int validate_image(const struct image* img) {
    if (img->pixels == NULL) {
        fprintf(stderr, "Tablica pikseli nie zostala zaalokowana.\n");
        return 0;
    }
    for (int i = 0; i < img->height; i++) {
        if (img->pixels[i] == NULL) {
            fprintf(stderr, "Wiersz %d tablicy pikseli nie zostal zaalokowany.\n", i);
            return 0;
        }
    }
    return 1;
}

bool read_image(struct image* img) {
    char filename[NAME_LENGTH];
    printf("Podaj nazwe pliku zawierajacego obraz:\n");
    scanf(" %s", filename);

    strcat(filename, ".pgm"); // Dodanie rozszerzenia ".pgm"

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Blad otwarcia pliku %s\n", filename);
        return false;
    }

    char buffer[128];
    int number;

    // Sprawdzanie naglowka
    if (fscanf(file, "%s", buffer) != 1 || strcmp(buffer, "P2") != 0) {
        printf("Nie jest to typ P2\n");
        fclose(file);
        return false;
    }

    // Pomijanie komentarzy
    bool dimensions_read = false;
    while (!dimensions_read) {
        int ch = fgetc(file);
        if (ch == '#') {
            fgets(buffer, sizeof(buffer), file); // Pomijanie calej linii komentarza
        }
        else if (isspace(ch)) {
            continue;
        }
        else {
            ungetc(ch, file); // Cofniecie znaku
            if (fscanf(file, "%d", &number) == 1) {
                img->width = number;
                if (fscanf(file, "%d", &number) == 1) {
                    img->height = number;
                    dimensions_read = true;
                }
            }
            else {
                printf("Blad odczytu wymiarow obrazu\n");
                fclose(file);
                return false;
            }
        }
    }

    // Odczyt maksymalnej szarosci
    if (fscanf(file, "%d", &img->max_gray) != 1) {
        printf("Blad odczytu maksymalnej wartosci szarosci\n");
        fclose(file);
        return false;
    }

    // Alokacja pamieci dla pikseli
    allocate_image(img);

    // Odczyt pikseli
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            if (fscanf(file, "%d", &number) != 1) {
                printf("Blad odczytu pikseli obrazu\n");
                free_image(img);
                fclose(file);
                return false;
            }
            img->pixels[i][j] = number;
        }
    }

    fclose(file);
    strcpy(img->name, filename);
    printf("Obraz %s wczytano poprawnie\n", filename);
    return true;
}



// Funkcje bazy obrazów
void add_image(struct image** base, int* size, struct image* new_img) {
    struct image* temp = realloc(*base, (*size + 1) * sizeof(struct image));
    if (temp == NULL) {
        printf("Blad realokacji bazy obrazow.\n");
        return;
    }
    *base = temp;
    (*base)[*size] = *new_img;
    (*size)++;
    printf("Dodano obraz do bazy.\n");
}

void remove_image(struct image** base, int* size, int index) {
    if (index < 0 || index >= *size) {
        printf("Nieprawidlowy indeks obrazu.\n");
        return;
    }

    free_image(&(*base)[index]);
    (*base)[index] = (*base)[*size - 1];
    struct image* temp = realloc(*base, (*size - 1) * sizeof(struct image));
    if (temp == NULL && (*size - 1) > 0) {
        printf("Blad realokacji podczas usuwania obrazu.\n");
        return;
    }
    *base = temp;
    (*size)--;
    printf("Usunieto obraz z bazy.\n");
}

void list_images(struct image* base, int size) {
    if (size == 0) {
        printf("Baza obrazow jest pusta.\n");
        return;
    }
    printf("Lista obrazow w bazie:\n");
    for (int i = 0; i < size; i++) {
        printf("%d. %s (%dx%d)\n", i + 1, base[i].name, base[i].width, base[i].height);
    }
}

int select_image(int size) {
    if (size == 0) {
        printf("Brak obrazow w bazie.\n");
        return -1;
    }
    int choice;
    printf("Wybierz obraz (1-%d): ", size);
    scanf("%d", &choice);
    if (choice < 1 || choice > size) {
        printf("Nieprawidlowy wybor.\n");
        return -1;
    }
    return choice - 1;
}

void free_base(struct image** base, int size) {
    for (int i = 0; i < size; i++) {
        free_image(&(*base)[i]);
    }
    free(*base);
    *base = NULL;
}

void generate_histogram(struct image* img, const char* filename_base) {
    if (!validate_image(img)) {
        printf("Obraz nie jest poprawnie zaladowany. Histogram nie może zostac wygenerowany.\n");
        return;
    }

    // Przygotowanie nazwy pliku z rozszerzeniem .csv
    char filename[NAME_LENGTH + 4]; // +4 dla ".csv" i końcowego '\0'
    snprintf(filename, sizeof(filename), "%s.csv", filename_base);

    // Alokacja pamięci dla histogramu
    int* histogram = (int*)calloc(img->max_gray + 1, sizeof(int));
    if (histogram == NULL) {
        printf("Blad alokacji pamieci dla histogramu.\n");
        return;
    }

    // Obliczanie histogramu
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            histogram[img->pixels[i][j]]++;
        }
    }

    // Zapis histogramu do pliku CSV
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Blad przy zapisie histogramu do pliku %s.\n", filename);
        free(histogram);
        return;
    }

    fprintf(file, "Intensywnosc;Ilosc\n"); 
    for (int i = 0; i <= img->max_gray; i++) {
        fprintf(file, "%d,%d\n", i, histogram[i]);
    }

    fclose(file);
    free(histogram);
    printf("Histogram zapisano w pliku %s\n", filename);
}


void negative_image(struct image* img) {
    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            img->pixels[i][j] = img->max_gray - img->pixels[i][j];
        }
    }
    printf("Negatyw obrazu zastosowany.\n");
}

void rotate_image_90(struct image* img) {
    struct image temp;
    temp.width = img->height;
    temp.height = img->width;
    temp.max_gray = img->max_gray;
    allocate_image(&temp);

    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            temp.pixels[j][img->height - 1 - i] = img->pixels[i][j];
        }
    }

    free_image(img);
    *img = temp;
    printf("Obraz obrocono o 90 stopni.\n");
}

// Funkcja zapisu obrazu PGM
void save_image(const char* filename_base, struct image* img) {
    char filename[NAME_LENGTH + 4]; // +4 dla ".pgm" i końcowego '\0'
    snprintf(filename, sizeof(filename), "%s.pgm", filename_base);

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Blad przy otwieraniu pliku %s do zapisu!\n", filename);
        return;
    }

    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", img->width, img->height);
    fprintf(file, "%d\n", img->max_gray);

    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            fprintf(file, "%d ", img->pixels[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Obraz zapisano w pliku %s\n", filename);
}



// Dodawanie szumu "pieprz i sol"
void add_salt_and_pepper_noise(struct image* img, double noise_level) {
    if (!validate_image(img)) {
        printf("Obraz nie jest gotowy do dodania szumu.\n");
        return;
    }

    if (noise_level < 0 || noise_level > 1) {
        printf("Nieprawidlowy poziom szumu. Podaj wartosc z zakresu [0, 1].\n");
        return;
    }

    int num_pixels = img->width * img->height;
    int num_noisy = (int)(0.1 * num_pixels * noise_level); // 0.1 dla zmniejszenia wielkosci szumu

    srand((unsigned int)time(NULL));

    for (int k = 0; k < num_noisy; k++) {
        int i = rand() % img->height; // Losowy wiersz
        int j = rand() % img->width;  // Losowa kolumna

        if (i >= 0 && i < img->height && j >= 0 && j < img->width) {
            img->pixels[i][j] = (rand() % 2) ? img->max_gray : 0;
        }
    }
    printf("Dodano szum pieprz i sol na poziomie %.2f.\n", noise_level);
}

// Filtr medianowy
void gauss_filter(struct image* img) {
    float matrix[3][3] =
    {
    {1.0 / 16, 2.0 / 16, 1.0 / 16},
    {2.0 / 16, 4.0 / 16, 2.0 / 16},
    {1.0 / 16, 2.0 / 16, 1.0 / 16}
    };

    struct image temp;
    copy_image(&temp, img);

    for (int i = 1; i < img->height - 1; i++) {
        for (int j = 1; j < img->width - 1; j++) {
            float sum = 0.0;
            for (int mi = -1; mi <= 1; mi++) {
                for (int mj = -1; mj <= 1; mj++) {
                    sum += img->pixels[i + mi][j + mj] * matrix[mi + 1][mj + 1];
                }
            }
            temp.pixels[i][j] = (int)round(sum);
        }
    }
    free_image(img);
    *img = temp;

}

void copy_image(struct image* dest, struct image* src) {
    dest->width = src->width;
    dest->height = src->height;
    dest->max_gray = src->max_gray;
    allocate_image(dest);

    for (int i = 0; i < src->height; i++) {
        for (int j = 0; j < src->width; j++) {
            dest->pixels[i][j] = src->pixels[i][j];
        }
    }
}

// Menu użytkownika
void menu() {
    struct image* base = NULL;
    int size = 0;
    int selected_index = -1;

    int choice;
    do {
        printf("\n*** Baza Obrazow ***\n");
        printf("1. Dodaj obraz\n");
        printf("2. Usun obraz\n");
        printf("3. Wyswietl liste obrazow\n");
        printf("4. Wybierz obraz do przetwarzania\n");
        printf("5. Przetwarzaj obraz\n");
        printf("6. Zakoncz program\n");
        printf("Wybierz opcje: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1: {
            struct image img = { 0 };
            read_image(&img);
            add_image(&base, &size, &img);
            break;
        }
        case 2: {
            list_images(base, size);
            printf("Podaj indeks obrazu do usuniecia: ");
            int index;
            scanf("%d", &index);
            remove_image(&base, &size, index - 1);
            break;
        }
        case 3:
            list_images(base, size);
            break;
        case 4: {
            list_images(base, size);
            selected_index = select_image(size);
            break;
        }
        case 5: {
            if (selected_index == -1 || selected_index >= size) {
                printf("Najpierw wybierz obraz do przetwarzania!\n");
                break;
            }
            printf("Wybrano obraz: %s\n", base[selected_index].name);
            printf("\n*** Przetwarzanie wybranego obrazu: %s ***\n", base[selected_index].name);
            int sub_choice;
            do {
                printf("1. Obrot o 90 stopni\n");
                printf("2. Negatyw obrazu\n");
                printf("3. Histogram do pliku CSV\n");
                printf("4. Szum pieprz i sol\n");
                printf("5. Filtr Gaussa (rozmycie)\n");
                printf("6. Zapisz obraz\n");
                printf("7. Powrot do glownego menu\n");
                printf("Wybierz opcje: ");
                scanf("%d", &sub_choice);

                switch (sub_choice) {
                case 1:
                    rotate_image_90(&base[selected_index]);
                    printf("Obraz obrocono o 90 stopni.\n");
                    break;
                case 2:
                    negative_image(&base[selected_index]);
                    printf("Negatyw obrazu zastosowany.\n");
                    break;
                case 3: {
                    char hist_filename[NAME_LENGTH];
                    printf("Podaj nazwe pliku do zapisu histogramu (bez rozszerzenia): ");
                    scanf("%s", hist_filename);
                    generate_histogram(&base[selected_index], hist_filename);
                    break;
                }
                case 4: {
                    double noise_level;
                    printf("Podaj poziom szumu (0-1): ");
                    scanf("%lf", &noise_level);
                    add_salt_and_pepper_noise(&base[selected_index], noise_level);
                    printf("Szum pieprz i sol dodany.\n");
                    break;
                }
                case 5: {
                    int intensity;
                    printf("Podaj nasilenie rozmycia (od 1 do 10): ");
                    scanf("%d", &intensity);
                    for (int i = 0; i < intensity; i++) {
                        gauss_filter(&base[selected_index]);
                    }
                    printf("Filtr Gaussa zastosowany.\n");
                    break;
                }
                case 6: {
                    char save_filename[NAME_LENGTH];
                    printf("Podaj nazwe pliku do zapisu (bez rozszerzenia): ");
                    scanf("%s", save_filename);
                    save_image(save_filename, &base[selected_index]);
                    break;
                }
                case 7:
                    printf("Powrot do glownego menu.\n");
                    break;
                default:
                    printf("Nieprawidlowa opcja. Sprobuj ponownie.\n");
                }
            } while (sub_choice != 7);

            break;
        }
        case 6:
            free_base(&base, size);
            printf("Zakonczono program.\n");
            break;
        default:
            printf("Nieprawidlowa opcja. Sprobuj ponownie.\n");
        }
    } while (choice != 6);
}

int main() {
    menu();
    return 0;
}
