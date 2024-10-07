#include <stdio.h>
#include <ctype.h>


int main() {
    char firstname[20];
    char secondname[20];
    char str[20]

    printf("Enter Firstname: ");
    scanf("%s", firstname);
    printf("Enter Secondname: ");
    scanf("%s", secondname);

    printf("Your full name is %s %s.\n", firstname, secondname);
    
    str = toupper(secondname);
    printf("%s", str);
    
    return 0;
}

