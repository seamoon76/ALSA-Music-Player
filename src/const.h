
// 定义音乐全局结构体，参考 https://www.cnblogs.com/ranson7zop/p/7657874.html 表3
// int 由uint32_t代替，short 由uint16_t代替，因为在跨平台后有可能不兼容，类型长度不一致，使用统一的类型
struct WAV_HEADER
{

	char 		chunk_id[4]; // riff 标志号
	uint32_t 	chunk_size; // riff长度
	char 		format[4]; // 格式类型(wav)
	
	char 		sub_chunk1_id[4]; // fmt 格式块标识
	uint32_t 	sub_chunk1_size; // fmt 长度 格式块长度。
	uint16_t 	audio_format; // 编码格式代码									常见的 WAV 文件使用 PCM 脉冲编码调制格式,该数值通常为 1
	uint16_t 	num_channels; // 声道数 									单声道为 1,立体声或双声道为 2
	uint32_t  	sample_rate; // 采样频率 									每个声道单位时间采样次数。常用的采样频率有 11025, 22050 和 44100 kHz。
	uint32_t 	byte_rate; // 传输速率 										该数值为:声道数×采样频率×每样本的数据位数/8。播放软件利用此值可以估计缓冲区的大小。
	uint16_t	block_align; // 数据块对齐单位									采样帧大小。该数值为:声道数×位数/8。播放软件需要一次处理多个该值大小的字节数据,用该数值调整缓冲区。
	uint16_t 	bits_per_sample; // 采样位数								存储每个采样值所用的二进制数位数。常见的位数有 4、8、12、16、24、32

	char 		sub_chunk2_id[4]; // 数据  不知道什么数据
	uint32_t 	sub_chunk2_size; // 数据大小
	
} wav_header;
int wav_header_size; // 接收wav_header数据结构体的大小


/**
	ALSA的变量定义
**/
// 定义用于PCM流和硬件的 
snd_pcm_hw_params_t *hw_params;
// PCM设备的句柄  想要操作PCM设备必须定义
snd_pcm_t *pcm_handle;
// 定义pcm的name snd_pcm_open函数会用到,strdup可以直接把要复制的内容复制给没有初始化的指针，因为它会自动分配空间给目的指针，需要手动free()进行内存回收。
char *pcm_name;
// 定义是播放还是回放等等，播放流 snd_pcm_open函数会用到 可以在 https://www.alsa-project.org/alsa-doc/alsa-lib/group___p_c_m.html#gac23b43ff55add78638e503b9cc892c24 查看
snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
// 定义采样位数
snd_pcm_format_t pcm_format = SND_PCM_FORMAT_S16_LE;


// 缓存大小
#define BUF_LEN 1024
//char buf[BUF_LEN];
unsigned char *buff;
// short * outBuff;// used for speed transfer

unsigned char charBuf1[4];
unsigned char charBuf2[2];

// 周期数
int periods = 2;
// 一个周期的大小，这里虽然是设置的字节大小，但是在有时候需要将此大小转换为帧，所以在用的时候要换算成帧数大小才可以
snd_pcm_uframes_t period_size = 12 * 1024;
snd_pcm_uframes_t frames;
snd_pcm_uframes_t buffer_size; // unit: bytes

// 初始化采样率
unsigned int rate;

// 音乐文件指针变量
FILE *fp;
FILE *out_ptr;

// 混音器
static snd_mixer_t *mixer_handle = NULL;
snd_mixer_elem_t *elem;
snd_mixer_selem_id_t *sid;
int rate_arg=44, format_arg=161;
bool has_file = false;
char *dir = NULL;
long set_max_vol=99;
bool set_volume=false;
int speed=1;
int rate_speed=1;
pthread_t pid_play;
// 函数声明
bool debug_msg(int result, const char *str);
void open_music_file(char *path_name);



int volume_change(long,snd_mixer_elem_t *,int add_type);

// void* set_volume_func();

// void change_volume_func(void * elem,int add_v);
// void change_volume_f(bool add);

// set pcm
void set_format(int format);
// init alsa
int init_alsa(void);
int init_mixer(void);
// set volume
int mixer_close();
// music name list
char **play_list;
int play_list_size=0;
int open_music_dir(char *dir);
int now_play_music_index=0;
void* play_music(void *);
#define MAX_SONG_NUM 20
// global command
bool play_or_not=false;
bool is_playing=false;
bool if_forward=false;
bool if_rewind=false;
int speed_mode=1;// 0 for 0.5x,1 for 1x, 2 for 1.5x, 3 for 2x 
bool change_speed=false;
bool change_sample_rate = false;
bool can_use_eq = false;
bool begin_using_eq=false;
