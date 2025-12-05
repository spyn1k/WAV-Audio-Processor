#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/*--GLOBAL VARIABLES--*/
unsigned int file_size;
unsigned int fmt_size;
unsigned int audio_format;
unsigned int channels;
unsigned int sample_rate;
unsigned int bytes_per_sec;
unsigned int block_align;
unsigned int bits_per_sample;
unsigned int data_size;

//Buffers to store raw bytes before converting them because WAV is a binary format
unsigned char tag4[4];    
unsigned char tag2[2];
unsigned char left_buf[4];
unsigned char right_buf[4];

int header_error=0; //header error 
char header_error_msg[128]; //buffer for the header error message

/*---READ FUNCTIONS---*/

//Function that reads 1 byte from stdin
static int read_byte(void)
    {
        int c = getchar();

        if (c == EOF) 
        {
            return -1; //value of EOF is -1
        } 

        return c & 0xFF; /* με το & 0xFF κρατάμε μόνο τα χαμηλά 8 bits ώστε να γίνουν 0–255 */
    }

    //Reads n bytes from stdin and stores them in the buffer
    static int read_n(unsigned char *buf, int n)
    {
        for (int i=0; i < n; i++) //Stores every byte in buf[i]
        {
            int b= read_byte(); //gets 1 byte , also int so that we check for EOF

            if (b < 0) //In case there aren't enough bytes to cover the n bytes which was asked 
            {
                return -1;
            }
            buf[i] = (unsigned char)b; //Stores byte in the buffer, we change it to unsigned char so that it doesnt come out negative
        }

        return 0;
    }

    //Reads 2 bytes and converts them into the form of little endian (unsigned 16bit number)
    static int read_u16(unsigned int *out)
    {
        unsigned char b[2]; //Create a buffer for the 2 bytes (b[0] = low byte, b[1] = high byte). 

        if (read_n(b , 2) < 0)  
        return -1;  // In case we cant read 2 bytes

        *out = b[0] | (b[1] << 8); //Combine the two bytes into a 16-bit number.
        return 0;
    }

    //Reads 4 bytes and converts them to little endian (unsigned 32bit number)
    static int read_u32(unsigned int *out) 
    {
        unsigned char b[4]; //Create a buffer for 4 bytes for a 32-bit number.

        if (read_n(b , 4 ) < 0)
        return -1;

        *out = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);        /* Combine the 4 bytes by shifting.
                                                                        Every byte moves to the right spot:
                                                                        b[0] = bits 0–7
                                                                        b[1] = bits 8–15
                                                                        b[2] = bits 16–23
                                                                        b[3] = bits 24–31 */
        return 0;
    }

static int read_header(void) 
{   
    //If it can't find RIFF it means that it is not a WAV file, τherefore the program ends
    if (read_n(tag4, 4) < 0 || strncmp((char*)tag4, "RIFF", 4) != 0) 
    {
        if (!header_error) //if no error has been recorded beforehand
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! \"RIFF\" not found");
        }
    }

    //Reads file size so that i know the overall content after the header
    if (read_u32(&file_size) < 0)
    {
        if (!header_error) 
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! truncated file");
        }
    }

    //if WAVE is missing then the file is not valid
    if (read_n(tag4, 4) < 0 || strncmp((char*)tag4,"WAVE", 4 ) != 0)
    {
        if (!header_error) 
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! \"WAVE\" not found");
        }
    }
    //There should always be fmt
     if (read_n(tag4, 4) < 0 || strncmp((char*)tag4, "fmt ", 4) != 0)
    {
         if (!header_error) 
         {
            header_error = 1;
            strcpy(header_error_msg, "Error! \"fmt\" not found");
        }
    }

    //ΤThe fmt chunk shall have the size of 16 in a simple PCM, if not then the file is corrupted
    if (read_u32(&fmt_size) < 0) return 0;
    if (fmt_size != 16)
    {
        if (!header_error) 
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! the size of format chunk should be 16");
        }
    }

    //audio_format = 1 means uncompressed PCM
    if(read_u16(&audio_format) < 0 ) return 0;
    if(audio_format != 1)
    {
       if (!header_error) 
       {
            header_error = 1;
            strcpy(header_error_msg, "Error! WAVE type format should be 1");
        }
    }

    //Mono and Stereo are the only options
    if(read_u16(&channels) < 0) return 0;
    if(!(channels == 1 || channels == 2))
    {
        if (!header_error) 
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! mono/stereo should be 1 or 2");
        }
    }


    /*must exist for the rate/channel method to be functional*/
    read_u32(&sample_rate);     
    read_u32(&bytes_per_sec);
    read_u16(&block_align);
    read_u16(&bits_per_sample);

    if (bits_per_sample != 8 && bits_per_sample != 16)
    {
        if (!header_error) 
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! bits per sample should be 8 or 16");
        }
    }


    //If the type below does not match then the file is probably corrupted
    if (bytes_per_sec != sample_rate * block_align)
    {
       if (!header_error) 
       {
            header_error = 1;
            strcpy(header_error_msg, "Error! bytes/second should be sample rate x block alignment");
        }
    }


    //How many bytes is a sample of all channels
