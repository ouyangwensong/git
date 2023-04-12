#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

int Exist(char* path); //判断文件/目录是否存在
int IsFolder(char* path); //判断是否为目录
int FileCopy(char* src, char* des); //复制文件
int FileCopyToFolder(char* file, char* folder); //复制文件到目录
int CreateFolder(char *folder); //创建文件夹
int FolderCopy(char* src, char* des); //直接复制目录
int FolderCopyToFolder(char* src, char* des); //复制目录到目录

/*函数名称：//判断文件/目录是否存在
//调用函数：fopen,fclose
//参数：路径名
返回值：0=不存在;1=存在*/
int Exist(char* path)
{
    FILE *fp; //文件指针(也能打开目录)

    //用读文件方式尝试打开路径名
    if((fp = fopen(path,"r")) == NULL) //打开失败，文件/目录不存在
        return 0;
    else //文件/目录存在
    {
        fclose(fp); //关闭文件指针
        return 1;
    }
}

/*函数名称：判断是否为目录
//调用函数：opendir,closedir
//参数：目录路径名
//返回值：0=不是目录;1=是目录*/
int IsFolder(char* path)
{
    DIR *dp; //目录指针

    //尝试以目录方式打开路径名
    if((dp = opendir(path)) == NULL) //打开失败，不是目录
        return 0;
    else //打开成功，是目录
    {
        closedir(dp); //关闭目录指针
        return 1;
    }
}

//函数名称：复制文件
//调用函数：fopen,fclose,fgetc,fputc,feof
//参数：源文件路径名，目标文件路径名
//返回值：-1=源文件打开失败;-2=目标文件打开失败;0=复制成功
int FileCopy(char* src, char* des)
{
    FILE *input,*output; //输入/输出文件指针
    char ch; //临时读取字符

    //尝试以只读方式打开源文件
    if((input = fopen(src,"r")) == NULL) //文件打开失败
    {
        printf("'%s' does not exist!\n", src);
        return -1;
    }
    //尝试以只写方式打开目标文件
    if((output = fopen(des,"w")) == NULL) //文件打开失败
    {
        fclose(input); //关闭文件指针
        printf("Wrong output file name or Permission denied!\n");
        return -2;
    }

    ch = fgetc(input); //尝试从源文件中读入一个字符
    while(!feof(input)) //没到结尾，继续循环
    {
        fputc(ch,output); //将字符输出到目标文件
        ch = fgetc(input); //从源文件读入下一个字符
    }

    //关闭文件指针
    fclose(input);
    fclose(output);

    return 0;
}

//函数名称：复制文件到目录
//调用函数：strlen,strncpy,strcpy,FileCopy
//参数：源文件路径名，目标目录路径名
//返回值：-1=源文件打开失败;-2=目标文件打开失败;0=复制成功
int FileCopyToFolder(char* file, char* folder)
{
    char fileName[500]; //文件名称
    char fileDest[500]; //目标文件路径
    int i; //循环变量
    int Length = strlen(file); //获得源文件路径名的长度

    //从后往前循环遍历源文件路径名
    for(i = Length - 1; i >= 0; i--)
    {
        if(file[i] == '/') //如果遇到斜杠
        {
            //提取斜杠后的文件名
            strncpy(fileName, file + i + 1, Length - i);
            break;
        }
    }
    if(i <= -1) strcpy(fileName, file); //如果没有斜杠，路径名就是文件名

    //构造目标文件路径名
    strcpy(fileDest, folder);
    strcat(fileDest, "/");
    strcat(fileDest, fileName);

    return FileCopy(file, fileDest); //调用文件复制函数
}

//函数名称：创建目录
//调用函数：strcpy,strlen,strcat,access,mkdir,IsFolder
//参数：要创建的目录路径名
//返回值：-1=创建失败;-2=同名文件已存在;0=创建成功
int CreateFolder(char *folder)
{
    char folderPath[500]; //目录路径名
    int i; //循环变量
    int len; //目录路径名长度

    strcpy(folderPath, folder); //拷贝目录路径名
    len = strlen(folderPath); //获得路径名长度

    //确保路径名最后字符为斜杠
    if(folderPath[len - 1] != '/')
    {
        strcat(folderPath, "/");
        len++;
    }
    //循环遍历路径名
    for(i = 1; i < len; i++)
    {
        if(folderPath[i] == '/') //遇到斜杠
        {
            folderPath[i] = '\0'; //将斜杠替换为结束符
            //尝试创建目录
            if(access(folderPath, F_OK) < 0) //目录不存在
            {
                if(mkdir(folderPath, 0755) < 0) //创建失败
                {
                    printf("mkdir '%s' error!\nWrong folder name or Permission denied!\n", folderPath);
                    return -1;
                }
            }
            else if(!IsFolder(folderPath)) //同名文件已存在
            {
                printf("cannot create directory ‘%s’,File exists!\n", folder);
                return -2;
            }
            folderPath[i] = '/'; //替换回斜杠
        }
    }

    return 0;
}

