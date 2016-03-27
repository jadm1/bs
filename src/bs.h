#ifndef BETTERSOCKETS_H
#define BETTERSOCKETS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define OS_WINDOWS
#else
#define OS_LINUX // temporary
#endif

#ifdef OS_WINDOWS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#ifdef OS_LINUX
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#endif


int loadsocklib(void);
int unloadsocklib(void);
int sclose(int socket);

// More reliable functions for socket communication
int sendl(int socket, const void* buf, int size, int flags);
int recvl(int socket, void* buf, int recvsize, int flags);
int recva(int socket, void* buf, int maxsize, int (*recvacb)(void* buf, int rcvd_processed, int* prcvd_last, void* data), void* recvacb_data);
int recvw(int socket, void* buf, int maxsize, int (*recvwcb)(void* buf, int* prcvd_last, void* data), void* recvwcb_data);
int sendw(int socket, void* buf, int (*sendwcb)(void* buf, int* pto_send, void* data), void* sendwcb_data);
int recvf(int socket, FILE* f, int recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data);
int sendf(int socket, FILE* f, int send_size, int bufsize, int (*stcb)(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data);

int recvmm(int socket, void* buf, int maxsize, const void* needle, int needle_len);
int recvs(int socket, char* s, int maxsize);
int sends(int socket, const char* s);
int sendpl(int socket, const void* buf, int size);
int recvpl(int socket, void* buf, int maxsize);
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
u_long hnametoipv4(const char* address);
void ipv4tostr(char* ips, int ips_size, u_long ip_address);


// Utility functions for other purposes (not very useful)
int get_ticks_ms(void);
int setsocktimeouts(int socket, int sendto_sec, int recvto_sec);
int setsockblocking(int socket, int nonblock);
int socketreadable(int socket, int seconds);
int setreuseaddr(int socket);



#ifdef __cplusplus
}
#endif

#endif

