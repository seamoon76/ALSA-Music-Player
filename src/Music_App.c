#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <bits/getopt_core.h>
#include "const.h"
#include <sys/types.h>
#include <dirent.h>
#include <poll.h>
#include "mp3towav.h"
#include "sonic.h"
#include "Equalizer.h"
//#include "TDStretch.h"

struct pollfd fds[2];
//TDStretch * st;
Equalizer* eq;
int gainValue=10+6;//db
int main(int argc, char *argv [])
{

    int ret;
    bool flag = true;
    play_list = (char **) malloc(sizeof(char*) * 20);
    for(int i=0;i<20;i++)
    {
        play_list[i]=(char *) malloc(sizeof(char) * 100);
    }
//    st=TDStretch::newInstance();


    while((ret = getopt(argc,argv,"d:m:f:r:v:e:h")) != -1){
        flag = false;
        switch(ret){
            case 'd':
                // 目录dir
                dir = optarg;
                if(!access(dir,0) ) {
                    printf("打开目录 \n");
                    debug_msg(open_music_dir(dir), "open_music_dir");
                    if(play_list_size==0)
                    {
                        printf("no music file in this dir \n");
                        return 0;
                    } else{
                        printf("打开文件 %s \n",play_list[0]);
                        now_play_music_index=0;
                        open_music_file(play_list[0]);
                        


                        has_file = true;
                    }
                }else{
                    printf("invalid dir value, now it is set as current dir \n");
                    char dd[2];
                    dd[0]='.';
                    dd[1]='\0';
                    open_music_dir(dd);
                    has_file = true;
                }
                break;
            case 'm':
                printf("打开文件 \n");
                open_music_file(optarg);
                has_file = true;
                break;
            case 'f':
                format_arg = atoi(optarg);
                set_format(format_arg);
                break;

            case 'r':

                rate_arg = atoi(optarg);
                if(rate_arg == 44){

                    printf("rate_arg value is : 44.1HZ \n");
                    rate = 44100;

                }else if(rate_arg == 88){

                    printf("rate_arg value is : 88.2HZ \n");
                    rate = 88200;

                } else if(rate_arg == 8)
                {
                    printf("rate_arg value is : 8HZ \n");
                    rate = 8000;
                }
                else{

                    printf("invalid rate_arg value, now it is set as sample rate 8000\n");
                    rate = 8000;
                    // rate=wav_header.sample_rate;

                }
                break;
            case 'v':
                if(strcmp("0",optarg)==0)
                {
                    set_max_vol = 0;
                    set_volume=true;
                    printf("volume value is : %ld percent \n",set_max_vol);
                    break;
                }
                else {
                    set_max_vol = atoi(optarg);
                    set_volume = true;
                    if (set_max_vol <= 0 || set_max_vol > 100) {
                        printf("volume value should be : 0-100, now it is set as 100. \n");
                        set_max_vol = 100;
                    } else {
                        printf("volume value is : %ld percent \n", set_max_vol);
                    }

                    break;
                }
             case 's':
                 // set play speed 0.5,1,2
                 speed = atof(optarg);
                 if(speed == 0.5 || speed == 1 || speed == 2){
                     printf("speed value is : %d \n",speed);
                 }else{
                     printf("invalid speed value, now it is set as 1 \n");
                     speed = 1;
                 }
                 rate=wav_header.sample_rate*speed;
                 break;
            case 'e':
                gainValue=atoi(optarg);
                printf("gain=%d\n",gainValue);
                gainValue+=6;
                can_use_eq=true;
                break;
            default:
                flag = true;
                break;

        }
    }

    if(flag|| (!has_file)){
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("Either 1'st, 2'nd, 3'th 4'th or all parameters were missing \n");
        printf("\n");
        printf("1'st : -d [music_dir] \n");
        printf("\n");
        printf("2'nd : -f [format 241bit or 16bit or 32bit] \n");
        printf("		161 for S16_LE, 162 for S16_BE \n");
        printf("		241 for S24_LE, 242 for S24_BE \n");
        printf("		2431 for S24_3LE, 2432 for S24_3BE \n");
        printf("		321 for S32_LE, 322 for S32_BE \n");
        printf("\n");
        printf("3'th : -r [rate,8, 44 or 88] \n");
        printf("		8 for 8000hz \n");
        printf("		44 for 44100hz \n");
        printf("		88 for 88200hz \n");
        printf("\n");
        printf("4'th : -v [volume,0-100] \n");
        printf("		0 for mute \n");
        printf("		100 for max volume \n");
        printf("\n");
        printf("5'th : -s [speed,0.5,1,2] \n");
        printf("		0.5 for half speed \n");
        printf("		1 for normal speed \n");
        printf("		2 for double speed \n");
        printf("\n");
        printf("6'th : -e [gain,10~15(dB)] \n");
        printf("Only add \'-e\' can you use Equalizer.\n");
        printf("\n");
        printf("For example: ./Music_App -d . -e 10\n");
        printf("Use \'n\' or \'l\' to play next or last song.\n");
        printf("Use \'f\' or \'r\' to forward or rewind 10s.\n");
        printf("Use \'s\' to change play speed. [1x->1.5x->2x->0.5x->1x]\n");
        printf("Use \'eh\' or \'el\' or \'em\' to add Equalizer gain for high/low/mid part of music.\n");
        //printf("Use \'ea\' to add Equalizer gain for all of high/low/mid part of music.\n");
        printf("Use \'es\' to shutdown the Equalizer\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        exit(1);
    }
    // 设置缓冲区 buffer_size = period_size * periods 一个缓冲区的大小可以这么算，我上面设定了周期为2，
    // 周期大小我们预先自己设定，那么要设置的缓存大小就是 周期大小 * 周期数 就是缓冲区大小了。
    buffer_size = period_size * periods;


    // 为buff分配buffer_size大小的内存空间
    buff = (unsigned char *)malloc(buffer_size*2);
    // outBuff=(short *)malloc(buffer_size);
    init_alsa();
    if(can_use_eq)
    {
        // init eq
        eq = createEqualizer(2, 44100);
    }
    if(set_volume){
        debug_msg(init_mixer(),"init amixer");
    }
    play_or_not=true;
    int ret_set=pthread_create(&pid_play, NULL, play_music, NULL);
    if(ret_set!=0)
    {
        printf("pthread_create error(play music): error_code=%d\n", ret_set);
        exit(1);
    }

    // 初始化监听器，循环接受键盘指令
    char ch;

    while ((ch= getchar())!='q') {
        bool add = false;
        bool sub = false;
        if (ch == 'a' && set_volume) {

            add = true;
            sub = false;
            
            debug_msg(volume_change(0,elem,1),"change volume");

        } else if (ch == 'd' && set_volume) {
            add = false;
            sub = true;
            debug_msg(volume_change(0,elem,-1),"change volume");
        } 
        else if(ch == 'n')
        {
            // next song
            snd_pcm_state_t state;
            state=snd_pcm_state(pcm_handle);
            //printf("state=%d\n",state);
            if ( true){
                
                play_or_not=false;
                snd_pcm_prepare ( pcm_handle );
                if(now_play_music_index<play_list_size-1)
                {
                    now_play_music_index++;
                }
                else if(now_play_music_index>=play_list_size-1)
                {
                    now_play_music_index=0;
                }
                // 打开文件的时候就会把rate和format_arg读取好
                printf("going to play index: %d ",now_play_music_index);
                open_music_file(play_list[now_play_music_index]);
                
                snd_pcm_close(pcm_handle);
                snd_pcm_hw_params_free(hw_params);
                // 重新设定rate
                init_alsa();
                //debug_msg(snd_pcm_hw_params_set_format (pcm_handle, hw_params, pcm_format), "设置样本长度(位数)");
                //debug_msg(snd_pcm_hw_params_set_rate_near (pcm_handle, hw_params, &wav_header.sample_rate, 0), "设置采样率");
                //printf("rate = %d\n", rate);
                

                // play
                play_or_not=true;
                int ret_set=pthread_create(&pid_play, NULL, play_music, NULL);
                if(ret_set!=0)
                {
                    printf("pthread_create error(play music): error_code=%d\n", ret_set);
                    exit(1);
                }
            }
            else
            {
                printf("not playing\n");
            }
        }
        else if(ch == 'l')
        {
            snd_pcm_state_t state;
            state=snd_pcm_state(pcm_handle);
            //printf("state=%d\n",state);
            if ( true){
                
                play_or_not=false;
                snd_pcm_prepare ( pcm_handle );
                if(now_play_music_index>0)
                {
                    now_play_music_index--;
                }
                else if(now_play_music_index==0)
                {
                    
                    now_play_music_index=play_list_size-1;
                }

                // 打开文件的时候就会把rate和format_arg读取好
                printf("going to play index:%d\n",now_play_music_index);
                open_music_file(play_list[now_play_music_index]);
                
                snd_pcm_close(pcm_handle);
                snd_pcm_hw_params_free(hw_params);
                // 重新设定rate
                init_alsa();

                //debug_msg(snd_pcm_hw_params_set_format (pcm_handle, hw_params, pcm_format), "设置样本长度(位数)");
                //debug_msg(snd_pcm_hw_params_set_rate_near (pcm_handle, hw_params, &wav_header.sample_rate, 0), "设置采样率");
                //printf("rate = %d\n", rate);
    
                // play
                play_or_not=true;
                int ret_set=pthread_create(&pid_play, NULL, play_music, NULL);
                if(ret_set!=0)
                {
                    printf("pthread_create error(play music): error_code=%d\n", ret_set);
                    exit(1);
                }
            }
            else
            {
                printf("not playing\n");
            }
        }
        else if(ch=='f')
        {
            snd_pcm_state_t state;
            state=snd_pcm_state(pcm_handle);
            //printf("state=%d\n",state);
            if ( state== SND_PCM_STATE_RUNNING){
                if_forward=true;
            }
        
        }
        else if(ch=='r')
        {
            snd_pcm_state_t state;
            state=snd_pcm_state(pcm_handle);
            //printf("state=%d\n",state);
            if ( state== SND_PCM_STATE_RUNNING){
                if_rewind=true;
            }
        
        }
        else if(ch=='s')
        {
            // 0 for 0.5x,1 for 1x, 2 for 1.5x, 3 for 2x 
            if(change_speed==false)
            {
                change_speed=true;
                // printf("304\n");
            
            }
        }
        else if(ch=='g')
        {
            // change sample rate
            if(change_sample_rate=false)
            {
                change_sample_rate=true;
            }
        }
        else if(ch=='e')
        {
            
            if(wav_header.num_channels != 2 || wav_header.sample_rate != 44100)
            {
                printf("Error, src must be stereo and 44100Hz\n");
                // 不理会该命令
                continue;
            }
            char nextch=getchar();
            if(nextch=='h')
            {
                setTrebleGain(eq,gainValue);
                printf("high gain %d dB\n",gainValue-6);
            }
            else if(nextch=='l')
            {
                setBassGain(eq,gainValue);
                printf("low gain %d dB\n",gainValue-6);
            }
            else if(nextch=='m')
            {
                setMidGain(eq,gainValue);
                printf("mid gain %d dB\n",gainValue-6);
            }
            // else if(nextch=='a'){
            //     setGain(eq,gainValue);
            //     printf("all gain %d dB\n",10);
            // }
            begin_using_eq=true;
            if(nextch=='s')
            {
                // shut down EQ
                begin_using_eq=false;
                printf("shutdown EQ\n");
            }
        }
        else{
            continue;
        }
    }
    snd_pcm_state_t state;
    if ( state== SND_PCM_STATE_RUNNING){
        play_or_not=false;
    }
    pthread_join(pid_play, NULL);
    snd_pcm_close(pcm_handle);
    snd_pcm_hw_params_free(hw_params);
    if(mixer_handle!=NULL) {
        mixer_close();
    }
    // release play_list memory
    for(int i = 0; i < 20; i++){
        free(play_list[i]);
    }
    destroyEqualizer(eq);
    free(play_list);
    free(buff);
    // if(can_use_eq)
    // {
    //     free((void*)eq);
    // }
    // free(outBuff);
    return 0;

}

void open_music_file(char *path_name){
    printf("opening ... : %s\n",path_name);
    // 如果文件是mp3文件，调用mp32wav转为wav
    char *file_name;
    char *wav_name;
    wav_name = (char *) malloc(sizeof(char) * 1024);
    file_name = (char *) malloc(sizeof(char) * 1024);
    if (file_name == NULL) {
        exit(1);
    }
    if (wav_name == NULL) {
        exit(1);
    }
    strcpy(file_name, path_name);
    int filename_len= strlen(file_name);
    if(filename_len>4&&file_name[filename_len-3]=='m'&&file_name[filename_len-2]=='p'&&file_name[filename_len-1]=='3'){
        file_name[filename_len-3] = 'w';
        file_name[filename_len-2] = 'a';
        file_name[filename_len-1] = 'v';
        if(access(file_name,0)==-1)
            strcpy(wav_name,mp32wav(path_name));
        else if(access(file_name,0)==0){
            strcpy(wav_name,file_name);
        }
        else{
            printf("cannot find file %s\n",path_name);
        }
    }
    else {
        strcpy(wav_name, path_name);
    }
//    if(fp!=NULL)
//    {
//        fclose(fp);
//    }
    fp = fopen(wav_name, "rb");
    if (fp == NULL) {
        printf("failed to open file %s, check if the file name is spelled correctly\n",wav_name);
        exit(1);
    }
    int read = 0;
    int wav_file_len= strlen(wav_name);
    if(wav_file_len>4&&wav_name[wav_file_len-3]=='w'&&wav_name[wav_file_len-2]=='a'&&wav_name[wav_file_len-1]=='v'){
        wav_name[wav_file_len-3] = 't';
        wav_name[wav_file_len-2] = 'x';
        wav_name[wav_file_len-1] = 't';
    }
    else{
        strcat(wav_name, ".txt");
    }
    out_ptr = fopen(wav_name, "w+");
    fprintf(out_ptr, "wav文件头结构体大小:%14lu\n", sizeof(wav_header));
    read = fread(wav_header.chunk_id, sizeof(wav_header.chunk_id), 1, fp);
    fprintf(out_ptr, "RIFF标志:%*s\n", 23, wav_header.chunk_id);
    read = fread(charBuf1, sizeof(charBuf1), 1, fp);
    wav_header.chunk_size = charBuf1[0] |
                            (charBuf1[1] << 8) |
                            (charBuf1[2] << 16) |
                            (charBuf1[3] << 24);
    fprintf(out_ptr, "文件大小:%24u\n", wav_header.chunk_size);
    read = fread(wav_header.format, sizeof(wav_header.format), 1, fp);
    fprintf(out_ptr, "文件格式:%*s\n", 24, wav_header.format);
    read = fread(wav_header.sub_chunk1_id, sizeof(wav_header.sub_chunk1_id), 1, fp);
    fprintf(out_ptr, "格式块标识:%*s\n", 23, wav_header.sub_chunk1_id);
    read = fread(charBuf1, sizeof(charBuf1), 1, fp);
    wav_header.sub_chunk1_size = charBuf1[0] |
                                 (charBuf1[1] << 8) |
                                 (charBuf1[2] << 16) |
                                 (charBuf1[3] << 24);
    fprintf(out_ptr, "格式块长度:%22u\n", wav_header.sub_chunk1_size);
    read = fread(charBuf2, sizeof(charBuf2), 1, fp);
    wav_header.audio_format = charBuf2[0] | (charBuf2[1] << 8);
    
    fprintf(out_ptr, "编码格式代码:%20u \n", wav_header.audio_format);
    
    read = fread(charBuf2, sizeof(charBuf2), 1, fp);
    wav_header.num_channels = charBuf2[0] | (charBuf2[1] << 8);
    fprintf(out_ptr, "声道数: %24u\n", wav_header.num_channels);
    read = fread(charBuf1, sizeof(charBuf1), 1, fp);
    wav_header.sample_rate = charBuf1[0] |
                             (charBuf1[1] << 8) |
                             (charBuf1[2] << 16) |
                             (charBuf1[3] << 24);   
    rate = wav_header.sample_rate;    
    printf("rate=%d\n",rate);              
    fprintf(out_ptr, "采样频率:%24u\n", wav_header.sample_rate);
    
    read = fread(charBuf1, sizeof(charBuf1), 1, fp);
    wav_header.byte_rate = charBuf1[0] |
                           (charBuf1[1] << 8) |
                           (charBuf1[2] << 16) |
                           (charBuf1[3] << 24);
    fprintf(out_ptr, "传输速率:%24u\n", wav_header.byte_rate);
    read = fread(charBuf2, sizeof(charBuf2), 1, fp);
    wav_header.block_align = charBuf2[0] |
                             (charBuf2[1] << 8);
    fprintf(out_ptr, "数据块对齐单位:%19u\n", wav_header.block_align);
    read = fread(charBuf2, sizeof(charBuf2), 1, fp);
    wav_header.bits_per_sample = charBuf2[0] |
                                 (charBuf2[1] << 8);
    fprintf(out_ptr, "采样位数(长度):%19u\n", wav_header.bits_per_sample);
    format_arg=wav_header.bits_per_sample;
    set_format(format_arg);
    //printf("413\n");
    printf("Header has been saved into %s\n",wav_name);
    //fclose(ptr);
    fclose(out_ptr);
    free(wav_name);
    free(file_name);

}


bool debug_msg(int result, const char *str)
{
    if(result < 0){
        printf("err: %s 失败!, result = %d, err_info = %s \n", str, result, snd_strerror(result));
        exit(1);
    }
    return true;
}

int volume_change(long set_max_volume,snd_mixer_elem_t *elem,int add_type)
{
    long min_vol, max_vol, now_vol;
    min_vol = 0; //最大最小音量
    max_vol = 0;
    now_vol = 0;
    int ret_val = 0;

    if (!elem)
    {
        printf("snd_mixer_find_selem Err\n");
        printf("3333");
        snd_mixer_close(mixer_handle);
        mixer_handle = NULL;
        return -ENOENT;
    }

    int ret1=snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
    min_vol/=100;
    max_vol/=100;
    debug_msg(ret1, "snd_mixer_selem_get_playback_volume_range error");
    printf("volume range: %ld -- %ld\n", min_vol, max_vol);

    // snd_mixer_handle_events(mixer_handle);
    snd_mixer_selem_get_playback_volume(elem,(snd_mixer_selem_channel_id_t)0,&now_vol);
    if(add_type==0)
    {
        set_max_volume=set_max_volume*max_vol/100;
    }
    else if(add_type==1)
    {
        set_max_volume = now_vol + (0.1) * max_vol;
    }
    else if(add_type==-1)
    {
        set_max_volume = now_vol - (0.1) * max_vol;
    }
    else{
        return 0;
    }

    
    if(set_max_volume>max_vol)
    {
        set_max_volume = max_vol;
        printf("set_max_volume > max_vol, set_max_volume = max_vol = %ld\n", set_max_volume);
    } else if(set_max_volume<min_vol)
    {
        set_max_volume = min_vol;
        printf("set_max_volume < min_vol, set_max_volume = min_vol = %ld\n", set_max_volume);
    }
    long vol_percent = (set_max_volume*100/max_vol);
    if (snd_mixer_selem_is_playback_mono(elem))
    {
        ret_val = snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, set_max_volume);
        if(ret_val < 0) {
            printf("set volume %ld, err:%d\n", vol_percent, ret_val);
        } else{
            printf("set volume %ld percent, \n", vol_percent);
        }
    }
    else
    {
        ret_val = snd_mixer_selem_set_playback_volume_all(elem, set_max_volume);
        if(ret_val < 0)
            printf("set volume %ld, err:%d\n",vol_percent, ret_val);
        else
            printf("set volume %ld percent\n", vol_percent);
    }
    snd_mixer_selem_get_playback_volume(elem,(snd_mixer_selem_channel_id_t)0,&now_vol);
    printf("currnet volume = %ld\n", now_vol);

    return 0;
}



