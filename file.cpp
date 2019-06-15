#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include <string.h>
#include "io.h"
#include "file.h"

using namespace std;

file_open fop[MAXN];
int number = 0;

int char_to_int(char *mem) {
	int ans = 0;
	ans += ((unsigned char)mem[0]) << 24;
	ans += ((unsigned char)mem[1]) << 16;
	ans += ((unsigned char)mem[2]) << 8;
	ans += ((unsigned char)mem[3]);
	return ans;
}
void int_to_char(int num, char *mem) {
	mem[0] = (num & 0xFF000000) >> 24;
	mem[1] = (num & 0x00FF0000) >> 16;
	mem[2] = (num & 0x0000FF00) >> 8;
	mem[3] = (num & 0x000000FF);
}
//申请空间
int get_space() {
	char mem[B];
	read_block(0, mem);
	for (int i = 8; i < B; i++) {
		char c = mem[i];
		for (int j = 7; j >= 0; j--) {
			if ((((int)c)&(1 << j)) == 0) {
				mem[i] = ((unsigned char)mem[i]) | (1 << j);
				write_block(0, mem);
				return 8 * i + 7 - j;
			}
		}
	}
	return -1;
}
//添加目录
void dir_add(char *filename, int index) {
	char mem[B];
	read_block(1, mem);
	file_des f;
	f.len = char_to_int(mem);
	for (int i = 0; i < 7; i++)
		f.num[i] = char_to_int(mem + (i + 1) * 4);
	bool flag = false;
	int i;
	for (i = 0; f.num[i] != -1; i++) {
		read_block(f.num[i], mem);
		for (int j = 0; j < B; j += 32) {
			int len = char_to_int(mem + j);
			if (len == -1) {
				strcpy(mem + j, filename);
				int_to_char(index, mem + j + 28);
				write_block(f.num[i], mem);
				flag = true;
				break;
			}
		}
		if (flag)break;
	}
	if (flag == false) {
		f.num[i] = get_space();
		read_block(f.num[f.len], mem);
		strcpy(mem, filename);
		int_to_char(index, mem + 28);
		write_block(f.num[f.len], mem);
		flag = true;
	}
	f.len++;
	read_block(1, mem);
	int_to_char(f.len, mem);
	for (int i = 0; i < 7; i++)
		int_to_char(f.num[i], mem + (i + 1) * 4);
	write_block(1, mem);
}
//创建文件
int create(char *filename) {
	char mem[B];
	int index = -1;
	for (int i = 1; i < 64; i++) {
		read_block(i, mem);
		for (int j = 0; j < B; j += 32) {
			if (i == 1 && j == 0)
				continue;
			int len = char_to_int(mem + j);	//查找可用文件描述符
			if (len == -1) {
				index = (i - 1) * 16 + j / 32;
				dir_add(filename, index);	//添加目录
				read_block(i, mem);
				file_des f;
				f.len = 0;
				f.num[0] = get_space();		//申请空间
				int_to_char(f.len, mem + j);
				for (int k = 0; k < 7; k++) {
					int_to_char(f.num[k], mem + j + 4 * (k + 1));
				}
				write_block(i, mem);			//写回磁盘
				break;
			}
		}
		if (index != -1) {
			write_block(i, mem);
			break;
		}
	}
	return index;
}
//删除磁盘
void des_disk(int num) {
	char mem[B];
	read_block(0, mem);
	int p = num / 8;
	int q = num % 8;
	mem[p] = ((unsigned char)mem[p]) ^ (1 << (7 - q));
	write_block(0, mem);
	read_block(num, mem);
	memset(mem, 0, sizeof(mem));
	write_block(num, mem);
}

