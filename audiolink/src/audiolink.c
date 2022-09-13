#include "../inc/audiolink.h"
/**
 * @brief 初始化保存频率的数组
 * 
 * @param plink 结构体指针
 */
void NK_FREQ_Init(void* plink)
{
	AUDIOLINK_S *psl = (AUDIOLINK_S*)plink;
	for (int i = 0;i < 95;i++)
	{
		psl->m_charfreq[i] = 1700 + (i*50);
	}
	return;
}

/**
 * @brief 初始化录音pcm设备参数
 * 
 * @param phandle 	指向PCM设备的句柄
 * @param phw_params 此结构包含有关硬件的信息，可用于指定PCM流的配置
 * @param format 	音频的格式
 * @param irate 	音频采样频率
 * @return int 		成功返回0，失败返回-1
 */
static int PCM_Init(snd_pcm_t *phandle,snd_pcm_hw_params_t *phw_params,snd_pcm_format_t format,unsigned int irate)
{
    int ierr = 0;
    if ((ierr = snd_pcm_hw_params_malloc(&phw_params)) < 0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}
	
	if ((ierr=snd_pcm_hw_params_any(phandle,phw_params)) < 0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}

	if ((ierr = snd_pcm_hw_params_set_access (phandle,phw_params,SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}

	if ((ierr=snd_pcm_hw_params_set_format(phandle, phw_params,format)) < 0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}

	if ((ierr=snd_pcm_hw_params_set_rate_near(phandle,phw_params,&irate,0))<0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}

	if ((ierr = snd_pcm_hw_params_set_channels(phandle, phw_params,AUDIO_CHANNEL_SET)) < 0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}

	if ((ierr=snd_pcm_hw_params (phandle,phw_params))<0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}
	printf("参数设置成功.\n");

	snd_pcm_hw_params_free(phw_params);

	if ((ierr=snd_pcm_prepare(phandle))<0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}
    return 0;
}

int prefreq,g_prev,g_startnum,g_recvnum = 0;		//prev:记录上一次音节; startnum:记录接收到开始音节次数;recvnum:记录接收到音节次数

/**
 * @brief 判断音节
 * 
 * @param plink 结构体指针
 * @param ifreq 频率
 * @return int 接收到结束音节返回0，否则返回1
 */
static int FREQ_Recognize(void *plink,int ifreq)
{
	AUDIOLINK_S *psl = (AUDIOLINK_S*)plink;

	if(ifreq == START_CODE_FREQ)
	{
		psl->m_check = 1;
		g_startnum ++;
		return 1;
	}
	else if (ifreq == END_CODE_FREQ)
	{
		psl->m_check = 0;
		psl->m_state = 0;
		return 0;
	}
	else if(ifreq == MARK_CODE_FREQ)
	{
		psl->m_check = 2;
		psl->m_state = 0;
		g_prev = 0;
		return 1;
	}

	if(psl->m_check == 1)
	{
		if (ifreq == START_CODE_FREQ)
		{
			g_startnum ++;
		}
		else if (ifreq > MARK_CODE_FREQ && ifreq < END_CODE_FREQ)
		{
			//printf("z = %d\n",z);
			psl->m_state = 1;
		}
		
	}
	else if (psl->m_check == 2)
	{
		if (ifreq > MARK_CODE_FREQ && ifreq < END_CODE_FREQ)
		{
			psl->m_state = 2;
		}	
	}	
	
	if (psl->m_state == 1)		//接收名称信息
	{   
        if(prefreq!=ifreq)
		{
			prefreq=ifreq;
			g_recvnum = 1;
		}
		else if (prefreq == ifreq && g_recvnum < g_startnum)
		{
			g_recvnum ++;
		}
		if (g_recvnum > g_startnum/2 || g_recvnum == g_startnum)
		{
			if(psl->m_namefreq[g_prev] != ifreq || g_recvnum == g_startnum)
			{
				psl->m_namefreq[psl->m_namelen] = ifreq;
				g_prev = psl->m_namelen;
				g_recvnum = 0;
				psl->m_namelen ++;
			}
		}
		
	}
	else if (psl->m_state == 2)	//接收密码信息
	{   
		if(prefreq!=ifreq)
		{
			prefreq=ifreq;
			g_recvnum = 1;
		}
		else if (prefreq == ifreq && g_recvnum < g_startnum)
		{
			g_recvnum ++;
		}
		if (g_recvnum > g_startnum/2 || g_recvnum == g_startnum)
		{
			if(psl->m_passfreq[g_prev] != ifreq || g_recvnum == g_startnum)
			{
				psl->m_passfreq[psl->m_passlen] = ifreq;
				g_prev = psl->m_passlen;
				g_recvnum = 0;
				psl->m_passlen ++;
			}
		}
	}
	psl->m_state = 0;
	return 1;
}

/**
 * @brief 快速傅里叶变换解析音频数据，判断标志音节,记录信息
 * 
 * @param plink 结构体指针
 * @param ilen 	数据个数
 * @param pin 	要进行快速傅里叶变换的数据
 * @return int 	接收到结束音节返回0，否则返回1
 */
static int FFT_Analysis(void *plink, int ilen,float *pin)
{
    int ire = 1;
	AUDIOLINK_S *psl = (AUDIOLINK_S*)plink;
	fftwf_complex *out = NULL;
	out = (fftwf_complex *) fftwf_malloc(sizeof(fftw_complex) * ilen);
	memset(out,0,sizeof(out));
	fftwf_plan q;
	q = fftwf_plan_dft_r2c_1d(ilen, pin, out, FFTW_ESTIMATE);
    fftwf_execute(q);			//快速傅里叶变换

	float max = 0;
	int freqidx = 0;
    for (int i = 0; i < ilen/2; i++) 
	{
        
        float cur = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        if(max < cur + 1000 && i-1 > 1)
        {
            max = cur;
			freqidx = i;
        }
	} 
	int freq = AUDIO_IDX_TO_FREQ(freqidx);
	//printf("freq = %d\n",freq);			//输出音频频率
	
	ire = FREQ_Recognize(psl,freq);			//判断音节
	
    fftwf_destroy_plan(q);
    fftwf_free(out);
    return ire;
}


/**
 * @brief 录音
 * 
 * @param plink 结构体指针
 * @return int 成功返回0，失败返回-1
 */
int NK_AUDIO_Record(void *plink)
{
	AUDIOLINK_S *psl = (AUDIOLINK_S*)plink;
	int ierr = 0;
	int16_t *pbuffer = NULL;
	int buffer_frames = 882; 
	snd_pcm_t *capture_phandle;
	snd_pcm_hw_params_t *phw_params;
	
	snd_pcm_format_t format=AudioFormat;

	if ((ierr = snd_pcm_open (&capture_phandle, "default",SND_PCM_STREAM_CAPTURE,0))<0) 
	{
		printf("%d\n",__LINE__);
		return -1;
	}
	/*
	FILE *pcm_data_file=NULL;
	if ((pcm_data_file = fopen("test.pcm", "wb")) == NULL)
	{
		printf("%d\n",__LINE__);
		return -1;
	} 
	*/
    PCM_Init(capture_phandle,phw_params,format,AUDIO_RATE_SET);

	int frame_byte=snd_pcm_format_width(format)/8;
	int bufLength = buffer_frames*frame_byte*AUDIO_CHANNEL_SET;
	pbuffer=(int16_t*)malloc(bufLength); 
	printf("开始采集数据...\n");

	float buf[882] = {0};

	int iloops = 1000;
	int iflag = 1;
	
	while (iloops > 0) 
	{
		iloops --;

		//读取音频数据到缓存区
		if ((ierr=snd_pcm_readi(capture_phandle,pbuffer,buffer_frames))!=buffer_frames) 
		{
			printf("%d\n",__LINE__);
			return -1;
		}
        
		for (int i = 0; i < AUDIO_FFT_LEN; i++)
		{
			buf[i] = pbuffer[i];
		}        
		
		if (iflag = FFT_Analysis(psl,AUDIO_FFT_LEN,buf) == 0)
		{
			break;
		}

		//写数据到文件
		//fwrite(buffer,(buffer_frames*AUDIO_CHANNEL_SET),frame_byte,pcm_data_file);	

	}
	printf("录音结束\n");
	free(pbuffer);
	pbuffer = NULL;
	snd_pcm_close(capture_phandle);
	//fclose(pcm_data_file);
	return 0;
}

/**
 * @brief 由频率找到对应的字符并保存到数组中
 * 
 * @param plink 结构体指针
 * @return int 成功返回0，失败返回-1
 */
int NK_FREQ_TO_Char(void* plink)
{
	AUDIOLINK_S *psl = (AUDIOLINK_S*)plink;

	if (psl->m_namelen == 0 || psl->m_passlen == 0)
	{
		return -1;
	}

	for (int i = 0;i < psl->m_namelen;i++)
	{
		for (int j = 0;j < 95;j++)
		{
			if (psl->m_namefreq[i] == psl->m_charfreq[j])
			{
				psl->m_name[i] = j + 32;
				break;
			}
		}
	}

	for (int i = 0;i < psl->m_passlen;i++)
	{
		for (int j = 0;j < 95;j++)
		{
			if (psl->m_passfreq[i] == psl->m_charfreq[j])
			{
				psl->m_pass[i] = j + 32;
				break;
			}
		}
	}
	return 0;
}
