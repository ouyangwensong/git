#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable : 4996)

static struct APP0infotype {
	unsigned short int marker;           // = 0xFFE0
	unsigned short int length;           
	unsigned char      JFIFsignature[5]; 
	unsigned char      versionhi;   
	unsigned char      versionlo;      
	unsigned char      xyunits;        
	unsigned short int xdensity;       
	unsigned short int ydensity;        
	unsigned char      thumbnwidth;  
	unsigned char      thumbnheight;  
} APP0info = { 0xFFE0,16,'J','F','I','F',0,1,1,0,1,1,0,0 };

static struct  SOF0infotype {
	unsigned short int marker;         // = 0xFFC0
	unsigned short int length;        
	unsigned char      precision;     
	unsigned short int height;
	unsigned short int width;
	unsigned char      nrofcomponents; 
	unsigned char      IdY;
	unsigned char      HVY;           
									   
	unsigned char      QTY;          
	unsigned char      IdCb;     
	unsigned char      HVCb;
	unsigned char      QTCb;        
	unsigned char      IdCr;       
	unsigned char      HVCr;
	unsigned char      QTCr;          
} SOF0info = { 0xFFC0,17,8,0,0,3,1,0x22,0,2,0x11,1,3,0x11,1 };



static struct DQTinfotype {
	unsigned short int marker;      // = 0xFFDB（标记代码）
	unsigned short int length;      // = 132（除标记代码后的数据总长度）

	unsigned char      QTYinfo;     //（高四位量化表精度0：8位，1：16位；低四位量化表ID：0~3） 
	unsigned char      Ytable[64];

	unsigned char      QTCbinfo;    
	unsigned char      Cbtable[64];
} DQTinfo;

static struct DHTinfotype {
	unsigned short int marker;           // = 0xFFC4（两字节的标记代码）
	unsigned short int length;           //0x01A2（除标记代码后的数据总长度）

	unsigned char      HTYDCinfo;       
										
	unsigned char      YDC_nrcodes[16];  
	unsigned char      YDC_values[12];	

	unsigned char      HTYACinfo;       
	unsigned char      YAC_nrcodes[16];
	unsigned char      YAC_values[162];  

	unsigned char      HTCbDCinfo;       
	unsigned char      CbDC_nrcodes[16];
	unsigned char      CbDC_values[12];

	unsigned char      HTCbACinfo;      
	unsigned char      CbAC_nrcodes[16];
	unsigned char      CbAC_values[162];
} DHTinfo;

static struct SOSinfotype {
	unsigned short int marker;    		
	unsigned short int length;    		
	unsigned char      nrofcomponents; 	

	unsigned char      IdY;            
	unsigned char      HTY;          
									

	unsigned char      IdCb;           	
	unsigned char      HTCb;           	

	unsigned char      IdCr;          
	unsigned char      HTCr;          

	unsigned char      Ss, Se, Bf;       	
} SOSinfo = { 0xFFDA,12,3,1,0,2,0x11,3,0x11,0,0x3F,0 };

typedef struct { unsigned char length; unsigned short int value; } bitstring;

#define  Y(R,G,B) ((unsigned char)( (YRtab[(R)] + YGtab[(G)] + YBtab[(B)]) >> 16 ) - 128)
#define Cb(R,G,B) ((unsigned char)( (CbRtab[(R)] + CbGtab[(G)] + CbBtab[(B)]) >> 16 ))
#define Cr(R,G,B) ((unsigned char)( (CrRtab[(R)] + CrGtab[(G)] + CrBtab[(B)]) >> 16 ))


#define writebyte(b) fputc((b),fp_jpeg_stream)
#define writeword(w) writebyte((w)/256);writebyte((w)%256);

static unsigned char zigzag[64] = {
  0,  1,  5,  6,  14, 15, 27, 28,
  2,  4,  7,  13, 16, 26, 29, 42,
  3,  8,  12, 17, 25, 30, 41, 43,
  9,  11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63
};


