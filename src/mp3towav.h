#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// ref:https://github.com/lieff/minimp3/blob/master/minimp3.h
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include <sys/stat.h>

//写wav文件
void wavWrite_int16(char* filename, int16_t* buffer, int sampleRate, uint32_t totalSampleCount, int channels) {

    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("文件打开失败.\n");
        return;
    }
    //修正写入的buffer长度
    totalSampleCount *= sizeof(int16_t)*channels;
    int nbit = 16;
    int FORMAT_PCM = 1;
    int nbyte = nbit / 8;
    char text[4] = { 'R', 'I', 'F', 'F' };
    uint32_t long_number = 36 + totalSampleCount;
    fwrite(text, 1, 4, fp);
    fwrite(&long_number, 4, 1, fp);
    text[0] = 'W';
    text[1] = 'A';
    text[2] = 'V';
    text[3] = 'E';
    fwrite(text, 1, 4, fp);
    text[0] = 'f';
    text[1] = 'm';
    text[2] = 't';
    text[3] = ' ';
    fwrite(text, 1, 4, fp);

    long_number = 16;
    fwrite(&long_number, 4, 1, fp);
    int16_t short_number = FORMAT_PCM;//默认音频格式
    fwrite(&short_number, 2, 1, fp);
    short_number = channels; // 音频通道数
    fwrite(&short_number, 2, 1, fp);
    long_number = sampleRate; // 采样率
    fwrite(&long_number, 4, 1, fp);
    long_number = sampleRate * nbyte; // 比特率
    fwrite(&long_number, 4, 1, fp);
    short_number = nbyte; // 块对齐
    fwrite(&short_number, 2, 1, fp);
    short_number = nbit; // 采样精度
    fwrite(&short_number, 2, 1, fp);
    char data[4] = { 'd', 'a', 't', 'a' };
    fwrite(data, 1, 4, fp);
    long_number = totalSampleCount;
    fwrite(&long_number, 4, 1, fp);
    fwrite(buffer, totalSampleCount, 1, fp);
    fclose(fp);
}
//读取文件buffer
char *getFileBuffer(const char *fname, int *size)
{
    FILE * fd = fopen(fname, "rb");
    if (fd == 0)
        return 0;
    struct stat st;
    char *file_buf = 0;
    if (fstat(fileno(fd), &st) < 0)
        goto doexit;
    file_buf = (char *)malloc(st.st_size + 1);
    if (file_buf != NULL)
    {
        if (fread(file_buf, st.st_size, 1, fd) < 1)
        {
            fclose(fd);
            return 0;
        }
        file_buf[st.st_size] = 0;
    }

    if (size)
        *size = st.st_size;
    doexit:
    fclose(fd);
    return file_buf;
}
//mp3解码
int16_t* DecodeMp3ToBuffer(char* filename, uint32_t *sampleRate, uint32_t *totalSampleCount, unsigned int *channels)
{
    int music_size = 0;
    int alloc_samples = 1024 * 1024, num_samples = 0;
    int16_t *music_buf = (int16_t *)malloc(alloc_samples * 2 * 2);
    unsigned char *file_buf = (unsigned char *)getFileBuffer(filename, &music_size);
    if (file_buf != NULL)
    {
        unsigned char *buf = file_buf;
        mp3dec_frame_info_t info;
        mp3dec_t dec;

        mp3dec_init(&dec);
        for (;;)
        {
            int16_t frame_buf[2 * 1152];
            int samples = mp3dec_decode_frame(&dec, buf, music_size, frame_buf, &info);
            if (alloc_samples < (num_samples + samples))
            {
                alloc_samples *= 2;
                int16_t* tmp = (int16_t *)realloc(music_buf, alloc_samples * 2 * info.channels);
                if (tmp)
                    music_buf = tmp;
            }
            if (music_buf)
                memcpy(music_buf + num_samples*info.channels, frame_buf, samples*info.channels * 2);
            num_samples += samples;
            if (info.frame_bytes <= 0 || music_size <= (info.frame_bytes + 4))
                break;
            buf += info.frame_bytes;
            music_size -= info.frame_bytes;
        }
        if (alloc_samples > num_samples)
        {
            int16_t* tmp = (int16_t *)realloc(music_buf, num_samples * 2 * info.channels);
            if (tmp)
                music_buf = tmp;
        }

        if (sampleRate)
            *sampleRate = info.hz;
        if (channels)
            *channels = info.channels;
        if (num_samples)
            *totalSampleCount = num_samples;

        free(file_buf);
        return music_buf;
    }
    if (music_buf)
        free(music_buf);
    return 0;
}
//分割路径函数
void splitpath(const char* path, char* drv, char* dir, char* name, char* ext)
{
    const char* end;
    const char* p;
    const char* s;
    if (path[0] && path[1] == ':') {
        if (drv) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    }
    else if (drv)
        *drv = '\0';
    for (end = path; *end && *end != ':';)
        end++;
    for (p = end; p > path && *--p != '\\' && *p != '/';)
        if (*p == '.') {
            end = p;
            break;
        }
    if (ext)
        for (s = end; (*ext = *s++);)
            ext++;
    for (p = end; p > path;)
        if (*--p == '\\' || *p == '/') {
            p++;
            break;
        }
    if (name) {
        for (s = p; s < end;)
            *name++ = *s++;
        *name = '\0';
    }
    if (dir) {
        for (s = path; s < p;)
            *dir++ = *s++;
        *dir = '\0';
    }
}

char* mp32wav(char* mp3file) {
    //总音频采样数
    uint32_t totalSampleCount = 0;
    //音频采样率
    uint32_t sampleRate = 0;
    //通道数
    unsigned int channels = 0;
    int16_t* wavBuffer = NULL;
    wavBuffer = DecodeMp3ToBuffer(mp3file, &sampleRate, &totalSampleCount, &channels);

    //保存结果
    char drive[3];
    char dir[256];
    char fname[256];
    char ext[256];
    char out_file[1024];
    splitpath(mp3file, drive, dir, fname, ext);
    sprintf(out_file, "%s%s%s.wav", drive, dir, fname);
    wavWrite_int16(out_file, wavBuffer, sampleRate, totalSampleCount, channels);
    if (wavBuffer)
    {
        free(wavBuffer);
    }
    printf("successfully convert mp3 file to wav file. \n");
    char* res = out_file;
    return res;
}


// int test()
// {
//     char* in_file = "test.mp3";
//     char* out_file = mp32wav(in_file);
//     printf("%s\n",out_file);
//     return 0;
// }
