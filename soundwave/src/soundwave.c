#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

unsigned int file_size;
unsigned int fmt_size;
unsigned int audio_format;
unsigned int channels;
unsigned int sample_rate;
unsigned int bytes_per_sec;
unsigned int block_align;
unsigned int bits_per_sample;
unsigned int data_size;


unsigned char tag4[4];
unsigned char tag2[2];
unsigned char left_buf[4];
unsigned char right_buf[4];


/*---READ FUNCTIONS---*/

//Συναρτηση που διαβάζει 1 byte απτο stdin
static int read_byte(void)
    {
        int c = getchar();

        if (c == EOF) 
        {
            return -1; //value of EOF is -1
        } 

        return c & 0xFF; /* με το & 0xFF κρατάμε μόνο τα χαμηλά 8 bits ώστε να γίνουν 0–255 */
    }

    //Διαβάζει n bytes από το stdin και τα αποθηκεύει στο buffer
    static int read_n(unsigned char *buf, int n)
    {
        for (int i=0; i < n; i++) //βάζω κάθε byte στο buf[i]
        {
            int b= read_byte(); //παιρνουμε 1 byte , επισης int ωστε να ελεγχουμε EOF

            if (b < 0) //Aν δεν υπαρχουν αρκετά bytes για να καλυψουν τα n που ζητήσαμε 
            {
                return -1;
            }
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

static int read_header(void)
{   
    //Αν δεν βρω RIFF σημαίνει ότι δεν είναι WAV αρχείο επομενως τελειωνει το προγραμμα
    if (read_n(tag4, 4) < 0 || strncmp((char*)tag4, "RIFF", 4) != 0) 
    {
        fprintf(stderr,"Error! \"RIFF\" not found\n");
        return -1;
    }
    //Διαβάζω file size για να ξέρω το συνολικο περιεχόμενο μετα το header
    if (read_u32(&file_size) < 0)
    {
        fprintf(stderr, "Error! truncated file\n");
        return -1;
    }
    //Αν λείπει το WAVE τότε το αρχείο δεν είναι έγκυρο
    if (read_n(tag4, 4) < 0 || strncmp((char*)tag4,"WAVE", 4 ) != 0)
    {
        fprintf(stderr, "Error! \"WAVE\" not found\n");
        return -1;
    }
    //Να υπάρχει παντα fmt
     if (read_n(tag4, 4) < 0 || strncmp((char*)tag4, "fmt ", 4) != 0)
    {
        fprintf(stderr, "Error! \"fmt \" not found\n");
        return -1;
    }
    //Το fmt chunk πρέπει να έχει μέγεθος 16 σε απλό PCM if not then corrupted file
    if (read_u32(&fmt_size) < 0) return -1;
    if (fmt_size != 16)
    {
        fprintf(stderr, "Error! the size of format chunk should be 16\n");
        return -1;
    }
    //audio_format = 1 σημαίνει uncompressed PCM
    if(read_u16(&audio_format) < 0 ) return -1;
    if(audio_format != 1)
    {
        fprintf(stderr, "Error! WAVE type format should be 1\n");
        return -1;
    }
    //Mono και Stereo ειναι οι μονες επιλογές 
    if(read_u16(&channels) < 0) return -1;
    if(!(channels == 1) || (channels == 2))
    {
        fprintf(stderr, "mono/stereo should be on 1 or 2\n");
        return 1;
    }


    /*must για να ειναι λειτουργικος ο τροπος του rate/channel*/
    if (read_u32(&sample_rate) < 0) return -1;      
    if (read_u32(&bytes_per_sec) < 0) return -1;
    if (read_u16(&block_align) < 0) return -1;
    if (read_u16(&bits_per_sample) < 0) return -1;


    //Αν δεν ταιριαζει ο παρακατω τύπος τοτε μαλλον το αρχειο ειναι κατεστραμμενο
    if (bytes_per_sec != sample_rate * block_align)
    {
        fprintf(stderr,"Error! bytes/second should be sample rate x block alignment\n");
        return 1;
    }


    //Ποσα bytes είναι ένα δειγμα ολων των καναλιων
unsigned int expected = (bits_per_sample/8) * channels;
    if(block_align != expected)
    {
        fprintf(stderr,"Error! block alignment should be bits per sample / 8 x mono/stereo\n");
        return 1;
    }


    /*Αν δεν υπάρχει data chunk δεν υπαρχουν audio samples*/
    if (read_n(tag4, 4) < 0 || strncmp((char*)tag4, "data", 4) != 0)
    {
        fprintf(stderr, "Error! \"data\" not found\n");
        return -1;
    }

    //raw sample data size για rate και channel
     if (read_u32(&data_size) < 0)
    {
        fprintf(stderr, "Error! cannot read data size\n");
        return -1;
    }
}   


// Γράφει 2 bytes σε little endian
static void write_u16(unsigned int value)
{
    unsigned char b[2];
    b[0] = value & 0xFF;        // low byte
    b[1] = (value >> 8) & 0xFF; // high byte
    fwrite(b, 1, 2, stdout);
}

// Γράφει 4 bytes σε little endian
static void write_u32(unsigned int value)
{
    unsigned char b[4];
    b[0] = value & 0xFF;
    b[1] = (value >> 8) & 0xFF;
    b[2] = (value >> 16) & 0xFF;
    b[3] = (value >> 24) & 0xFF;
    fwrite(b, 1, 4, stdout);
}

static void write_header(void)
{
    printf("RIFF");                 //Γραφω το RIFF πρωτο για αναγνώριση του αρχείου
    write_u32(file_size);           //Το συνολικο μεγεθσο του αρχειου
    printf("WAVE");                 //Μπαινει μετα το RIFF παντα για να δηλωσει το format
    printf("fmt ");                 //Απο τι αποτελέιται ο ηχος
    write_u32(fmt_size);            //Πρέπει να είναι 16 για PCM
    write_u16(audio_format);        /*raw uncompressed audio*/
    write_u16(channels);            //1 = mono , 2 = stereo
    write_u32(sample_rate);         //Πόσα δειγματα ανα sec αν λαθος τοτε ειναι sped up ή slowed down
    write_u32(bytes_per_sec);       //sample_rate * block_align
    write_u16(block_align);         //ποσα bytes ειναι ενα audio frame , (channels * bits_per_sample/8)
    write_u16(bits_per_sample);     //8bit ή 16bit
    printf("data");                 //δειχνει πως κατω απο αυτο ξεκινανε τα raw samples
    write_u32(data_size);           //Ποσα bytes ηχου θα ακολουθησουν μετα το header
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