static unsigned char DQT_Y[64] = {
  0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07,
  0x07, 0x07, 0x09, 0x09, 0x08, 0x0a, 0x0c, 0x14,
  0x0d, 0x0c, 0x0b, 0x0b, 0x0c, 0x19, 0x12, 0x13,
  0x0f, 0x14, 0x1d, 0x1a, 0x1f, 0x1e, 0x1d, 0x1a,
  0x1c, 0x1c, 0x20, 0x24, 0x2e, 0x27, 0x20, 0x22,
  0x2c, 0x23, 0x1c, 0x1c, 0x28, 0x37, 0x29, 0x2c,
  0x30, 0x31, 0x34, 0x34, 0x34, 0x1f, 0x27, 0x39,
  0x3d, 0x38, 0x32, 0x3c, 0x2e, 0x33, 0x34, 0x32
};

static unsigned char DQT_C[64] = {
  0x09, 0x09, 0x09, 0x0c, 0x0b, 0x0c, 0x18, 0x0d,
  0x0d, 0x18, 0x32, 0x21, 0x1c, 0x21, 0x32, 0x32,
  0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
  0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
  0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
  0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
  0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
  0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32
};


static unsigned char std_luminance_qt[64] = {
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};
static unsigned char std_chrominance_qt[64] = {
  17,  18,  24,  47,  99,  99,  99,  99,
  18,  21,  26,  66,  99,  99,  99,  99,
  24,  26,  56,  99,  99,  99,  99,  99,
  47,  66,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99
};