int open_music_dir(char *directory){
    // check if directory path exist and get file name list
    // please help to implement this function
    // return 0 if success, -1 if fail
    // you can use global variable "play_list" to store the file name list
    // you can use global variable "play_list_size" to store the file name list size

    int retval = 0;
    struct dirent *de;  // Pointer for directory entry
    DIR *dr;
    // check if the Directory exist or not
    dr = opendir(directory);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
        retval = -1;
        return retval;
    }
    int path_length=0;
    char* file_name = (char *) malloc(sizeof(char) * 1024);

    // Traverse directory and store the File name in a list
    while ((de = readdir(dr)) != NULL) {
        // ignore '.' and '..' folder
        if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0)
            continue;
        path_length=strlen(de->d_name);
        if(path_length>4&&de->d_name[path_length-3]=='m'&&de->d_name[path_length-2]=='p'&&de->d_name[path_length-1]=='3')
        {
            strcpy(file_name, de->d_name);
            file_name[path_length-3] = 'w';
            file_name[path_length-2] = 'a';
            file_name[path_length-1] = 'v';
            if(access(file_name,0)==-1){
		mp32wav(de->d_name);
                sprintf(play_list[play_list_size], "%s", de->d_name);
                play_list_size++;
            }
            else{
                // 已经存在
                //strcpy(wav_name,file_name);
            } 
            
        }
        else if(path_length>4&&de->d_name[path_length-3]=='w'&&de->d_name[path_length-2]=='a'&&de->d_name[path_length-1]=='v')
        {
            sprintf(play_list[play_list_size], "%s", de->d_name);
            play_list_size++;
        }

        // Store the file name in a list
        
    }
    closedir(dr);
    free(file_name);

    // If the File name list is empty, directory doesn't contain any music files
    if(play_list_size == 0){
        printf("Directory doesn't contain any music files\n");
        retval = -1;
        return retval;
    }
    return retval;
}

