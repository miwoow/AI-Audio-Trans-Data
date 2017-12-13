#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wavpack/wavpack.h>

#pragma pack(1)
typedef struct
{
    unsigned short FormatTag, NumChannels;
    uint32_t SampleRate, BytesPerSecond;
    unsigned short BlockAlign, BitsPerSample;
    unsigned short cbSize;//, ValidBitsPerSample;
}MYWaveHeader;
#pragma pack()

const char *WAV_FILE="20171212.wav";

int print_mem(char *buf, int size)
{
    int i=0;
    for(i=0; i<size; i++) {
        printf("%c", buf[i]);
    }
    printf("\n");
    return 0;
}    

int print_mem_int(int16_t *buf, int size)
{
    int i=0;
    for(i=0; i< size; i++) {
        if (i%16 == 0) {
            printf("\n");
        }
        printf(" %d", buf[i]);
    }
    printf("\n");
    return 0;
}

int write_wav_header(FILE *fh, MYWaveHeader *wave_header, int32_t size)
{
    RiffChunkHeader new_riff_chunk_header;
    memcpy(new_riff_chunk_header.ckID, "RIFF", 4);
    new_riff_chunk_header.ckSize = size + 22;
    memcpy(new_riff_chunk_header.formType, "WAVE", 4);

    ChunkHeader new_fmt_chunk_header;
    memcpy(new_fmt_chunk_header.ckID, "fmt ", 4);
    new_fmt_chunk_header.ckSize = 18;

    ChunkHeader new_data_chunk_header;
    memcpy(new_data_chunk_header.ckID, "data", 4);
    new_data_chunk_header.ckSize = size;

    fwrite(&new_riff_chunk_header, 1, sizeof(RiffChunkHeader), fh);
    fwrite(&new_fmt_chunk_header, 1, sizeof(ChunkHeader), fh);
    fwrite(wave_header, 1, sizeof(MYWaveHeader), fh);
    fwrite(&new_data_chunk_header, 1, sizeof(ChunkHeader), fh);

    return 0;
}

int change_file_to_wav(char *file_name, MYWaveHeader *wave_header)
{
    char wav_file_name[128] = {0};

    snprintf(wav_file_name, 127, "%s.wav", file_name);
    FILE *pcm_fh = fopen(file_name, "rb");
    int32_t pcm_size = 0;
    fseek(pcm_fh, 0L, SEEK_END);
    pcm_size = ftell(pcm_fh);
    FILE *wav_fh = fopen(wav_file_name, "wb");
    write_wav_header(wav_fh, wave_header, pcm_size);
    char *buf = (char *)malloc(pcm_size);
    fseek(pcm_fh, 0L, SEEK_SET);
    fread(buf, 1, pcm_size, pcm_fh);
    fwrite(buf, 1, pcm_size, wav_fh);
    fclose(pcm_fh);
    fclose(wav_fh);
}

int parse_and_split_audio(FILE *nfh, int16_t *buf, size_t size, int *split_file_index, MYWaveHeader *wave_header)
{
    size_t i=0;
    int distance = 0;
    char n_file_name[128] = {0};

    for(i=0; i<size/2; i++) {
        if ( abs(buf[i]) < 2000 ) {
            distance++;
        } else {
            if (distance >= 0.2 * 16000) {
                // It's audio start or end
                if (nfh == NULL) {
                    snprintf(n_file_name, 127, "sample_%04d.pcm", *split_file_index);
                    (*split_file_index)++;
                    nfh = fopen(n_file_name, "wb");
                    fwrite((void *)&(buf[i-200]), 1, 2* 200, nfh);
                } else {
                    fwrite((void *)&(buf[i-distance]), 1, 2 * 200, nfh);
                    fclose(nfh);
                    change_file_to_wav(n_file_name, wave_header);
                    nfh = NULL;
                    snprintf(n_file_name, 127, "sample_%04d.pcm", *split_file_index);
                    (*split_file_index)++;
                    nfh = fopen(n_file_name, "wb");
                    fwrite((void *)&(buf[i-200]), 1, 2  * 200, nfh);
                }
                distance = 0;
            } else {
                if (!nfh) {
                    snprintf(n_file_name, 127, "sample_%04d.pcm", *split_file_index);
                    (*split_file_index)++;
                    nfh = fopen(n_file_name, "wb");
                }
                if (distance != 0) {
                    fwrite((void *)&(buf[i - distance]), 1, 2 * (distance + 1), nfh);
                } else {
                    fwrite((void *)&(buf[i]), 1, 2, nfh);
                }
                distance = 0;
            }
        }
    }

    if (nfh) {
        fclose(nfh);
        change_file_to_wav(n_file_name, wave_header);
        nfh = NULL;
    }
    return 0;
}

int output_wave_header(MYWaveHeader *wav_header)
{
    printf("FormatTag: %d\n", wav_header->FormatTag);
    printf("NumChannels: %d\n", wav_header->NumChannels);
    printf("SampleRate: %d\n", wav_header->SampleRate);
    printf("BytesPerSecond: %d\n", wav_header->BytesPerSecond);
    printf("BlockAlign: %d\n", wav_header->BlockAlign);
    printf("BitsPerSample: %d\n", wav_header->BitsPerSample);
    printf("cbSize: %d\n", wav_header->cbSize);
    return 0;
}




int main(int argc, char *argv[])
{
    FILE *fh = fopen(WAV_FILE, "rb");
    //FILE *fh = fopen("news.wav", "rb");
    FILE *nfh = NULL;
    int16_t data_for_one_sec[100 * 16000];
    size_t count = 0;
    int split_file_index = 0;

    RiffChunkHeader riff_chunk_header;
    fread(&riff_chunk_header,  1, sizeof(RiffChunkHeader), fh);

    print_mem(riff_chunk_header.ckID, 4);
    printf("%d\n", riff_chunk_header.ckSize);
    print_mem(riff_chunk_header.formType, 4);

    ChunkHeader chunk_header;
    fread(&chunk_header, 1, sizeof(ChunkHeader), fh);
    print_mem(chunk_header.ckID, 4);
    printf("%d\n", chunk_header.ckSize);

    MYWaveHeader wave_header;
    fread(&wave_header, 1, sizeof(MYWaveHeader), fh);
    output_wave_header(&wave_header);

    ChunkHeader data_header;
    fread(&data_header, 1, sizeof(ChunkHeader), fh);

    print_mem(data_header.ckID, 4);
    printf("%d\n", data_header.ckSize);

    while(!feof(fh)) {
        count = fread(&data_for_one_sec, 1, 100 * 16000 * 2, fh);
        parse_and_split_audio(nfh, data_for_one_sec, count, &split_file_index, &wave_header);
    }

//    FILE *nfh = fopen("news.wav", "wb");
//    write_wav_header(nfh, &wave_header, 100 * 16000 * 2);
//
//    count = fread(data_for_one_sec, 1, 100 * 16000 * 2, fh);
//    printf("%ld\n", count);
//    fwrite(data_for_one_sec, 1, count, nfh);
//    
//    fclose(nfh);

    fclose(fh);

    return 0;
}
