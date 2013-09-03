#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <zlib.h>
#include <assert.h>	//利用assert()在开发时调试

#define MAXSIZE 8000    //缓存区大小
#define OS_CODE 0X03    //用于gzip压缩头

#define ERROR 1
#define LOG 2

void send_http_header(int client_sockfd,int status,char *s_status,char *filetype);
void send_html(int client_sockfd,char *uri);
void get_filetype(char *uri,char **filetype);
void twebser_exit(int server_sockfd);
int gzip_proc(char *html_buff,char *gzip_buff,int n,int *gzip_len);


//用于http头中文件类型的支持
struct {
        char *ext;
        char *type;
} extensions [] = { 
        {"gif", "image/gif" },  
        {"jpg", "image/jpg" },  
        {"jpeg","image/jpeg"},
        {"png", "image/png" },  
        {"ico", "image/ico" },  
        {"zip", "image/zip" },  
        {"gz",  "image/gz"  },  
        {"tar", "image/tar" },  
        {"htm", "text/html" },  
        {"html","text/html" }, 
	{"css","text/css"},
	{"js","text/javascript"},
        {0,0} };

//gzip头，此代码运行在Unix-like系统上，故OS_CODE为0X03
static const char gzip_header[10] = {'\037', '\213', Z_DEFLATED, 0, 0, 0, 0, 0, 0, OS_CODE};


void put_long (unsigned char *string, unsigned long x) {
	string[0] = (x & 0xff);
	string[1] = ((x >> 8) & 0xff) ;
	string[2] = ((x >> 16) & 0xff) ;
	string[3] = ((x >> 24) & 0xff);
}

/*
@description:向客户端发送网页头文件的信息 
@parameter 
client_socket:套接字描述符 
status:http协议的返回状态码
@s_status:http协议的状态码的含义 
@filetype:向客户端发送的文件类型 
*/
void send_http_header(int client_sockfd,int status,char *s_status,char *filetype)
{
	char header_buff[MAXSIZE];
	memset(header_buff,0,MAXSIZE);
	sprintf(header_buff, "HTTP/1.0 %d %s\r\n", status, s_status);  
    	sprintf(header_buff, "%sServer: Tiny Web Server\r\n", header_buff);  
    	sprintf(header_buff, "%sContent-Type: %s\r\n", header_buff, filetype);  
	sprintf(header_buff, "%sContent-Encoding: gzip\r\n\r\n", header_buff); 
    	write(client_sockfd, header_buff, strlen(header_buff));  
}

/* 
@description:向客户端发送文件 
@parameter 
client_sockfd:套接字描述符 
@uri:要发送文件路径 
*/
void send_html(int client_sockfd,char *uri)
{
	int fd,n;
	char *filetype=NULL;
	char html_buff[MAXSIZE*2];
	char gzip_buff[MAXSIZE*2+18];
	unsigned int gzip_len;
	char file_path[256]="www";
	strcat(file_path,uri);
	if(4==strlen(file_path))
		strcat(file_path,"index.html");
	fd=open(file_path,O_RDONLY);
	n=read(fd,html_buff,MAXSIZE*2);
	get_filetype(file_path,&filetype);
	gzip_proc(html_buff,gzip_buff,n,&gzip_len);
	send_http_header(client_sockfd,200,"OK",filetype);
	write(client_sockfd,gzip_buff,gzip_len);
}