//函数名称：直接复制目录
//        直接把源目录下的文件和目录复制到到目标目录下
//调用函数：opendir,CreateFolder,readdir,strcpy,strcat,strcmp,FileCopy
//参数：源目录路径名，目标目录路径名
//返回值：-1=源目录打开失败;-2=目标目录创建失败;0=复制成功
int FolderCopy(char* src, char* des)
{
    DIR *dp; //目录指针
    struct dirent *entry; //读取目录返回的结构体
    char srcInside[500]; //源目录/文件路径名
    char desInside[500]; //目标目录/文件路径名

    //源目录打开失败
    if((dp = opendir(src))  ==  NULL)
    {
        printf("%s is not a folder!\n", src);
        return -1;
    }
    //目标目录创建失败，目标目录已存在则忽略
    if(CreateFolder(des) != 0) return -2;
    //循环读取目录下所有条目信息
    while((entry = readdir(dp)) != NULL)
    {
        //创建对应条目的源/目标路径名
        strcpy(srcInside, src);
        strcpy(desInside, des);
        strcat(srcInside, "/");
        strcat(desInside, "/");
        strcat(srcInside, entry->d_name);
        strcat(desInside, entry->d_name);
        //如果条目是目录
        if(IsFolder(srcInside))
        {
            //忽略.和..目录
            if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
            //递归调用直接复制目录函数
            FolderCopy(srcInside, desInside);
        }
        else //如果是文件
        {
            FileCopy(srcInside, desInside); //调用文件复制函数
        }
    }

    return 0;
}

//函数名称：复制目录到目录
//         将源目录直接复制到目标目录下
//调用函数：strlen,strncpy,strcpy,strcat,FolderCopy
//参数：源目录路径名，目标目录路径名
//返回值：-1=源目录打开失败;-2=目标目录创建失败;0=复制成功
int FolderCopyToFolder(char* src, char* des)
{
    char folderName[500]; //源目录名称
    char folderDesPath[500]; //目标目录路径名
    int i; //循环变量
    int Length = strlen(src); //获得源文件路径名的长度

    //从后往前循环遍历源文件路径名
    for(i = Length - 2; i >= 0; i--) //从倒数第二个字符开始遍历
    {
        if(src[i] == '/') //如果遇到斜杠
        {
            //提取斜杠后的目录名
            strncpy(folderName, src + i + 1, Length - i);
            break;
        }
    }
    if(i <= -1) strcpy(folderName, src); //如果没有斜杠，路径名就是目录名

    //复制目标目录路径名
    strcpy(folderDesPath, des);
    //确认目标目录路径名以斜杠结尾
    if(folderDesPath[strlen(folderDesPath) - 1] != '/') strcat(folderDesPath, "/");
    //创建新的目标目录路径名
    strcat(folderDesPath, folderName);

    //调用直接复制目录函数
    return FolderCopy(src, folderDesPath);
}


int main(int argc, char* argv[])
{
    int i; //循环变量

    //根据参数执行不同的操作
    if(argc < 3) //参数不够
    {
        printf("Not enough input arguments!\n");
        return -1;
    }
    else if(argc == 3) //只有两个参数
    {
        if(!Exist(argv[1])) //源文件/目录不存在
        {
            printf("'%s' does not exist!\n",argv[1]);
            return -2;
        }
        else if(IsFolder(argv[1]) && !IsFolder(argv[2])) //直接复制源目录，制作目录副本
        {
            return FolderCopy(argv[1], argv[2]);
        }
        else if(IsFolder(argv[1]) && IsFolder(argv[2])) //将源目录复制到目标目录下
        {
            return FolderCopyToFolder(argv[1], argv[2]);
        }
        else if(!IsFolder(argv[2])) //直接复制源文件，制作文件副本
        {
            return FileCopy(argv[1], argv[2]);
        }
        else if(IsFolder(argv[2])) //将源文件复制到目标目录下
        {
            return FileCopyToFolder(argv[1], argv[2]);
        }
    }
    else //多个源文件/目录参数
    {
        if(!IsFolder(argv[argc - 1])) //目标目录不存在
        {
            printf("'%s' is not a folder!\n", argv[argc - 1]);
            return -3;
        }
        //遍历各个源文件/目录参数
        for(i = 1; i < argc - 1; i++)
        {
            if(!Exist(argv[i])) //源文件/目录不存在
            {
                printf("'%s' does not exist!\n", argv[i]);
            }
            else if(IsFolder(argv[i])) //复制目录
            {
                FolderCopyToFolder(argv[i], argv[argc - 1]);
            }
            else //复制文件
            {
                FileCopyToFolder(argv[i], argv[argc - 1]);
            }
        }
    }

    return 0;
}