bool cmp(char *mem, char *name) {
	for (int i = 0; i < 28; i++) {
		if (*(name + i) == '\0')
			return true;
		if (*(mem + i) != *(name + i))
			return false;
	}
	return true;
}
//删除文件
int destory(char *filename) {
	char mem[B], mem1[B];
	read_block(1, mem);
	file_des f;
	f.len = char_to_int(mem);
	for (int i = 0; i < 7; i++)
		f.num[i] = char_to_int(mem + (i + 1) * 4);
	bool flag = true;
	int index;
	for (int i = 0; f.num[i] != -1 && flag; i++) {
		read_block(f.num[i], mem);
		for (int j = 0; j < B; j += 32) {
			if (cmp(mem + j, filename)) {
				index = char_to_int(mem + j + 28);
				memset(mem + j, -1, 32);            //删除目录项
				write_block(f.num[i], mem);
				int p = index / 16 + 1;
				int q = (index % 16) * 32;
				read_block(p, mem1);
				file_des ff;
				ff.len = char_to_int(mem1 + q);
				for (int k = 0; k < 7; k++)
					ff.num[k] = char_to_int(mem1 + q + (k + 1) * 4);
				for (int k = 0; ff.num[k] != -1; k++)
					des_disk(ff.num[k]);        //删除文件
				memset(mem1 + q, -1, 32);
				write_block(p, mem1);            //删除文件描述符
				flag = false;
				return 1;
			}
		}
	}
	return -1;
}
//打开文件
int open(char *filename) {
	char mem[B], mem1[B];
	read_block(1, mem);
	file_des f;
	f.len = char_to_int(mem);
	for (int i = 0; i < 7; i++)
		f.num[i] = char_to_int(mem + (i + 1) * 4);    //恢复目录
	for (int i = 0; f.num[i] != -1; i++) {
		read_block(f.num[i], mem);
		for (int j = 0; j < B; j += 32) {
			if (cmp(mem + j, filename)) {
				int index = char_to_int(mem + j + 28);  //文件描述符序号
				int p = index / 16 + 1;
				int q = (index % 16) * 32;
				read_block(p, mem1);
				int x = char_to_int(mem1 + q + 4);
				fop[number].index = index;
				fop[number].p = 0;
				read_block(x, fop[number].buf);      //将文件第一块磁盘读入缓冲
				number++;
				return index;
			}
		}
	}
	return -1;
}
//关闭文件
int close(int index) {
	char mem[B];
	for (int i = 0; i < number; i++) {
		if (fop[i].index == index) {
			int p = index / 16 + 1;
			int q = (index % 16) * 32;
			read_block(p, mem);
			file_des f;                             //恢复文件描述符
			f.len = char_to_int(mem + q);
			for (int k = 0; k < 7; k++)
				f.num[k] = char_to_int(mem + q + (k + 1) * 4);
			int pp = fop[i].p / B;
			write_block(f.num[pp], fop[i].buf);      //缓冲写回

			for (int j = i; j < number - 1; j++) {      //更新打开文件表
				fop[j] = fop[j + 1];
			}
			number--;
			return i;
		}
	}
	return -1;
}
//读文件
int read(int index, char *mem_area, int count) {
	char mem[B];
	int p = index / 16 + 1;
	int q = (index % 16) * 32;
	read_block(p, mem);
	file_des f;
	f.len = char_to_int(mem + q);
	for (int k = 0; k < 7; k++)
		f.num[k] = char_to_int(mem + q + (k + 1) * 4);      //恢复文件描述符
	int id = 0;
	for (int i = 0; i < number; i++) {
		if (fop[i].index == index) {
			int p = fop[i].p / B;
			int q = fop[i].p%B;
			while (count) {
				//read_block(f.num[p],mem);
				int x = min(B - q, count);
				for (int k = 0; k < x; k++) {
					*(mem_area + id) = *(fop[i].buf + q + k);
					id++;
				}
				fop[i].p += q;
				if (fop[i].p == B) {              //打开下一块磁盘
					q = 0;
					p++;
					if (p>7)
						return -2;
					if (f.num[p] == -1)
						return -2;
					read_block(f.num[p], fop[number].buf);
				}
				count -= x;
			}
			return 1;
		}
	}
	return 0;
}
//写文件
int write(int index, char *mem_area, int count) {
	char mem[B];
	int p = index / 16 + 1;
	int q = (index % 16) * 32;
	read_block(p, mem);
	file_des f;
	f.len = char_to_int(mem + q);
	f.len += count;
	for (int k = 0; k < 7; k++)
		f.num[k] = char_to_int(mem + q + (k + 1) * 4);      //恢复文件描述符
	bool flag = false;
	for (int i = 0; i < number; i++) {
		if (fop[i].index == index) {
			flag = true;
			int p = fop[i].p / B;
			int q = fop[i].p%B;
			while (count) {
				int x = min(B - q, count);
				memcpy(fop[i].buf, mem_area, x);
				fop[i].p += x;
				if (fop[i].p == B) {              //打开下一块磁盘
					write_block(f.num[p], fop[i].buf);
					q = 0;
					p++;
					if (p>7)
						return -2;              //文件已满
					if (f.num[p] == -1)
						f.num[p] = get_space();
					read_block(f.num[p], fop[number].buf);
				}
				count -= x;
			}
		}
	}
	if (!flag)
		return 0;
	read_block(p, mem);                  //更新文件描述符
	f.len += count;
	int_to_char(f.len, mem + q);
	for (int i = 0; i < 7; i++)
		int_to_char(f.num[i], mem + q + (i + 1) * 4);
	write_block(p, mem);
	return 1;
}
//移动文件指针
int lseek(int index, int pos) {
	if (pos < 0)
		return -1;
	for (int i = 0; i < number; i++) {
		if (fop[i].index == index) {
			fop[i].p = pos;
			return 1;
		}
	}
	return 0;
}
//显示所有文件
int directory() {
	char mem[B], mem1[B];
	read_block(1, mem);
	file_des f;
	f.len = char_to_int(mem);
	for (int i = 0; i < 7; i++)
		f.num[i] = char_to_int(mem + (i + 1) * 4);
	printf("%28s\t\t%s\n", "文件名", "文件长度");
	for (int i = 0; f.num[i] != -1; i++) {
		read_block(f.num[i], mem);
		for (int j = 0; j < B; j += 32) {
			int index = char_to_int(mem + j + 28);
			if (index != -1) {
				char name[28];
				strcpy(name, mem + j);
				int p = index / 16 + 1;
				int q = (index % 16) * 32;
				read_block(p, mem1);
				int x = char_to_int(mem1 + q);
				printf("%28s\t\t%d\n", name, x);
			}
		}
	}
	return 0;
}
//显示打开文件
int directory_open() {
	printf("%10s\t\t%10s\n", "文件描述符号", "文件读写指针");
	for (int i = 0; i < number; i++) {
		printf("%10d\t\t%10d\n", fop[i].index, fop[i].p);
	}
	return 0;
}