/*  
@description:将要传输的静态文件用gzip压缩后传输
@parameter  
html_buff：要压缩静态文件的缓冲区指针
gzip_buff：压缩后存储的缓存区指针
n：html_buff中的字符的个数
gzip_len：用zip压缩后的长度
*/ 
int gzip_proc(char *html_buff,char *gzip_buff,int n,int *gzip_len)
{
	z_stream stream;
	int ret,have;

	memcpy(gzip_buff,gzip_header,10);
	
	stream.zalloc=Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;

	int tmp_result = deflateInit2(&stream,
					Z_DEFAULT_COMPRESSION,//压缩级别
					Z_DEFLATED,//压缩方式
					-MAX_WBITS,
					8,
					Z_DEFAULT_STRATEGY);
	if (Z_OK != tmp_result) {
		printf("deflateInit error: %d\r\n", tmp_result);
		return 0;
	}
	stream.avail_in = n; //要压缩数据的长度
	stream.next_in = html_buff;	//要压缩数据的首地址
	stream.avail_out = MAXSIZE*2;  //可存放的最大输出结果的长多。就是压缩后数据的最大长度
	stream.next_out = gzip_buff + 10; //存放压缩数据的开始位置，send前十个字节用来放头部
	ret = deflate (&stream,Z_FINISH); //压缩
	assert (ret != Z_STREAM_ERROR);
	switch (ret) { 
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd (&stream);
			return ret;
	}
	have = MAXSIZE*2 - stream.avail_out;
	unsigned crc = crc32(0L, html_buff, n); 
	char * tail = gzip_buff + 10 + have;
	put_long (tail, crc);
	put_long (tail + 4, n);
	*gzip_len=have+18;
	deflateEnd (&stream);
	return 1;
}

/*
@description:根据uri获得文件的类型信息
@parameter
uri:文件的uri
filetype：文件的类型
*/
void get_filetype(char *uri,char **filetype)
{
	int i,len,uri_len;

	uri_len=strlen(uri);
	
	for(i=0;extensions[i].ext != 0;i++) {
		len=strlen(extensions[i].ext);
		if(!strncmp(&uri[uri_len-len],extensions[i].ext,len)) {
			*filetype=extensions[i].type;
			break;
		}
	}	
}

void web_logger(int i,char *msg)
{
	
}

int main(int argc, char *argv[])	{
	int server_sockfd;
	int client_sockfd;

	int n,hit;

	char method[20],uri[80],version[60];

	int port;
	char basedir[20]={0};

	int server_len, client_len;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	static char buff[MAXSIZE+1];

	struct stat etc_s;

	server_sockfd=socket(AF_INET,SOCK_STREAM,0);

	if(server_sockfd==-1) {
		perror("socket error");
		web_logger(ERROR,"socket error");
		exit(1);
	}

	if(stat("etc/twebserver.conf",&etc_s)) {
		port=8080;
		strcpy(basedir,"www");
	}else {
		char *s;
		char t[80];
		char key[15];
		char value[15];
		FILE *conf_fp;
		conf_fp=fopen("etc/twebserver.conf","r");
		if(NULL==conf_fp) {
			perror("open twebserver.conf error");
			web_logger(ERROR,"open twebserver.conf error");
			exit(2);
		}
		while((s=fgets(t,80,conf_fp)) != NULL) {
			sscanf(s,"%s %s",key,value);
			if(!strcmp(key,"Port"))
				port=strtol(value,NULL,10);
			if(!strcmp(key,"BaseDir"))
				strcpy(basedir,value);
		}
		printf("according the conf file the port is %d,the basedir is %s",port,basedir);
		fclose(conf_fp);
	}

	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(port);

	server_len=sizeof(server_addr);

	if(bind(server_sockfd,(struct sockaddr *)&server_addr,server_len)) {
		perror("bind error");
		web_logger(ERROR,"bind error");
		exit(3);
	}

	if(listen(server_sockfd,10)) {
		perror("listen error");
		web_logger(ERROR,"listen error");
		exit(4);
	}

	signal(SIGINT,twebser_exit);

	for(hit=1; ;hit++)	{
		printf("waiting for connect request %d\n",hit);
		client_len=sizeof(client_addr);
		client_sockfd=accept(server_sockfd,(struct sockaddr *)&client_addr,&client_len);
		if(client_sockfd == -1) {
			perror("accept error");
			web_logger(ERROR,"the system call accept error");
			exit(5);
		}
		while(0==(n=read(client_sockfd,buff,MAXSIZE)) || n!=EOF) {
			printf("the n is %d\nfrom werbrowser:\n%s\n",n,buff);
			break;
		}
		sscanf(buff,"%s %s %s",method,uri,version);
		printf("connect request %d:method:%s,uri:%s,version:%s\n",hit,method,uri,version);
		send_html(client_sockfd,uri);
		close(client_sockfd);
	}
}

void twebser_exit(int server_sockfd)
{
	signal(SIGINT,SIG_DFL);
	close(server_sockfd);
	printf("the twebser exits\n");
	exit(0);
}
