#include "../inc/audiolink.h"


int main()
{
	AUDIOLINK_S *psl = calloc(1,sizeof(AUDIOLINK_S));
	int ierr = 0;
	NK_FREQ_Init(psl);
	NK_AUDIO_Record(psl);

	if (ierr = NK_FREQ_TO_Char(psl) == -1)
	{
		printf("No data !\n");
		return 0;
	}

	printf("wifi name :\n");
	for (int i = 0;i < psl->m_namelen;i++)
	{
		printf("%c",psl->m_name[i]);
	}
    printf("\n\n");
	printf("wifi password :\n");

	for (int i = 0;i < psl->m_passlen;i++)
	{
		printf("%c",psl->m_pass[i]);
	}
    printf("\n\n");
	free(psl);
    return 0;
}