void set_format(int format)
{
    // 判断是哪种采样位
    switch(format){
        case 161:
            printf("format_arg value is : S16LE \n");
            pcm_format = SND_PCM_FORMAT_S16_LE;
            break;

        case 162:
            printf("format_arg value is : S16BE \n");
            pcm_format = SND_PCM_FORMAT_S16_BE;
            break;

        case 201:
            printf("format_arg value is : S20LE \n");
            //pcm_format = SND_PCM_FORMAT_S20_LE;
            break;

        case 202:
            printf("format_arg value is : S20BE \n");
            //pcm_format = SND_PCM_FORMAT_S20_BE;
            break;

        case 241:
            printf("format_arg value is : S24LE \n");
            pcm_format = SND_PCM_FORMAT_S24_LE;
            break;

        case 242:
            printf("format_arg value is : S24BE \n");
            pcm_format = SND_PCM_FORMAT_S24_BE;
            break;

        case 2431:
            printf("format_arg value is : S243LE \n");
            pcm_format = SND_PCM_FORMAT_S24_3LE;
            break;

        case 2432:
            printf("format_arg value is : S243BE \n");
            pcm_format = SND_PCM_FORMAT_S24_3BE;
            break;

        case 321:
            printf("format_arg value is : S32LE \n");
            pcm_format = SND_PCM_FORMAT_S32_LE;
            break;

        case 322:
            printf("format_arg value is : S32BE \n");
            pcm_format = SND_PCM_FORMAT_S32_BE;
            break;
        default:
            // printf("invalid format_arg value, now it is set as: S16LE \n");
            pcm_format = SND_PCM_FORMAT_S16_LE;
            break;
    }
    return;
}

