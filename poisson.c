#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

char nazwa_dane[100];
char nazwa_wykres[100];
int MIN=0;
int MAX=-1;
int lambda=-1;
int tryb=0;
FILE *plik=NULL;

double poisson(int k);
void zapisz_plik(double *tablica, int min, int max);
void odczytaj_plik();
double *poisson_tablica();

int main(int argc, char *argv[])
{
    // obsługa argumentów programu
    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            lambda = atoi(argv[i + 1]);
            if (lambda < 0){
                printf("lambda musi być dodatnia\n");
                return -1;
            }
            i += 2;
        }
        else if (strcmp(argv[i], "-min") == 0)
        {
            MIN = atoi(argv[i + 1]);
            if (MIN < 0){
                printf("min musi być nieujemne\n");
                return -1;
            }
            i += 2;
        }
        else if (strcmp(argv[i], "-max") == 0)
        {
            MAX = atoi(argv[i + 1]);
            if (MAX < 0){
                printf("max musi być nieujemne\n");
                return -1;
            }
            i += 2;
        }
        else if (strcmp(argv[i], "-tryb") == 0)
        {
            tryb = atoi(argv[i + 1]);
            i+=2;
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
            strcpy(nazwa_dane, argv[i + 1]);
            i += 2;
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            strcpy(nazwa_wykres, argv[i + 1]);
            i += 2;
        }
        else i++;
    }

    // 2 szansa na podanie lambdy
    if (lambda == -1 && tryb>=0)
    {
        printf("lambda: ");
        scanf("%d", &lambda);
        if (lambda < 1){
            printf("lambda musi być większa od zera\n");
            return -1;
        }
    }

    // domyślna wartość MAX
    if (MAX == -1)
    {
        MAX = lambda * 2 + 10;
    } 
    MAX+=1;

    if (MAX < MIN){
        printf("max nie może być mniejsze od min\n");
        return -1;
    }
    // domyślne nazwy plików
    if (strlen(nazwa_dane) == 0)
    {
        strcpy(nazwa_dane, "poisson");
    }

    if (strlen(nazwa_wykres) == 0)
    {
        strcpy(nazwa_wykres, "poisson");
    }

    // obliczenia i zapis do pliku
    if (tryb>=0){
    plik=fopen(nazwa_dane,"w");
    while (MAX > MIN)
    {
        int min = MIN;                          // zapamiętuje poprzednią wartość min dzięki czemu zapisz_plik wie co jest w tablicy
        double *tablica = poisson_tablica();    // poisson_tablica zmienia MIN na najmniejsze k dla którego nie obliczyła poisson(k)
        zapisz_plik(tablica, min, MIN);
    }
    fclose(plik);
    }

    // odczyt z pliku i wykres
    if (tryb<=0){
        odczytaj_plik();
    }
    return 0;
}

double poisson(int k)
{
    return exp(-lambda + k * log(lambda) - lgamma(k + 1));
}

void odczytaj_plik() //i stwórz wykres przy użyciu gnuplot 
{

    FILE *gnuplot = popen("gnuplot -persist", "w");
    if (gnuplot == NULL)
    {
        perror("Błąd przy uruchamianiu gnuplot");
        return;
    }
    fprintf(gnuplot, "set terminal png\n");                 
    fprintf(gnuplot, "set output '%s.png'\n",nazwa_wykres); 
    fprintf(gnuplot, "set xlabel 'k'\n");
    fprintf(gnuplot, "set ylabel 'P(k,%d)'\n",lambda);
    fprintf(gnuplot, "set style data histogram\n");            
    fprintf(gnuplot, "set style fill solid border -1\n");
    fprintf(gnuplot, "set auto x\n");               
    fprintf(gnuplot, "plot '%s' with boxes\n", nazwa_dane); 
    /// fprintf(gnuplot, "plot '%s' with lines, '%s' with points pointtype 7 pointsize 1\n", "poisson.txt", "poisson.txt"); // linia i punkty
    pclose(gnuplot);

    return;
}

double *poisson_tablica()
{
    double *tablica = (double *)malloc(sizeof(double));
    if (tablica == NULL) // jeśli nie udało się zaalokować pamięci na tablicę o rozmiarze 1 double zakończy program
    {
        printf("niewystarczająca pamięć, zamykanie programu.\n");
        return NULL;
    }

    int liczba_niepoliczonych = MAX - MIN;
    for (int i = 0; i <= liczba_niepoliczonych; i++)
    {
        tablica[i] = poisson(i + MIN);
        double *nowa_tablica = (double *)realloc(tablica, (i + 2) * sizeof(double)); // realokacja tablicy na rozmiar i+1
        if (nowa_tablica == NULL)                                                      
        {                                                                            // jeśli się nie udało
            MIN += i + 1;                                                            // zapisuje w MIN ile udało się obliczyć
            return tablica;
        }
        tablica = nowa_tablica;
    }
    MIN = MAX;
    return tablica;
}

void zapisz_plik(double *tablica, int min, int max)
{
    for (int x = 0; x < max - min; x++)
    {
        fprintf(plik, "%d %lf\n", x + min, *(tablica+x*sizeof(double))); //x+min to k dla którego obliczyliśmy poisson(k)
    }
    free(tablica);                                      // zwalnia pamięć którą zaalokowała funkcja poisson_tablica
    return;
}
