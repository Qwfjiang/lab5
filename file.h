#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

//�ļ���������ÿ��ռ32�ֽ�
struct file_des {
	int len;        //�ļ�����
	int num[7];     //�ļ����䵽�Ĵ��̿������
	file_des() {
		len = 0;
		for (int i = 0; i < 7; i++)
			num[i] = -1;
	}
};
//Ŀ¼�ÿ��ռ32�ֽ�
struct dir {
	char name[28];  //�ļ���
	int index;      //�ļ����������
};
//���ļ���
struct file_open {
	char buf[B];
	int p, index;
};

int create(char *filename);     //����ָ���ļ����������ļ�
int destory(char *filename);   //ɾ��ָ���ļ�
int open(char *filename);       //���ļ�
int close(int index);          //�ر�ָ���ļ�
int read(int index, char *mem_area, int count);      //��ָ���ļ�˳�����count���ֽ�mem_areaָ�����ڴ�λ��
int write(int index, char *mem_area, int count);     //��mem_areaָ�����ڴ�λ�ÿ�ʼ��count���ֽ�˳��д��ָ���ļ�
int lseek(int index, int pos);      //���ļ��Ķ�дָ���ƶ��� pos ָ����λ��
int directory();                   //�б���ʾ�����ļ����䳤��
int directory_open();

#endif // FILE_H_INCLUDED