static unsigned char std_dc_luminance_nrcodes[17] = { 0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0 };
static unsigned char std_dc_luminance_values[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

static unsigned char std_dc_chrominance_nrcodes[17] = { 0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };
static unsigned char std_dc_chrominance_values[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

static unsigned char std_ac_luminance_nrcodes[17] = { 0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d };
static unsigned char std_ac_luminance_values[162] = {
  0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
  0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
  0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
  0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
  0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
  0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
  0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
  0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
  0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
  0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
  0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
  0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
  0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
  0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
  0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
  0xf9, 0xfa };

static unsigned char std_ac_chrominance_nrcodes[17] = { 0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77 };
static unsigned char std_ac_chrominance_values[162] = {
  0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
  0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
  0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
  0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
  0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
  0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
  0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
  0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
  0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
  0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
  0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
  0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
  0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
  0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
  0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
  0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
  0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
  0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
  0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
  0xf9, 0xfa };

static unsigned char bytenew = 0; 	
static signed char bytepos = 7; 	
								
static unsigned short int mask[16] = { 1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768 };


static bitstring YDC_HT[12];
static bitstring CbDC_HT[12];
static bitstring YAC_HT[256];
static bitstring CbAC_HT[256];

static unsigned char *category_alloc;
static unsigned char *category; 		
static bitstring *bitcode_alloc;
static bitstring *bitcode; 				

static unsigned char *buffer; 			
unsigned short int Ximage, Yimage;		


static signed char DU_Y[256];
static signed char DU_Cb[64];
static signed char DU_Cr[64];

static signed short int DU_DCT[64]; 

static signed short int DU[64]; 	

FILE *fp_jpeg_stream;

void write_APP0info()
{
	writeword(APP0info.marker);						//标记代码
	writeword(APP0info.length);						//数据长度
	writebyte('J'); writebyte('F'); writebyte('I'); writebyte('F'); writebyte(0);
	writebyte(APP0info.versionhi);
	writebyte(APP0info.versionlo);
	writebyte(APP0info.xyunits);
	writeword(APP0info.xdensity);
	writeword(APP0info.ydensity);
	writebyte(APP0info.thumbnwidth);
	writebyte(APP0info.thumbnheight);
}

void write_SOF0info() {
	writeword(SOF0info.marker);
	writeword(SOF0info.length);
	writebyte(SOF0info.precision);
	writeword(SOF0info.height);
	writeword(SOF0info.width);
	writebyte(SOF0info.nrofcomponents);
	writebyte(SOF0info.IdY);
	writebyte(SOF0info.HVY);
	writebyte(SOF0info.QTY);
	writebyte(SOF0info.IdCb);
	writebyte(SOF0info.HVCb);
	writebyte(SOF0info.QTCb);
	writebyte(SOF0info.IdCr);
	writebyte(SOF0info.HVCr);
	writebyte(SOF0info.QTCr);
}

void write_DQTinfo() {
	unsigned char i;
	writeword(DQTinfo.marker);
	writeword(DQTinfo.length);
	writebyte(DQTinfo.QTYinfo);
	for (i = 0; i < 64; i++) writebyte(DQTinfo.Ytable[i]);
	writebyte(DQTinfo.QTCbinfo);
	for (i = 0; i < 64; i++) writebyte(DQTinfo.Cbtable[i]);
}

void set_DQTinfo() {
	int i;
	DQTinfo.marker = 0xFFDB;	//标记段0xFFDB
	DQTinfo.length = 132;		//数据长度固定为132字节
	DQTinfo.QTYinfo = 0;		//亮度分量的精度（0），ID（0）
	DQTinfo.QTCbinfo = 1;		//色度分量的精度（0），ID（1）
	for (i = 0; i < 64; i++) 
		DQTinfo.Ytable[i] = DQT_Y[i];
	for (i = 0; i < 64; i++) 
		DQTinfo.Cbtable[i] = DQT_C[i];
}

void write_DHTinfo() {
	unsigned char i;
	writeword(DHTinfo.marker);
	writeword(DHTinfo.length);
	writebyte(DHTinfo.HTYDCinfo);
	for (i = 0; i < 16; i++)  writebyte(DHTinfo.YDC_nrcodes[i]);
	for (i = 0; i <= 11; i++) writebyte(DHTinfo.YDC_values[i]);
	writebyte(DHTinfo.HTYACinfo);
	for (i = 0; i < 16; i++)  writebyte(DHTinfo.YAC_nrcodes[i]);
	for (i = 0; i <= 161; i++) writebyte(DHTinfo.YAC_values[i]);
	writebyte(DHTinfo.HTCbDCinfo);
	for (i = 0; i < 16; i++)  writebyte(DHTinfo.CbDC_nrcodes[i]);
	for (i = 0; i <= 11; i++)  writebyte(DHTinfo.CbDC_values[i]);
	writebyte(DHTinfo.HTCbACinfo);
	for (i = 0; i < 16; i++)  writebyte(DHTinfo.CbAC_nrcodes[i]);
	for (i = 0; i <= 161; i++) writebyte(DHTinfo.CbAC_values[i]);
}

void set_DHTinfo() {
	unsigned char i;
	DHTinfo.marker = 0xFFC4;
	DHTinfo.length = 0x01A2;
	DHTinfo.HTYDCinfo = 0;												//亮度直流哈夫曼表
	for (i = 0; i < 16; i++)											//不同位数的码字数量
		DHTinfo.YDC_nrcodes[i] = std_dc_luminance_nrcodes[i + 1];
	for (i = 0; i <= 11; i++)											//编码内容
		DHTinfo.YDC_values[i] = std_dc_luminance_values[i];

	DHTinfo.HTYACinfo = 0x10;											
	for (i = 0; i < 16; i++)  
		DHTinfo.YAC_nrcodes[i] = std_ac_luminance_nrcodes[i + 1];
	for (i = 0; i <= 161; i++) 
		DHTinfo.YAC_values[i] = std_ac_luminance_values[i];

	DHTinfo.HTCbDCinfo = 1;
	for (i = 0; i < 16; i++)  
		DHTinfo.CbDC_nrcodes[i] = std_dc_chrominance_nrcodes[i + 1];
	for (i = 0; i <= 11; i++)  
		DHTinfo.CbDC_values[i] = std_dc_chrominance_values[i];

	DHTinfo.HTCbACinfo = 0x11;
	for (i = 0; i < 16; i++)  
		DHTinfo.CbAC_nrcodes[i] = std_ac_chrominance_nrcodes[i + 1];
	for (i = 0; i <= 161; i++) 
		DHTinfo.CbAC_values[i] = std_ac_chrominance_values[i];
}

void write_SOSinfo() {
	writeword(SOSinfo.marker);
	writeword(SOSinfo.length);
	writebyte(SOSinfo.nrofcomponents);
	writebyte(SOSinfo.IdY);
	writebyte(SOSinfo.HTY);
	writebyte(SOSinfo.IdCb);
	writebyte(SOSinfo.HTCb);
	writebyte(SOSinfo.IdCr);
	writebyte(SOSinfo.HTCr);
	writebyte(SOSinfo.Ss);
	writebyte(SOSinfo.Se);
	writebyte(SOSinfo.Bf);
}

void writebits(bitstring bs)
{
	unsigned short int value;
	signed char posval;
	value = bs.value;				//获得码字		
	posval = bs.length - 1;			//编码的长度（1-16）
	while (posval >= 0)
	{
		if (value & mask[posval]) 
			bytenew |= mask[bytepos];
		posval--;
		bytepos--;
		if (bytepos < 0)
		{
			if (bytenew == 0xFF)
			{
				writebyte(0xFF);	//数据若为0xFF需要再写入一个0x00以作区分
				writebyte(0);
			}
			else
			{
				writebyte(bytenew);
			}
			bytepos = 7;
			bytenew = 0;
		}
	}
}

void compute_Huffman_table(unsigned char *nrcodes, unsigned char *std_table, bitstring *HT)
{
	unsigned char k, j;
	unsigned char pos_in_table;
	unsigned short int codevalue;
	codevalue = 0; pos_in_table = 0;
	for (k = 1; k <= 16; k++)
	{
		for (j = 1; j <= nrcodes[k]; j++)					//不同位数的编码数量
		{
			HT[std_table[pos_in_table]].value = codevalue;	//存储码字
			HT[std_table[pos_in_table]].length = k;			//编码的长度
			pos_in_table++;									//位置加加
			codevalue++;									//值加加
		}
		codevalue *= 2;										//长度加1
	}
}
void init_Huffman_tables()
{
	compute_Huffman_table(std_dc_luminance_nrcodes, std_dc_luminance_values, YDC_HT);			//获得亮度直流Huffman表
	compute_Huffman_table(std_dc_chrominance_nrcodes, std_dc_chrominance_values, CbDC_HT);		//获得色度直流Huffman表
	compute_Huffman_table(std_ac_luminance_nrcodes, std_ac_luminance_values, YAC_HT);			//获得交流亮度Huffman表	
	compute_Huffman_table(std_ac_chrominance_nrcodes, std_ac_chrominance_values, CbAC_HT);		// Huffman表
}

void set_numbers_category_and_bitcode()
{
	signed long int nr;
	signed long int nrlower, nrupper;
	unsigned char cat, value;

	category_alloc = (unsigned char *)malloc(65535 * sizeof(unsigned char));//对类别分配内存
	category = category_alloc + 32767; 										//允许复数下标
	bitcode_alloc = (bitstring *)malloc(65535 * sizeof(bitstring));			//对码值分配内存
	bitcode = bitcode_alloc + 32767;		//允许数组负数下标
	nrlower = 1;
	nrupper = 2;
	for (cat = 1; cat <= 15; cat++)
	{
		//设置正数的类别及编码值
		for (nr = nrlower; nr < nrupper; nr++)
		{
			category[nr] = cat;											//nr便是实际的数值，cat表示编码的长度
			bitcode[nr].length = cat;									//码字的长度
			bitcode[nr].value = (unsigned short int)nr;					//值对应的编码
		}	
		//设置负数的类别及编码值
		for (nr = -(nrupper - 1); nr <= -nrlower; nr++)
		{
			category[nr] = cat;								
			bitcode[nr].length = cat;
			bitcode[nr].value = (unsigned short int)(nrupper - 1 + nr);
		}
		nrlower <<= 1;
		nrupper <<= 1;
	}
}

void DCT(signed char *data, unsigned char *fdtbl, signed short int *outdata)
{
//量化表系数=1/(原量化系数*行aa&n因子*列aa&n因子*8)
	double aanscalefactor[8] = { 1.0, 1.387039845, 1.306562965, 1.175875602,1.0, 0.785694958, 0.541196100, 0.275899379 };
	float tmp0,  tmp1,  tmp2,  tmp3,  tmp4,  tmp5,  tmp6,  tmp7;
	float tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, tmp16, tmp17, tmp18, tmp19;
	float z1, z2, z3, z4, z5, z11, z13;
	float *dataptr;
	float datafloat[64];
	float temp;
	signed char ctr;
	unsigned char i;
	for (i = 0; i < 64; i++)
		datafloat[i] = data[i];

	dataptr = datafloat;
	for (ctr = 7; ctr >= 0; ctr--)
	{
		tmp0 = dataptr[0] + dataptr[7];
		tmp1 = dataptr[1] + dataptr[6];
		tmp2 = dataptr[2] + dataptr[5];
		tmp3 = dataptr[3] + dataptr[4];
		
		tmp4 = dataptr[3] - dataptr[4];		
		tmp5 = dataptr[2] - dataptr[5];		
		tmp6 = dataptr[1] - dataptr[6];		
		tmp7 = dataptr[0] - dataptr[7];			

		// Phase 2
		tmp10 = tmp0 + tmp3;		
		tmp11 = tmp1 + tmp2;		
		tmp12 = tmp1 - tmp2; 		
		tmp13 = tmp0 - tmp3;		
		
		
		
		tmp14 = (tmp4 + tmp5) * ((float) 0.923879533);		//PI/8
		tmp15 = (tmp4 + tmp5) * ((float) 0.382683432);		
		tmp16 = (tmp6 + tmp7) * ((float) 0.382683432);
		tmp17 = (tmp6 + tmp7) * ((float) 0.923879533);
		tmp18 = tmp7;
		tmp19 = (tmp5 + tmp6) * ((float) 0.707106781);		//sqrt(2)/2

		// Phase 3
		dataptr[0] = (tmp10  + tmp11) / aanscalefactor[0];
		dataptr[4] = (tmp10  - tmp11) / aanscalefactor[4];
		z1 = (tmp12 + tmp13) * ((float) 0.707106781);		//sqrt(2)/2
		z2 = tmp14  - tmp16;
		z4 = tmp15  + tmp17;
		z11 = tmp18 + tmp19;
		z13 = tmp18 - tmp19;

		// Phase 4
		dataptr[2] = (tmp13 + z1)   / aanscalefactor[2];
		dataptr[6] = (tmp13 - z1)   / aanscalefactor[6];
		dataptr[5] = (z13   + z2)   / aanscalefactor[5];
		dataptr[3] = (z13   - z2)   / aanscalefactor[3];
		dataptr[1] = (z11   + z4)   / aanscalefactor[1];
		dataptr[7] = (z11   - z4)   / aanscalefactor[7];

		dataptr += 8;
	}



	dataptr -= 64;
	for (ctr = 7; ctr >= 0; ctr--)
	{
		tmp0 = dataptr[0]  + dataptr[56];
		tmp7 = dataptr[0]  - dataptr[56];
		tmp1 = dataptr[8]  + dataptr[48];
		tmp6 = dataptr[8]  - dataptr[48];
		tmp2 = dataptr[16] + dataptr[40];
		tmp5 = dataptr[16] - dataptr[40];
		tmp3 = dataptr[24] + dataptr[32];
		tmp4 = dataptr[24] - dataptr[32];

		// Phase 2
		tmp10 = tmp0  + tmp3;
		tmp13 = tmp0  - tmp3;
		tmp11 = tmp1  + tmp2;
		tmp12 = tmp1  - tmp2;
		tmp14 = (tmp4 + tmp5) * ((float) 0.923879533);
		tmp15 = (tmp4 + tmp5) * ((float) 0.382683432);
		tmp16 = (tmp6 + tmp7) * ((float) 0.382683432);
		tmp17 = (tmp6 + tmp7) * ((float) 0.923879533);
		tmp18 = tmp7;
		tmp19 = (tmp5 + tmp6) * ((float) 0.707106781);

		// Phase 3
		dataptr[0] = (tmp10 + tmp11)  / aanscalefactor[0];
		dataptr[32] = (tmp10 - tmp11) / aanscalefactor[1];
		z1 = (tmp12 + tmp13) * ((float) 0.707106781);
		z2 = tmp14  - tmp16;
		z4 = tmp15  + tmp17;
		z11 = tmp18 + tmp19;
		z13 = tmp18 - tmp19;

		// Phase 4
		dataptr[16] = (tmp13 + z1) / aanscalefactor[2];
		dataptr[48] = (tmp13 - z1) / aanscalefactor[6];
		dataptr[40] = (z13 + z2)   / aanscalefactor[5];
		dataptr[24] = (z13 - z2)   / aanscalefactor[3];
		dataptr[8]  = (z11 + z4)   / aanscalefactor[1];
		dataptr[56] = (z11 - z4)   / aanscalefactor[7];

		dataptr++;
	}

	// DQT
	for (i = 0; i < 64; i++)
	{
		temp = datafloat[i] / (float)((double)fdtbl[zigzag[i]] * 8.0);					//量化

		outdata[i] = (signed short int) ((signed short int)(temp + 16384.5) - 16384);
	}
}

void process_DU(signed char *ComponentDU, unsigned char *fdtbl, signed short int *DC, bitstring *HTDC, bitstring *HTAC)
{
	unsigned char i;
	unsigned char startpos;
	unsigned char end0pos;
	unsigned char nrzeroes;
	unsigned char nrmarker;
	signed short int Diff;

	DCT(ComponentDU, fdtbl, DU_DCT);					//DCT和量化后的输出矩阵	

	// 之字型扫描
	for (i = 0; i <= 63; i++) 
		DU[zigzag[i]] = DU_DCT[i];

	Diff = DU[0] - *DC;									//直流分量采用DPCM编码
	*DC = DU[0];

	if (Diff == 0)
		writebits(HTDC[0]); 						
	else
	{
		writebits(HTDC[category[Diff]]);				//category[Diff]，求得码字的长度即为huffman编码的值，HTDC[category[Diff]]表示huffman编码的值对应的码字和码字长度
		writebits(bitcode[Diff]);						//再写数据
	}
	for (end0pos = 63; (end0pos > 0) && (DU[end0pos] == 0); end0pos--);	//检测到非0数据跳出循环
	if (end0pos == 0)
	{
		writebits(HTAC[0x00]);							//EOB
		return;
	}

	i = 1;
	while (i <= end0pos) 
	{
		startpos = i;
		for (;(DU[i] == 0) && (i <= end0pos); i++);
		nrzeroes = i - startpos;										//获得前置0的个数
		if (nrzeroes >= 16) 
		{
			
			for (nrmarker = 1; nrmarker <= nrzeroes / 16; nrmarker++) 
				writebits(HTAC[0xF0]);									//说明中间有16个连0
			nrzeroes = nrzeroes % 16;
		}
		writebits(HTAC[nrzeroes * 16 + category[DU[i]]]);				//写码字
		writebits(bitcode[DU[i]]);										//写值
		i++;
	}
	if (end0pos != 63) 													//写入EOB表示一个块结束
		writebits(HTAC[0x00]); 
}

void RGB2YCbCr(unsigned short int xpos, unsigned short int ypos)
{
	unsigned char x, y;
	unsigned char R, G, B;
	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			R = buffer[ypos * Ximage * 3 + xpos * 3 + y * Ximage * 3 + x * 3 + 2];
			G = buffer[ypos * Ximage * 3 + xpos * 3 + y * Ximage * 3 + x * 3 + 1];
			B = buffer[ypos * Ximage * 3 + xpos * 3 + y * Ximage * 3 + x * 3 + 0];
			DU_Y[y * 16 + x] = (unsigned char)(0.299 *R + 0.587 *G + 0.114 *B) - 128;				//亮度分量 
			if ((x % 2 == 0) & (y % 2 == 0)) 
			{
				DU_Cb[y / 2 * 8 + x / 2] = (unsigned char)(0.5 *B - 0.33126 *G - 0.16874 *R);		//色差分量Cb
				DU_Cr[y / 2 * 8 + x / 2] = (unsigned char)(0.5 *R - 0.41869 *G - 0.08131 *B);		//色差分量Cr
			}
		}
	}
}

