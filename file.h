#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

//文件描述符，每个占32字节
struct file_des {
	int len;        //文件长度
	int num[7];     //文件分配到的磁盘块号数组
	file_des() {
		len = 0;
		for (int i = 0; i < 7; i++)
			num[i] = -1;
	}
};
//目录项，每个占32字节
struct dir {
	char name[28];  //文件名
	int index;      //文件描述符序号
};
//打开文件表
struct file_open {
	char buf[B];
	int p, index;
};

int create(char *filename);     //根据指定文件名创建新文件
int destory(char *filename);   //删除指定文件
int open(char *filename);       //打开文件
int close(int index);          //关闭指定文件
int read(int index, char *mem_area, int count);      //从指定文件顺序读入count个字节mem_area指定的内存位置
int write(int index, char *mem_area, int count);     //把mem_area指定的内存位置开始的count个字节顺序写入指定文件
int lseek(int index, int pos);      //把文件的读写指针移动到 pos 指定的位置
int directory();                   //列表显示所有文件及其长度
int directory_open();

#endif // FILE_H_INCLUDED
