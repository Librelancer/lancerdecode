#include <lancerdecode.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	char chunkID[4];
	uint32_t chunkSize;
	char format[4];
} riff_header_t;

typedef struct {
	char subChunkID[4];
	uint32_t subChunkSize;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
} wave_format_t;

typedef struct {
  char subChunkID[4];
  uint32_t subChunk2Size;
} wave_data_t;

#define BUFFER_SIZE 32768

int main(int argc, char **argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: %s inputfile output.wav\n", argv[0]);
        return 0;
    }
    ld_stream_t input = ld_stream_fopen(argv[1]);
    if(!input) {
        fprintf(stderr, "unable to open file %s\n", argv[1]);
        return 2;
    }
    
    ld_pcmstream_t audio = ld_pcmstream_open(input);
    if(!audio) {
        fprintf(stderr, "unable to decode %s\n", argv[1]);
        return 1;
    }
    
    FILE *output = fopen(argv[2], "wb");
    if(!output) {
        fprintf(stderr, "unable to open file %s for writing\n", argv[2]);
        return 2;
    }
    
    printf("frequency: %d\n", audio->frequency);
    const char* formats[] = {
        "", "mono8", "mono16", "stereo8", "stereo16"
    };
    printf("format: %s\n", formats[audio->format]);
    
    riff_header_t riff;
    memcpy(riff.chunkID, "RIFF", 4);
    memcpy(riff.format, "WAVE", 4);
    fwrite(&riff, sizeof(riff_header_t), 1, output);
    
    wave_format_t wav;
    memcpy(wav.subChunkID, "fmt ", 4);
    wav.subChunkSize = 16;
    wav.audioFormat = 0x1;
    wav.numChannels = (audio->format == LDFORMAT_MONO8 || audio->format == LDFORMAT_MONO16) ? 1 : 2;
    wav.sampleRate = audio->frequency;
    wav.bitsPerSample = (audio->format == LDFORMAT_MONO8 || audio->format == LDFORMAT_STEREO8) ? 8 : 16;
    wav.blockAlign = (wav.numChannels * wav.bitsPerSample) / 8;
    wav.byteRate = wav.blockAlign * wav.sampleRate;
    fwrite(&wav, sizeof(wave_format_t), 1, output);
    
    fwrite("data", 4, 1, output);
    long int data_offset = ftell(output);
    int32_t length = 0;
    fwrite(&length, 4, 1, output);
    
    int readlen = 0;
    unsigned char buffer[BUFFER_SIZE];
    while((readlen = audio->stream->read(buffer, BUFFER_SIZE, audio->stream))) {
        length += readlen;
        fwrite(buffer, readlen, 1, output);
    }
    fseek(output, data_offset, SEEK_SET);
    fwrite(&length, 4, 1, output);
    int32_t riff_length = length + 24;
    fseek(output, 4, SEEK_SET);
    fwrite(&riff_length, 4, 1, output);
    
    printf("decoded %d bytes\n", length);
    return 0;
}