int init_alsa(){
    // 需要先指定文件
    if(!has_file)
    {
        debug_msg(-1,"没有指定文件");
        return -1;
    }
    // 在堆栈上分配snd_pcm_hw_params_t结构体的空间，参数是配置pcm硬件的指针,返回0成功
    debug_msg(snd_pcm_hw_params_malloc(&hw_params), "分配snd_pcm_hw_params_t结构体");

    // 打开PCM设备 返回0 则成功，其他失败
    // 函数的最后一个参数是配置模式，如果设置为0,则使用标准模式
    // 其他值位SND_PCM_NONBLOCL和SND_PCM_ASYNC 如果使用NONBLOCL 则读/写访问, 如果是后一个就会发出SIGIO
    pcm_name = strdup("default");
    debug_msg(snd_pcm_open(&pcm_handle, pcm_name, stream, 0)  , "打开PCM设备");

    // 在我们将PCM数据写入声卡之前，我们必须指定访问类型，样本长度，采样率，通道数，周期数和周期大小。
    // 首先，我们使用声卡的完整配置空间之前初始化hw_params结构
    debug_msg(snd_pcm_hw_params_any(pcm_handle,hw_params), "配置空间初始化");

    // 设置交错模式（访问模式）
    // 常用的有 SND_PCM_ACCESS_RW_INTERLEAVED（交错模式） 和 SND_PCM_ACCESS_RW_NONINTERLEAVED （非交错模式）
    // 参考：https://blog.51cto.com/yiluohuanghun/868048
    debug_msg(snd_pcm_hw_params_set_access (pcm_handle, hw_params,  SND_PCM_ACCESS_RW_INTERLEAVED), "设置交错模式（访问模式）");


    debug_msg(snd_pcm_hw_params_set_format (pcm_handle, hw_params, pcm_format), "设置样本长度(位数)");

    debug_msg(snd_pcm_hw_params_set_rate_near (pcm_handle, hw_params, &rate, 0), "设置采样率");
    //printf("rate = %d\n", rate);

    debug_msg(snd_pcm_hw_params_set_channels(pcm_handle, hw_params, wav_header.num_channels), "设置通道数");



    if(format_arg == 161 || format_arg == 162){

        frames = buffer_size >> 2;

        debug_msg(snd_pcm_hw_params_set_buffer_size(pcm_handle,hw_params,frames), "设置S16_LE OR S16_BE缓冲区");

    }else if(format_arg == 2431 || format_arg == 2432){

        frames = buffer_size / 6;

        /*
            当位数为24时，就需要除以6了，因为是24bit * 2 / 8 = 6
        */
        debug_msg(snd_pcm_hw_params_set_buffer_size(pcm_handle,hw_params,frames), "设置S24_3LE OR S24_3BE的缓冲区");

    }else if(format_arg == 321 || format_arg == 322 || format_arg == 241 || format_arg == 242){

        frames = buffer_size >> 3;
        /*
            当位数为32时，就需要除以8了，因为是32bit * 2 / 8 = 8
        */
        debug_msg(snd_pcm_hw_params_set_buffer_size(pcm_handle,hw_params,frames), "设置S32_LE OR S32_BE OR S24_LE OR S24_BE缓冲区");
    }
    else{
        // default format_arg = 161
        frames = buffer_size >> 2;

        debug_msg(snd_pcm_hw_params_set_buffer_size(pcm_handle,hw_params,frames), "设置S16_LE OR S16_BE缓冲区");


    }
    //printf("frames = %d\n", frames);


    // 设置的硬件配置参数，加载，并且会自动调用snd_pcm_prepare()将stream状态置为SND_PCM_STATE_PREPARED
    debug_msg(snd_pcm_hw_params (pcm_handle,  hw_params), "设置的硬件配置参数");
    return 0;
}

