#include <stdio.h>
#include <stdlib.h>
//call it with some parameters from the command line 

/*
 * Task: change the ./a.out 1 2 3 => 2 3 4
 * Hints: The ./a.out is the first elements in character array
*/

int main(int argc,char ** argv) 
// argc means the number of argument 
// char** means an array of character arrays = array of strings
{   
    int i;
    printf("Number of command line arguments are: %i\n", argc);
    for (i = 1; i < argc; i++){
        int newInt = atoi(argv[i]) + 1;
        // atoi() function convert string to interger
        printf("%i. argument is %d\n", i, newInt);
    }

    return 0;
}