unsigned int expected = (bits_per_sample/8) * channels;
    if(block_align != expected)
    {
        if (!header_error) 
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! block alignment should be bits per sample / 8 x mono/stereo");
        }
    }


    /*If a data chunk does not exist then there are no audio samples*/
    if (read_n(tag4, 4) < 0 || strncmp((char*)tag4, "data", 4) != 0)
    {
        if (!header_error) 
        {   
            header_error = 1;
            strcpy(header_error_msg, "Error! \"data\" not found");
        }
    }

    //raw sample data size for rate and channel
     if (read_u32(&data_size) < 0)
    {
        if (!header_error) 
        {
            header_error = 1;
            strcpy(header_error_msg, "Error! cannot read data size");
        }
    }
    return 0;
}   


// Writes 2 bytes in little endian
static void write_u16(unsigned int value)
{
    unsigned char b[2];
    b[0] = value & 0xFF;        // low byte
    b[1] = (value >> 8) & 0xFF; // high byte
    fwrite(b, 1, 2, stdout);
}

// Writes 4 bytes in little endian
static void write_u32(unsigned int value)

{
    
    unsigned char b[4];                //split 32-bit integer into 4 little-endian bytes and write them to WAV output 
    b[0] = value & 0xFF;
    b[1] = (value >> 8) & 0xFF;
    b[2] = (value >> 16) & 0xFF;
    b[3] = (value >> 24) & 0xFF;
    fwrite(b, 1, 4, stdout);    
}
static void write_n(unsigned char *buf, int n)
{
    fwrite(buf, 1, n, stdout); output to WAV file exactly n bytes from buffer 
}
static void write_header(void)
{
    printf("RIFF");                 //ΓWrite RIFF first to mark the file as a WAV container
    write_u32(file_size);           //Total file size
    printf("WAVE");                 //Always comes after RIFF so that it declares the format
    printf("fmt ");                 //Format chunk that describes the audio format
    write_u32(fmt_size);            //should be 16 for PCM
    write_u16(audio_format);        /*raw uncompressed audio*/
    write_u16(channels);            //1 = mono , 2 = stereo
    write_u32(sample_rate);         //How many samples per sec, wrong values means sped up/slowed down audio
    write_u32(bytes_per_sec);       //sample_rate * block_align
    write_u16(block_align);         //Size of one audio frame(bytes) , (channels * bits_per_sample/8)
    write_u16(bits_per_sample);     //8bit or 16bit
    printf("data");                 //Indicates that raw audio samples follow after it
    write_u32(data_size);           //How many bytes of sound will follow after the header 
}


