#ifndef BETTERSOCKETS_H
#define BETTERSOCKETS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>

int loadsocklib(void);
int freesocklib(void);
int socktcp(int* psocket);
int sockclose(int socket);

int sockconnect(int socket, const char* address, int port);
int socklisten(int socket, const char* address, int port, int n);
int sockaccept(int socket, int* p_c_socket, char* c_address, int* p_c_port);
int hnametoipv4(const char* address, char* ipv4str);

int recvl(int socket, void* buf, int recvsize, int flags);
int prepl(void* buf, int maxsize, int* pwritten, const void* p, const int size);
int sendl(int socket, const void* buf, int size, int flags);

int recva(int socket, void* buf, int maxsize, int (*recvacb)(void* buf, int rcvd_processed, int* prcvd_last, void* data), void* recvacb_data);
int recvw(int socket, void* buf, int maxsize, int (*recvwcb)(void* buf, int* prcvd_last, void* data), void* recvwcb_data);
int sendw(int socket, void* buf, int (*sendwcb)(void* buf, int* pto_send, void* data), void* sendwcb_data);
int recvf(int socket, FILE* f, int recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data, int (*recvwcb)(void* buf, int* prcvd_last, void* data), void* recvwcb_data);
int sendf(int socket, FILE* f, int send_size, int bufsize, int (*stcb)(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data, int (*sendwcb)(void* buf, int* pto_send, void* data), void* sendwcb_data);

int recvm(int socket, void* buf, int maxsize, const void* needle, int needle_len);
int recvfm(int socket, FILE* f, int max_recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data, const void* needle, int needle_len);
int recvfp(int socket, FILE* f, int max_recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data);
int sendfp(int socket, FILE* f, int send_size, int bufsize, int (*stcb)(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data);
int recvp(int socket, void* buf, int maxsize);
int prepp(void* buf, int maxsize, int* pwritten, const void* p, const int size);
int sendp(int socket, const void* buf, int size);

int recvsm(int socket, char* s, int maxsize, const char* needle);
int recvs(int socket, char* s, int maxsize);
int recvsp(int socket, char* s, int maxsize);
int prepsm(void* buf, int maxsize, int* pwritten, const char* s);
int preps(void* buf, int maxsize, int* pwritten, const char* s);
int prepsp(void* buf, int maxsize, int* pwritten, const char* s);

int recvnints(int socket, int* p, int n);
int recvnlls(int socket, long long* p, int n);
int recvnfloats(int socket, float* p, int n);
int recvndoubles(int socket, double* p, int n);
int prepnints(void* buf, int maxsize, int* pwritten, const int* p, const int n);
int prepnlls(void* buf, int maxsize, int* pwritten, const long long* p, const int n);
int prepnfloats(void* buf, int maxsize, int* pwritten, const float* p, const int n);
int prepndoubles(void* buf, int maxsize, int* pwritten, const double* p, const int n);

int recvsints(int socket, char* buf, int maxsize, const char* sep, const char* end, int* p, int n);
int recvslls(int socket, char* buf, int maxsize, const char* sep, const char* end, long long* p, int n);
int recvsfloats(int socket, char* buf, int maxsize, const char* sep, const char* end, float* p, int n);
int recvsdoubles(int socket, char* buf, int maxsize, const char* sep, const char* end, double* p, int n);
int prepsints(char* buf, int maxsize, int* pwritten, const char* sep, const char* end, const int* p, const int n);
int prepslls(char* buf, int maxsize, int* pwritten, const char* sep, const char* end, const long long* p, const int n);
int prepsfloats(char* buf, int maxsize, int* pwritten, const char* sep, const char* end, const float* p, const int n, const char* float_format);
int prepsdoubles(char* buf, int maxsize, int* pwritten, const char* sep, const char* end, const double* p, const int n, const char* float_format);


#define recvprintf(p_ret, socket, bufsize) {\
		if(p_ret!=NULL) *p_ret = recvfp(socket, stdout, 0, bufsize, NULL, 1024, NULL);\
		else recvfp(socket, stdout, 0, bufsize, NULL, 1024, NULL);}
#define sendprintf(p_ret, socket, s, ...) {\
		int size = snprintf(NULL, 0, s, __VA_ARGS__) + 1;\
		char* buf = (char*)malloc(size); if (buf != NULL){\
			snprintf(buf, size, s, __VA_ARGS__);\
			if(p_ret!=NULL) *p_ret = sendp(socket, (const void*)buf, strlen(buf));\
			else sendp(socket, (const void*)s, strlen(s)); free((void*)buf);} else {if(p_ret!=NULL) *p_ret=-1;}}


int setsockrcvtimeout(int socket, int recvto_s, int recvto_us);
int setsocksndtimeout(int socket, int sendto_s, int sendto_us);
int getsockrcvtimeout(int socket, int* precvto_s, int* precvto_us);
int getsocksndtimeout(int socket, int* psendto_s, int* psendto_us);
int setsockrcvbsize(int socket, int buf_size);
int setsocksndbsize(int socket, int buf_size);
int getsockrcvbsize(int socket, int* pbuf_size);
int getsocksndbsize(int socket, int* pbuf_size);
int setsockblockmode(int socket, int block);
int socketreadable(int socket);



#ifdef __cplusplus
}
#endif

#endif

