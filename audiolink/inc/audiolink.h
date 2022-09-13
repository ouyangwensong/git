#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <fftw3.h>


#define AudioFormat SND_PCM_FORMAT_S16_LE   //音频的格式
#define AUDIO_CHANNEL_SET   1  			    //1单声道  
#define AUDIO_RATE_SET      44100           //音频采样频率


#define START_CODE_FREQ 1500    //开始音节频率
#define MARK_CODE_FREQ  1650    //名称与密码之间的标志位音节频率
#define END_CODE_FREQ   6500    //结束音节频率
#define AUDIO_FFT_LEN   882     //进行傅里叶变换数据的个数
#define AUDIO_IDX_TO_FREQ(idx ) ( AUDIO_RATE_SET * (idx) / AUDIO_FFT_LEN)


typedef struct 
{
    int      m_charfreq[96];  //保存音节频率
    int      m_namefreq[32];  //保存接收的名称的频率
    int      m_passfreq[32];  //保存接收的密码的频率
    int      m_namelen;       
    int      m_passlen;       
    char     m_name[32];
    char     m_pass[32];
    int      m_state  ;       //接收的状态：0：不接收；1：接收名称信息；2：接收密码信息
    int      m_check  ;   
} AUDIOLINK_S;

void NK_FREQ_Init(void* plink);
int NK_AUDIO_Record(void* plink);
int NK_FREQ_TO_Char(void* plink);