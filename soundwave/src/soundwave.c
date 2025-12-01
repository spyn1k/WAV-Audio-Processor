#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*ΣΚΕΛΕΤΟΣ*/
int main(int argc, char **argv) 
{
    if( argv < 2)
    {
        fprintf(stderr, "Usage %s command", argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "info") == 0) 
    {
        //υλοποιηση
    }
        else if (strcmp(argv[1], "rate") == 0) 
        {
            //υλοποιηση
        }
            else if(strcmp(argv[1], "channel") ==0)
            {
                //υλοποιηση
            }
    else
    {
        fprintf(stderr, "Unknown command\n");
        return 1;
    }
    return 0;
}