void main_encoder()
{
	signed short int DCY = 0, DCCb = 0, DCCr = 0;						//DC系数用来做差分编码
	unsigned short int xpos, ypos;
	signed char DU[64];
	int i;
	for (ypos = 0; ypos < Yimage; ypos += 16) 
	{
		for (xpos = 0; xpos < Ximage; xpos += 16) 
		{
			RGB2YCbCr(xpos, ypos);
			for (i = 0; i < 64; i++) 									//获得MCU左上角的Y分量矩阵
				DU[i] = DU_Y[(i / 8) * 16 + i % 8 + 0];
			process_DU(DU, DQTinfo.Ytable, &DCY, YDC_HT, YAC_HT);		//第一个参数为数据值，第二个参数为亮度的量化表，第四个参数为亮度直流分量的huffman表，第四个参数为亮度交流分量的huffman表
			for (i = 0; i < 64; i++) 
				DU[i] = DU_Y[(i / 8) * 16 + i % 8 + 8];
			process_DU(DU, DQTinfo.Ytable, &DCY, YDC_HT, YAC_HT);
			for (i = 0; i < 64; i++) 
				DU[i] = DU_Y[(i / 8) * 16 + i % 8 + 128];
			process_DU(DU, DQTinfo.Ytable, &DCY, YDC_HT, YAC_HT);
			for (i = 0; i < 64; i++) 
				DU[i] = DU_Y[(i / 8) * 16 + i % 8 + 136];
			process_DU(DU, DQTinfo.Ytable, &DCY, YDC_HT, YAC_HT);

			process_DU(DU_Cb, DQTinfo.Cbtable, &DCCb, CbDC_HT, CbAC_HT);
			process_DU(DU_Cr, DQTinfo.Cbtable, &DCCr, CbDC_HT, CbAC_HT);
		}
	}
}

