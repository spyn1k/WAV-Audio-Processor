                                                              SOUNDWAVE.C
*The soundwave program is a command line tool for basic editing of PCM WAV files.*

*Supports displaying metadata, changing sample rate, and extracting channels*

•**info**  _Shows the info of the WAV header_

•**rate <factor>** _Multiplies the sample rate by a real factor_

•**channel <left|right>** ‎ _Extracts the left or right channel from a stereo file and produces a mono file_

                                                              COMPILE WITH
gcc -Ofast -Wall -Wextra -Werror -pedantic -o soundwave soundwave.c -lm

                                                              COMMAND GUIDE
Show info
-
./soundwave info < input.wav

Change sample rate 
-
./soundwave rate 0.5 < input.wav > out.wav

./soundwave rate 2.0 < input.wav > out.wav

Extract channel
-
./soundwave channel left < stereo.wav > left.wav

./soundwave channel right < stereo.wav > right.wav

                                                            WAV Validity Checks
***The program checks:***

<sub>RIFF and WAVE tags<sub>

<sub>fmt chunk (μέγεθος 16)<sub>

<sub>audio_format == 1<sub>

<sub>mono or stereo<sub>

<sub>bits per sample (8 or 16)<sub>

<sub>block alignment & bytes/sec<sub>

<sub>existence "data" chunk<sub>

**In case of an error, it displays a message on stderr.**

                                                                 TEST FILE
**PATH ----->** soundwave/test/

                                                                  SOURCES
***1.*** _Wikipedia – WAVE PCM Format_ https://en.wikipedia.org/wiki/WAV#RIFF_WAVE_chunks

Block_Align = channels × BitsPerSample / 8

Byte_Rate = sample_rate × block_align

Subchunk1Size = 16 (PCM)

AudioFormat = 1
__________________________________________
***2.*** _RIFF Specification (Microsoft)_ https://learn.microsoft.com/en-us/windows/win32/xaudio2/resource-interchange-file-format--riff-

RIFF Header order

file_size = 36 + data_size
__________________________________________
***3.*** _Microsoft – WAVEFORMATEX structure_ https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatex

FormatTag = 1 , PCM

channels → mono/stereo = 1/2

nSamplesPerSec → sample_rate

BitsPerSample = 8, 16, …