/*SKELETON*/
int main(int argc, char **argv)  //   argc = αριθμός ορισμάτων από τη γραμμή εντολών 
                                //    argv = πίνακας συμβολοσειρών που περιέχει τα ορίσματα 
{
    if( argc < 2) //Ελεγχος αν δωθει τουλαχιστον ενα ορισμα
    {
        fprintf(stderr, "Usage %s <command>\n", argv[0]); //Μήνυμα χρησης
        return -1;
    }

    if(strcmp(argv[1], "info") == 0)  //αν δωθει η εντολη info   
    {
        if (read_header() < 0) return -1;

        printf("size of file: %u\n", file_size);
        printf("size of format chunk: %u\n", fmt_size);
        printf("WAVE type format: %u\n", audio_format);
        printf("mono/stereo: %u\n", channels);
        printf("sample rate: %u\n", sample_rate);
        printf("bytes/sec: %u\n", bytes_per_sec);
        printf("block alignment: %u\n", block_align);
        printf("bits/sample: %u\n", bits_per_sample);
        printf("size of data chunk: %u\n", data_size);

         if (header_error) 
        {
            printf("%s\n", header_error_msg); //if the header had an error, show it now (after printing all info fields)
            return -1;
        }

        return 0;
    }

        else if (strcmp(argv[1], "rate") == 0)  //adjust sample rate by a factor
        {
            //πρεπει να περαστει ενας παραγοντας
            if(argc < 3)    //check if the factor is missing
            {
                fprintf(stderr, "Missing factor\n");
                    return -1;
            }
            double factor = atof(argv[2]); //για να υποστηρίζει δεκαδικους
        

        if(read_header() < 0) return -1;

            sample_rate = (unsigned int)(sample_rate*factor); /This makes the sound slower or faster

            bytes_per_sec = sample_rate * block_align; //to bytes/sec ειναι αναλογο με το sample rate
            // After modifying the audio (rate/channel), we must update the header fields accordingly.
            file_size = 36 + data_size; //36 byte header + data chunk = new header

        write_header(); 


            for (unsigned int i = 0; i < data_size; i++)
            {
                int c = read_byte();
                if (c < 0) return -1;

                putchar(c);
            }
        }
            else if(strcmp(argv[1], "channel") ==0)
            {
                if (argc < 3 ) //check if the factor is missing
                {
                    fprintf(stderr,"Missing channel option\n");
                    return -1;
                }

                //Checks if user wants left or right
                int want_left = strcmp(argv[2], "left") == 0;
                int want_right = strcmp(argv[2], "right") == 0;


                if(!want_left && !want_right)
                {
                    fprintf(stderr,"Bad channel option\n");
                    return -1;
                }

                if(read_header() < 0)
                {
                    return -1;
                }

                //If file already is a mono then dont split
                if (channels == 1)
                {
                    write_header();
                    for (unsigned int i = 0; i < data_size; i++)
                    {
                        int c = read_byte();
                        if (c < 0) return -1;
                        putchar(c);
                    }
                    return 0;
                }

            /* A frame = one complete set of samples for all channels (left+right in stereo)*/
            unsigned int bps = bits_per_sample / 8;         //For stereo calculate how many bytes every sample has
            unsigned int frames = data_size / (2 * bps);    //For frames how many left+right pairs there are in total 
            

            //Here i create the new Mono WAV
            channels = 1;                                   //mono
            block_align = bps;                              //mono frame = 1 sample*bps
            bytes_per_sec = sample_rate * block_align;      
            data_size = frames * bps;                       //new datasize for one channel only
            file_size = 36 + data_size;                     //36 bytes header + chunk || After modifying the audio (rate/channel), we must update the header fields accordingly.

            //I write the new mono header to the output
            write_header();


                //Here the actual split happens
                //For every frame i read first left to right
                for (unsigned int i = 0; i < frames; i++)
                {
                    read_n(left_buf, bps);
                    read_n(right_buf, bps);

                    if (want_left)
                    {
                        write_n(left_buf, bps);
                    }
                    else
                    {
                        write_n(right_buf, bps);
                    }
                }
            }   
    else
    {
        fprintf(stderr, "Error! Not a valid command!\n");
        return -1;
    }
    return 0;
}