int init_mixer()
{
    // set mixer
    long now_vol = 0;
    //get_volume(&now_vol);

    int ret_val = 0;



    if ((ret_val = snd_mixer_open(&mixer_handle, 0)) < 0)
    {
        debug_msg(ret_val, "snd_mixer_open error");
        //return ret_val;
    }
    snd_mixer_attach(mixer_handle, "default");
    snd_mixer_selem_register(mixer_handle, NULL, NULL);
    snd_mixer_load(mixer_handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, "Master");
    elem = snd_mixer_find_selem(mixer_handle, sid);


    if (!elem)
    {
        printf("snd_mixer_find_selem Err\n");
//        printf("1111\n");
        snd_mixer_close(mixer_handle);
        mixer_handle = NULL;
        exit(1);
        //return -ENOENT;
    }
    else{
        printf("find elem\n");
    }
    snd_mixer_handle_events(mixer_handle);


    if(set_volume) {
        int r = volume_change(set_max_vol,elem,0);
        printf("r = %d\n", r);
        if (r == -ENOENT) {
            printf("set volume error:-ENOENT\n");
        }
    }

    
    return 0;
}

int mixer_close()
{
    int res = snd_mixer_close(mixer_handle);
    mixer_handle = NULL;
    
    return res;
}

uint16_t speedUpMultiplier(unsigned char *buff, uint16_t buff_size)
{
    uint16_t buffint[1];

    buffint[0]=*(uint16_t *)buff;
    uint16_t new_int_buffer_size= buff_size;
    uint16_t *temp = (uint16_t *)malloc(new_int_buffer_size);

    int i;
    int j = 0;
    printf("797\n");
    // Copy all the even elements from the
    // original array to the new array
    for (i = 0; i < new_int_buffer_size/2 - 1; i += 2*periods)
    {
        temp[j] = buffint[i];
        j++;
        temp[j]=buffint[i+1];
        j++;
    }
    printf("805\n");
    // Copy all the decoded elements of the
    // new array to the original array
    for (i = 0; i < j; i++)
    {
        buffint[i] = temp[i];
    }
    printf("812\n");
    // To make the speed of audio file by 2
    // reduce the size of the original array
    buff_size = j*2;

    // Reallocate the size of the original array
    // to the new size
//    buff =  (unsigned char*)realloc(buff, buff_size * sizeof(unsigned char));
    return buff_size;
}

