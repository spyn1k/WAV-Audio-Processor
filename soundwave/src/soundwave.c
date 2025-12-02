#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



    //Συναρτηση που διαβάζει 1 byte απτο stdin
    static int read_byte(void)
    {
        int c = getchar();
        if (c == EOF) return -1; //value of EOF is -1

        return c & 0xFF; /* Το getchar μπορεί να επιστρεψει αρνητικες τιμές για bytes >127
                            και με το & 0xFF κρατάμε μόνο τα χαμηλά 8 bits ώστε να γίνουν 0–255 */
    }

    //Διαβάζει n bytes από το stdin και τα αποθηκεύει στο buffer
    static int read_n(unsigned char *buf, int n)
    {
        for (int i=0; i < n; i++) //βάζω κάθε byte στο buf[i]
        {
            int b= read_byte(); //παιρνουμε 1 byte , επισης int ωστε να ελεγχουμε EOF

            if (b < 0) //δεν υπαρχουν αρκετά bytes για να καλυψουν τα n που ζητήσαμε 
            return -1;

            buf[i] = (unsigned char)b; //Αποθηκευομε byte στον buffer Το κάνουμε cast σε unsigned char για να μην βγει αρνητικος)
        }

        return 0;
    }

    //Διαβάζει 2 bytes και τα μετατρεπει σε little endian (unsigned 16bit αριθμό)
    static int read_u16(unsigned int *out)
    {
        unsigned char b[2]; //Δημιουργούμε buffer για τα 2 bytes (b[0] = low byte, b[1] = high byte). 

        if (read_n(b , 2) < 0)  
        return -1;  // Αν δεν μπορέσαμε να διαβάσουμε 2 bytes

        *out = b[0] | (b[1] << 8); //Συνδυάζουμε τα δύο bytes σε έναν 16-bit αριθμό.
        return 0;
    }

    //Διαβάζει 4 bytes και τα μετατρεπει σε little endian (unsigned 32bit αριθμό)
    static int read_u32(unsigned int *out) 
    {
        unsigned char b[4]; //Δημιουργούμε buffer για 4 bytes για ένα 32-bit αριθμό.

        if (read_n(b , 4 ) < 0)
        return -1;

        *out = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);        /* Ενώνουμε τα 4 bytes με shift.
                                                                        Το κάθε byte μετακινείται στη σωστή του θέση:
                                                                        b[0] = bits 0–7
                                                                        b[1] = bits 8–15
                                                                        b[2] = bits 16–23
                                                                        b[3] = bits 24–31 */
        return 0;
    }



/*ΣΚΕΛΕΤΟΣ*/
int main(int argc, char **argv)  //   argc = αριθμός ορισμάτων από τη γραμμή εντολών 
                                //    argv = πίνακας συμβολοσειρών που περιέχει τα ορίσματα 
{
    if( argc < 2) //Ελεγχος αν δωθει τουλαχιστον ενα ορισμα
    {
        fprintf(stderr, "Usage %s <command>\n", argv[0]); //Μήνυμα χρησης
        return 1;
    }

    if(strcmp(argv[1], "info") == 0)  //αν δωθει η εντολη info 
    {   
            unsigned char riff[4]; //4 bytes για RIFF

                if(read_n(riff, 4) < 0 || strncmp((char*)riff, "RIFF", 4) != 0 )   //Διαβάζουμε 4 bytes και ελέγχουμε αν είναι RIFF
                {
                 fprintf(stderr, "Error! \"RIFF\" not found\n"); //Μηνυμα λάθους αν δε βρεθει
                 return 1;
                }

            unsigned int file_size; //Μεγεθος του αρχείου 

                if(read_u32(&file_size) < 0)
                {
                    fprintf(stderr, "Error! truncated file\n"); //Αν δεν υπαρχουν bytes -> ERROR 
                    return 1;
                }

                printf("size of file: %u\n" , file_size); //Μεγεθος αρχειου 

            unsigned char wave[4]; //4 bytes για WAVE 

            if(read_n(wave,4) < 0 || strncmp((char*)wave, "WAVE", 4) != 0)
            {
                fprintf(stderr, "Error! \"WAVE\" not found\n");
                return 1;
            }

        return 0;

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
        fprintf(stderr, "Error! Not a valid command!\n");
        return 1;
    }
    return 0;
}

