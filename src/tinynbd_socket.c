#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <endian.h>
#include <linux/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define B 1048576
uint64_t init_magic=0x0000420281861253;
uint32_t request_magic=0x25609513;
uint32_t reply_magic=0x67446698;
int fd=-1;
char rbuf[B]={'\0'};
int s,s2;
struct sockaddr_un sock;

void err(char *m){fprintf(stderr,m);if(fd!=-1){close(fd);}exit(1);}
#define E(x) if(-1==x){err(strerror(errno));}
void w(void *b,size_t l){int i; for(i=0;i<l;i+=write(fd,b+i,l-i));}
void r(void *b,size_t l){int i; for(i=0;i<l;i+=read(fd,b+i,l-i));}
void W(void *b,size_t l){int i; for(i=0;i<l;i+=write(s2,b+i,l-i));}
void R(void *b,size_t l){int i=0; while(i<l){ int j=read(s2,b+i,l-i); if(j==0){exit(0);} i+=j;}}
void in4(uint32_t *x){*x=be32toh(*x);}
void in8(uint64_t *x){*x=be64toh(*x);}
int64_t clip(int64_t x){return x > B ? B : x;}

int main(int argc, char **argv){
  if(argc<2){err("Need a file to serve\n");}
  if(argc<3){err("Need a unix socket path\n");}
  fd=open(argv[1],O_RDWR|O_SYNC); if(-1==fd){err("Couldn't open file\n");};

  s=socket(AF_UNIX,SOCK_STREAM,0);
  sock.sun_family=AF_UNIX;
  strcpy(sock.sun_path,argv[2]);
  unlink(sock.sun_path);
  E(bind(s,(struct sockaddr *)&sock, strlen(sock.sun_path)+sizeof(sock.sun_family)));
  E(listen(s,0));
  s2 = accept(s,NULL,NULL);
  E(s2);

  off64_t size=lseek64(fd,0,SEEK_END);__be64 wsz=htobe64(size);
  __be64 wmag=htobe64(init_magic); __be32 rmag=htobe32(reply_magic);
  W("NBDMAGIC",8);W(&wmag,8);W(&wsz,8); int i; for(i=0;i<128;i++){W("\0",1);}
  while(1){
    uint32_t m,t,l;char h[8];uint64_t f;
    R(&m,4);R(&t,4);R(h,8);R(&f,8);R(&l,4);
    in4(&m);in4(&t);in8(&f);in4(&l);
    if(m == request_magic && f < (uint64_t)1<<63 && l+f <= size){
      int64_t ll=l;
      lseek64(fd,f,SEEK_SET);
      if(t==0){
        W(&rmag,4);W("\0\0\0\0",4);W(h,8);
        for(;ll>0;ll-=B){r(rbuf,clip(ll));W(rbuf,clip(ll));}
      } else if(t==1){
        for(;ll>0;ll-=B){R(rbuf,clip(ll));w(rbuf,clip(ll));}
        W(&rmag,4);W("\0\0\0\0",4);W(h,8);
      }
    } else {
      W(&rmag,4);W("\1\0\0\0",4);W(h,8);
    }
  }
  return 0;
}
