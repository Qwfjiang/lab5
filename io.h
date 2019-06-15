#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

const int MAXN = 1000;
const int L = 4096;
const int B = 512;


void read_block(int i, char *p);
void write_block(int i, char *p);
void read_file();
void write_file();

#endif // IO_H_INCLUDED