/* Run sonic. */
static int runSonic(short* inBuffer, short* outBuffer, float tempo, float rate,int samplesHave) {
  // both inBuffer and outBuffer size is buffer size
  sonicStream stream;

  int samplesRead, samplesWritten, maxSamplesRead;

  if (inBuffer == NULL) {
    printf("Unable to read from buffer\n");
    exit(1);
  }
  stream = sonicCreateStream(rate, wav_header.num_channels);
  sonicSetSpeed(stream, tempo);

  maxSamplesRead = (samplesHave/2)/wav_header.num_channels;

  sonicWriteShortToStream(stream, inBuffer, maxSamplesRead);
    
  samplesWritten = sonicReadShortFromStream(stream, outBuffer,
                                                    maxSamplesRead*2);

    
  sonicDestroyStream(stream);
  return samplesWritten;
}

/* Run sonic. */
static int runEQ(short* inBuffer, short* outBuffer, float rate,int samplesHave) {
  // both inBuffer and outBuffer size is buffer size

  int processedSamples, samplesRead;

  samplesRead = (samplesHave/2)/wav_header.num_channels;

  eqProcess(eq,inBuffer, samplesRead);

  int readBytes = samplesRead * 2 * sizeof(int16_t);
    
  processedSamples = samplesRead;
  return processedSamples;
}

