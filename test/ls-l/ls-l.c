#include<grp.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<pwd.h>
#include<dirent.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
void  do_ls( char  []);
void  dostat( char  *);
void  show_file_info( char  *, struct  stat *);
 
int  main( int  argc, char  *argv[])
{
         if (argc == 1)
                 do_ls( "." );
         else
/*
  *依次遍历每一个参数，打印目录下的所有文件
  */
                 while (--argc){
                         printf ( "%s:\n" ,*++argv);
                         do_ls(*argv);
                 }
}
 
void  do_ls( char  dirname[])
{
/*
  *定义一个目录流,和目录流结构体保存读到的结果。
  */
         DIR *dir_ptr;
         struct  dirent *direntp;
         if ((dir_ptr = opendir(dirname)) == NULL)
                 fprintf (stderr, "ls1:cannot open %s\n" ,dirname);
         else
         {
                 while ((direntp = readdir(dir_ptr)) != NULL)
                         //打印结果
                         dostat(direntp->d_name);
 
                //关闭目录流
                 closedir(dir_ptr);
         }
}
 
//获取文件信息stat结构体
void  dostat( char  *filename)
{
         struct  stat info;
         if (stat(filename,&info) == -1)
                 perror (filename);
         else
                //分析stat结构体
                 show_file_info(filename,&info);
}
 
void  show_file_info( char  *filename, struct  stat *info_p)
{
         char * ctime (),*filemode();
         char  modestr[11];
         printf ( "%8ld" ,( long )info_p->st_size);
         printf ( "%.12s" ,4+ ctime (&info_p->st_mtime));
         printf ( " %s\n" ,filename);
}
 