void load_bitmap(char *bitmap_name, unsigned short int *Ximage_original, unsigned short int *Yimage_original)
{
	unsigned short int Xdiv8, Ydiv8;
	unsigned char nr_fillingbytes;								
																
	unsigned char lastcolor[3];
	unsigned short int column;
	unsigned char TMPBUF[256];
	unsigned short int nrline_up, nrline_dn, nrline;
	unsigned short int dimline;
	unsigned char *tmpline;
	FILE *fp_bitmap = fopen("1.jpeg", "rb");					//打开bmp文件
	fread(TMPBUF, 1, 54, fp_bitmap);							//读入位图的bmp文件头(bmp file header)、位图信息头(bitmap information)
	Ximage = (unsigned short int)TMPBUF[19] * 256 + TMPBUF[18];	//获得图像的宽度
	Yimage = (unsigned short int)TMPBUF[23] * 256 + TMPBUF[22];	//获得图像的高度
	*Ximage_original = Ximage;
	*Yimage_original = Yimage; 							
	if (Ximage % 16 != 0)										//保证长宽均为MCU宽度的整数倍
		Xdiv8 = (Ximage / 16) * 16 + 16;
	else
		Xdiv8 = Ximage;
	if (Yimage % 16 != 0)
		Ydiv8 = (Yimage / 16) * 16 + 16;
	else
		Ydiv8 = Yimage;

	buffer = (unsigned char *)(malloc(3 * Xdiv8 * Ydiv8));			//24位位图一个像素占24个字节
	if (Ximage % 4 != 0) 											//读取文件数一行字节数必须是4的倍数，如果不够四字节要补齐四字节
		nr_fillingbytes = 4 - (Ximage % 4);							//填充的字节数
	else
		nr_fillingbytes = 0;
	for (nrline = 0; nrline < Yimage; nrline++)						//读出每一行数据并在其后填充以满足与16求余为0
	{
		fread(buffer + nrline * Xdiv8 * 3, 1, Ximage * 3, fp_bitmap);			//读取图像一行的数据
		fread(TMPBUF, 1, nr_fillingbytes, fp_bitmap);							//读取填充的字节数
		memcpy(&lastcolor, buffer + nrline * Xdiv8 * 3 + Ximage * 3 - 3, 3);	//读出上一像素的RGB值用于扩展
		for (column = Ximage; column < Xdiv8; column++)
		{
			memcpy(buffer + nrline * Xdiv8 * 3 + column * 3, &lastcolor, 3);	//扩展列
		}
	}
	Ximage = Xdiv8;																//扩展后的图像宽度
	dimline = Ximage * 3;														//扩展后每一行所包含的字节数
	tmpline = (unsigned char *)malloc(dimline);

	//位图信息头的图像高度是正数，所以位图数据在文件中的排列顺序是从左下角到右上角的，以行为主序排列的
	for (nrline_up = Yimage - 1, nrline_dn = 0; nrline_up > nrline_dn; nrline_up--, nrline_dn++)
	{
		memcpy(tmpline, buffer + nrline_up * Ximage * 3, dimline);
		memcpy(buffer + nrline_up * Ximage * 3, buffer + nrline_dn * Ximage * 3, dimline);
		memcpy(buffer + nrline_dn * Ximage * 3, tmpline, dimline);
	}

	//Y completion:
	memcpy(tmpline, buffer + (Yimage - 1) * Ximage * 3, dimline);		
	for (nrline = Yimage; nrline < Ydiv8; nrline++)
	{
		memcpy(buffer + nrline * Ximage * 3, tmpline, dimline);
	}

	Yimage = Ydiv8;				//扩展后的图像长宽
	free(tmpline);				//释放内存空间
	fclose(fp_bitmap);			//关闭bmp图像文件
}

void init_all()
{
	set_DQTinfo();						//设置量化表信息
	set_DHTinfo();						//设置哈夫曼表信息
	init_Huffman_tables();				//初始化哈夫曼表
	set_numbers_category_and_bitcode();	//设置数值的类别（编码位数及编码值）
}