void* play_music(void * e)
{

    if(fp==NULL)
    {
        printf("fp is null\n");
        pthread_exit(NULL);
    }
    int ret_f =0;
    memset(buff, '\0', sizeof(buff));

    // feof函数检测文件结束符，结束：非0, 没结束：0 !feof(fp)
    while(1){
        
        if(!play_or_not)
        {
            // stop play
            printf("stop playing\n");
            debug_msg(snd_pcm_drop(pcm_handle),"stop error");
            fclose(fp);
            pthread_exit(NULL);
        }
        if(if_forward)
        {
            // forward
            //printf("before forward at:%ld\n",ftell(fp));
            int r=fseek(fp,10*wav_header.byte_rate,SEEK_CUR);
            if(r!=0)
            {
                printf("fseek error code:%d\n",r);
            }
            else
                printf("forward 10s.now at:%ld\n",ftell(fp));
            if_forward=false;
        }
        else if(if_rewind)
        {
            //byte is fseek unit
            //printf("before rewind at:%ld\n",ftell(fp));
            int now_rate = wav_header.sample_rate;
            long nn=-10*((long)wav_header.byte_rate);
            //printf("byte rate long%ld",(long)wav_header.byte_rate);
            //printf("byte rate%u\n",wav_header.byte_rate);
            //printf("rewind %ld, byte rate%ld\n",nn,(long)wav_header.byte_rate);
            int r=fseek(fp,nn,SEEK_CUR);
            if(r!=0)
            {
                printf("fseek error code:%d\n",r);
            }
            else
                printf("rewind 10s. now at:%ld\n",ftell(fp));
            if_rewind=false;
        }
        // else if(change_speed)
        // {
        //     if(speed==1)
        //     {
        //         speed=2;
        //     }
        //     else if(speed==2)
        //     {
        //         speed=3;
        //     }
        //     else if(speed==3)
        //     {
        //         speed=0;
        //     }
        //     else if(speed==0)
        //     {
        //         speed=1;
        //     }
        //     else{
        //         speed=1;
        //     }
        // }
        // 读取文件数据放到缓存中
        if(change_speed)
        {
            if(speed==1)
            {
                speed=2;
                

                printf("set speed to : 1.5x\n");
            }
            else if(speed==2)
            {
                speed=3;
                
                printf("set speed to : 2x\n");
            }
            else if(speed==3)
            {
                speed=0;
                
                printf("set speed to : 0.5x\n");
            }
            else if(speed==0)
            {
                speed=1;
                
                printf("set speed to : 1x\n");
            }
            else{
                speed=1;
                
                // printf("set speed to : 1.5x\n");
            }
            snd_pcm_close(pcm_handle);
            snd_pcm_hw_params_free(hw_params);
            // 重新设定rate
            init_alsa();
            change_speed=false;
        }
        if(change_sample_rate)
        {
            if(rate_speed==1)
            {
                rate_speed=2;
                rate=wav_header.sample_rate*1.5;
                printf("set sample rate to : 1.5x\n");
            }
            else if(rate_speed==2)
            {
                rate_speed=3;
                rate=wav_header.sample_rate*2;
                printf("set sample rate to : 2x\n");
            }
            else if(rate_speed==3)
            {
                rate_speed=0;
                rate=wav_header.sample_rate*0.5;
                printf("set sample rate to : 0.5x\n");
            }
            else if(rate_speed==0)
            {
                rate_speed=1;
                rate=wav_header.sample_rate;
                printf("set sample rate to : 1x\n");
            }
            else{
                rate_speed=1;
                rate=wav_header.sample_rate;
                // printf("set speed to : 1.5x\n");
            }
            snd_pcm_close(pcm_handle);
            snd_pcm_hw_params_free(hw_params);
            // 重新设定rate
            init_alsa();
            change_sample_rate=false;
        }

        ret_f = fread(buff, 1, buffer_size, fp);// ret_f unit is Byte
        uint16_t speeded_frames=frames;
        // printf("935\n");



//         //printf("begin play 793\n");
//         if(speed!=1)
//         {

//             int writed_samples = 0;
//             uint16_t new_buffer_size=buffer_size;
//             float tempo= 0.5+speed*0.5;
//             printf("want tempo %f\n",tempo);
//             new_buffer_size = speedUpMultiplier(buff, new_buffer_size);

//             printf("convert %d bytes to %d bytes\n", ret_f,new_buffer_size);
//             writed_samples=new_buffer_size/4;
//             speeded_frames=writed_samples;
//             printf("convert %d frames to %d frames\n",frames,speeded_frames);

//             if(change_speed) {
//                 change_speed = false;
//             }

// //            st->setParameters(wav_)
//         }

        if(ret_f == 0){

            printf("end of music file input! use [n] to play next or [l] to play last.\n");
            pthread_exit(NULL);
        }

        if(ret_f < 0){

            printf("read pcm from file! \n");
            pthread_exit(NULL);
        }
        int samWritten=(ret_f/2)/2;
        int framesWritten=frames;
        if(speed!=1)
        {
            short *inBuffer=(short*)buff;
            float tempo= 0.5+speed*0.5;
            int samWritten=runSonic(inBuffer, inBuffer, tempo, rate,ret_f);
            if(samWritten>0)
            {
                // memcpy(buff,outBuff,buffer_size);
                //printf("samples num from %d to %d\n",(ret_f/2)/2,samWritten);
                framesWritten=samWritten;
                //printf("frames num from %d to %d\n",frames,framesWritten);
            }
        }
        if(begin_using_eq && speed==1)
        {
            short *inBuffer=(short*)buff;
            
            int samWritten=runEQ(inBuffer, inBuffer, rate,ret_f);
            if(samWritten>0)
            {
                // memcpy(buff,outBuff,buffer_size);
                //printf("samples num from %d to %d\n",(ret_f/2)/2,samWritten);
                framesWritten=samWritten;
                // printf("frames num from %d to %d\n",frames,framesWritten);
            }
        }
        else if(begin_using_eq && speed!=1)
        {
            printf("Eq can only used in speed 1x.\n");
        }
// printf("970\n");
        // 向PCM设备写入数据,
        while((ret_f= snd_pcm_writei(pcm_handle, buff, framesWritten)) < 0){
            //printf("in while : ret_f= %d\n", ret);
            if (ret_f == -EPIPE){
                /* EPIPE means underrun -32  的错误就是缓存中的数据不够 */
                //printf("underrun occurred -32, err_info = %s \n", snd_strerror(ret_f));
                // sleep(1);
                //完成硬件参数设置，使设备准备好
                snd_pcm_prepare(pcm_handle);
            }
            else if(ret_f== -EBADFD){
                snd_pcm_state_t state;
                state=snd_pcm_state(pcm_handle);
                printf("state error:%d\n",state);
                snd_pcm_prepare(pcm_handle);
            }
            else if(ret_f== -ESTRPIPE){
                printf("-ESTRPIPE\n");
            }
            else if(ret_f< 0){
                // printf("ret_f value is : %d \n", ret_f);
                printf("stop.\n");
                fclose(fp);
                pthread_exit(NULL);
                //debug_msg(-1, "write to audio interface failed");

            } else{
                printf("exception code:%d.\n",ret_f);
                fclose(fp);
                pthread_exit(NULL);
                // printf("254: ret_f = %d\n", ret_f);
                // debug_msg(ret_f, "unknown error");
            }
        }
        //printf("end of while: ret_f= %d\n", ret);
    }

    fprintf(stderr, "end of music file input\n");


    // 关闭文件
    fclose(fp);
    pthread_exit(NULL);

}
