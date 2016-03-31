#ifndef BETTERSOCKETS_H
#define BETTERSOCKETS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#define OS_WINDOWS
#else
	#define OS_UNIX // bad way of checking,... to change later
#endif

#ifdef OS_WINDOWS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif
#ifdef OS_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#endif

int loadsocklib(void);
int freesocklib(void);
int sockclose(int socket);

int sendl(int socket, const void* buf, int size, int flags);
int recvl(int socket, void* buf, int recvsize, int flags);
int recva(int socket, void* buf, int maxsize, int (*recvacb)(void* buf, int rcvd_processed, int* prcvd_last, void* data), void* recvacb_data);
int recvw(int socket, void* buf, int maxsize, int (*recvwcb)(void* buf, int* prcvd_last, void* data), void* recvwcb_data);
int sendw(int socket, void* buf, int (*sendwcb)(void* buf, int* pto_send, void* data), void* sendwcb_data);
int recvf(int socket, FILE* f, int recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data, int (*recvwcb)(void* buf, int* prcvd_last, void* data), void* recvwcb_data);
int sendf(int socket, FILE* f, int send_size, int bufsize, int (*stcb)(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data, int (*sendwcb)(void* buf, int* pto_send, void* data), void* sendwcb_data);

int recvm(int socket, void* buf, int maxsize, const void* needle, int needle_len);
int recvfm(int socket, FILE* f, int max_recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data, const void* needle, int needle_len);
int recvp(int socket, void* buf, int maxsize);
int sendp(int socket, const void* buf, int size);
int recvfp(int socket, FILE* f, int max_recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data);
int sendfp(int socket, FILE* f, int send_size, int bufsize, int (*stcb)(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data);

int recvsm(int socket, char* s, int maxsize);
int sendsm(int socket, const char* s);
int recvsp(int socket, char* s, int maxsize);
int sendsp(int socket, const char* s);
#define recvprintf(p_ret, socket, bufsize) {\
		if(p_ret!=NULL) *p_ret = recvfp(socket, stdout, 0, bufsize, NULL, 1000, NULL);\
		else recvfp(socket, stdout, 0, bufsize, NULL, 1000, NULL);}
#define sendprintf(p_ret, socket, s, ...) {\
		int size = snprintf(NULL, 0, s, __VA_ARGS__) + 1;\
		char* buf = (char*)malloc(size); if (buf != NULL){\
		snprintf(buf, size, s, __VA_ARGS__);\
		if(p_ret!=NULL) *p_ret = sendsp(socket, buf);\
		else sendsp(socket, s); free((void*)buf);} else {if(p_ret!=NULL) *p_ret=-1;}}


int sendchar(int socket, const char c);
int recvchar(int socket, char* pc);
int sendshort(int socket, const short s);
int recvshort(int socket, short* ps);
int sendint(int socket, const int i);
int recvint(int socket, int* pi);
int sendlonglong(int socket, const long long i);
int recvlonglong(int socket, long long* pi);
int sendfloat(int socket, const float f);
int recvfloat(int socket, float* pf);
int senddouble(int socket, const double f);
int recvdouble(int socket, double* pf);


// Utility functions
int hnametoipv4(const char* address, struct in_addr* p_ipv4);
int ipv4tostr(const struct in_addr* p_ipv4, char* ips, int ips_size);
int inttosaport(const int port, u_short* p_saport);
int saporttoint(const u_short* p_saport, int* p_port);

int sockconnect(int socket, const char* address, int port);
int socklisten(int socket, const char* address, int port, int n);
int sockaccept(int socket, int* p_c_socket, char* c_address, int* p_c_port);

// Utility functions for other purposes (not very useful)
int get_ticks_ms(void);
int setsockrcvtimeout(int socket, int recvto_s, int recvto_us);
int setsocksndtimeout(int socket, int sendto_s, int sendto_us);
int getsockrcvtimeout(int socket, int* precvto_s, int* precvto_us);
int getsocksndtimeout(int socket, int* psendto_s, int* psendto_us);
int setsockrcvbsize(int socket, int buf_size);
int setsocksndbsize(int socket, int buf_size);
int getsockrcvbsize(int socket, int* pbuf_size);
int getsocksndbsize(int socket, int* pbuf_size);
int setsockblockmode(int socket, int block);
int socketreadable(int socket, int seconds);
int setreuseaddr(int socket);


#ifdef __cplusplus
}
#endif

#endif

