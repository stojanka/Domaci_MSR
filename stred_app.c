#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
FILE *stred;
char buffer[120];
char string1[8];
int choice;
int l;

while(1){
  printf("Meni: \n");
  printf("1: Pročitaj trenutno stanje stringa \n");
  printf("2: Upiši novi string \n");
  printf("3: Konkataniraj string na trenutni \n");
  printf("4: Izbriši čitav string \n");
  printf("5: Izbriši vodeće i prateće space karaktere \n");
  printf("6: Izbriši izraz iz stringa \n");
  printf("7: Izbriši poslednjih n karaktera iz stringa \n");

  printf("Izaberite opciju (1-7): ");
  scanf("%d", &choice);

switch(choice){
  case 1: {
    stred = fopen("/dev/stred", "r");
    l=fread(buffer, 1, 100, stred);
    buffer[l]=0;
    printf("%s\n", buffer);
    fclose(stred);
    break;
  }

  case 2: {
    stred=fopen("/dev/stred", "w");
    /*strcpy(string1,"string=");

    printf("Upisite novi string: ");
    fgets(buffer, 100, stdin);

    //scanf("%[^\n]%*c",buffer);
    //printf("BUffer je: %s \n", buffer);

    printf("spojeni string: %s\n",strcat(string1, buffer));
    fwrite(strcat(string1, buffer), 1, 100, stred);*/

    strcpy(buffer,"string=");
    printf("Upisite novi string: ");
    fgets(buffer+7, 100, stdin);
    fwrite(buffer, 1, 100, stred);
    fclose(stred);
    break;
  }

  case 3: {
    stred=fopen("/dev/stred", "w");
    strcpy(buffer,"append=");
    printf("Dodajte novi string: ");
    fgets(buffer+7, 100, stdin);

    //scanf("%s",buffer+7);
    fwrite(buffer, 1, 100, stred);
    fclose(stred);
    break;
  }

  case 4: {
    stred=fopen("/dev/stred", "w");
    strcpy(buffer,"clear");
    fwrite(buffer, 1, 100, stred);
    fclose(stred);
    break;
  }

  case 5: {
    stred=fopen("/dev/stred", "w");
    strcpy(buffer, "shrink");
    fwrite(buffer, 1, 100, stred);
    fclose(stred);
    break;
  }

  case 6: {
    stred=fopen("/dev/stred", "w");
    strcpy(buffer,"remove=");
    printf("Napisite koju rec zelite da izbrisete: ");
    fgets(buffer+7, 100, stdin);
    //scanf("%s",buffer+7);
    fwrite(buffer, 1, 100, stred);
    fclose(stred);
    break;
  }

  case 7: {
    stred=fopen("/dev/stred", "w");
    strcpy(buffer,"truncate=");
    printf("Koliko karaktera zalite da izbrisete n= ");
    fgets(buffer+9, 100, stdin);
    //scanf("%[^\n]%*c",buffer+9);
    fwrite(buffer, 1, 100, stred);
    fclose(stred);
    break;
  }

  default:
  printf("Izabrali ste broj koji ne postoji u meniju :(. \n");

}


}


  return 0;
}
