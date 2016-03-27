#include "bs.h"

#ifndef min
#define min(a, b) ((b > a) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((b > a) ? (b) : (a))
#endif


void* memmem2(const void* haystack, size_t haystack_len, const void* needle,  size_t needle_len);

#ifdef OS_LINUX
int get_ticks_ms(void)
{
	struct tms tm;
	return times(&tm) * 10;
}
#endif
#ifdef OS_WINDOWS
int get_ticks_ms(void)
{
	return (int)GetTickCount();
}
#endif


int loadsocklib()
{
#ifdef OS_WINDOWS
	WSADATA wsaData;
	WORD wsaVer;
	memset(&wsaData, 0, sizeof(WSADATA));
	wsaVer = MAKEWORD(2, 2);
	return WSAStartup(wsaVer, &wsaData);
#endif
#ifdef OS_LINUX
	return 0;
#endif
	return -1;
}

int unloadsocklib()
{
#ifdef OS_WINDOWS
	return WSACleanup();
#endif
#ifdef OS_LINUX
	return 0;
#endif
	return -1;
}


int sclose(int socket)
{
#ifdef OS_WINDOWS
	int ret = 0;
	closesocket(socket);
	return ret;
#endif
#ifdef OS_LINUX
	return close(socket);
#endif
	return -1;
}

/**
 * This function sends data of a given size
 * Keeps sending data while there is remaining data to be sent according to the size parameter
 *
 * Parameters:
 * int socket
 * socket identifier used for communication
 * const void* buf
 * buffer of the data to be sent
 * sendl does not modify the buffer
 * int size
 * number of bytes to send
 * the buffer should has at least size bytes
 * int flags
 * flags for the send() function
 *
 * Return value:
 * number of bytes sent (should be size if everything went well)
 * if less it means the socket is not connected or no data is available (non blocking mode)
 */
int sendl(int socket, const void* buf, int size, int flags)
{
	int ret;
	const char* p = (const char*)buf;
	int sent = 0;

	while (sent < size)
	{
		ret = send(socket, (const void*)p, size - sent, flags);
		if (ret < 0)
			return -1;
		if (ret == 0) {
			// exit because no data sent (usually means socket disconnected)
			return sent;
		}
		p += ret;
		sent += ret;
	}

	return sent;
}


/**
 * This function receives data until a given size is reached
 * It keeps receiving data while there is remaining data to be received according to the recvsize parameter
 * This function is useful only if we know beforehand the exact size of the message we want to receive
 * All the data will be written in the buffer buf
 *
 * Parameters:
 * int socket
 * socket identifier used for communication
 * void* buf
 * pointer to a buffer used to receive the data
 * int recvsize
 * exact number of bytes to receive.
 * The function will continue receiving until recvsize bytes have been received
 * int flags
 * flags for the recv() function
 *
 * Return value:
 * number of bytes read (should be recvsize if everything went well)
 * if less it means the socket is not connected or no data is available (non blocking mode)
 */
int recvl(int socket, void* buf, int recvsize, int flags)
{
	int ret;
	char* p = (char*)buf;
	int rcvd = 0;

	while (rcvd < recvsize)
	{
		ret = recv(socket, (void*)p, recvsize - rcvd, flags);
		if (ret < 0)
			return -1;
		if (ret == 0) {
			// exit because no data available (usually means socket disconnected)
			return rcvd;
		}
		p += ret;
		rcvd += ret;
	}

	return rcvd;
}

/**
 * This function receives data continuously until a callback function
 * determines from the data received and an optional databag
 * that it must stop
 *
 * It is useful if we do not know the size of the message to be received but we
 * know how to infer that we are done from the received data
 *
 * Parameters:
 * int socket
 * socket identifier used for communication
 * void* buf
 * buffer where to receive the data
 * new data is copied after the data that has already been received and processed
 * int maxsize
 * the maximum size in bytes to use in the buffer
 * the buffer itself maybe larger but no more than maxsize bytes can be written in it
 * int maxrecvsize
 * a limit on the number of bytes that can be received for one recv() call
 * this can be useful if we want to process incoming data often (every maxrecvsize)
 * (*recvacb)
 * the callback it must not be NULL otherwise will crash (the function would be useless with this set as NULL)
 * void* recvacb_data
 * an optional data bag for the callback
 * Return value:
 * number of bytes received
 *
 * Callback info:
 * // Call the callback
 * should return -1 if error, 1 to continue receiving, 0 to stop reception
 *
 * *prcvd_size is the size of the last read in the network buffer
 * but it might get updated to a smaller value if we only want to process the begining of the data
 * the callback can know what size of the buffer was written: rcvd_total + *prcvd_size
 * *prcvd_size can be modified and its modified value will be effectively read and process from the network buffer
 * so that at the next callback call: rcvd_total(next) = rcvd_total(now) + *prcvd_size(modified)
 * Callback parameters:
 * void* buf
 * buffer where the data is written to when received
 * int maxsize
 * maximum size in bytes of the buffer
 */
int recva(int socket, void* buf, int maxsize, int (*recvacb)(void* buf, int rcvd_processed, int* prcvd_last, void* data), void* recvacb_data)
{
	int ret = 0;
	int retacb;
	char* p;
	int rcvd_total;
	int rcvd_last;

	rcvd_total = 0;
	p = (char*)buf;
	while (1)
	{
		// First only peek received data because we don't know yet if we want to process it all
		ret = recv(socket, (void*)p, maxsize - rcvd_total, MSG_PEEK);
		if (ret < 0)
			return -1;
		if (ret == 0) {
			// exit because no data available (usually means socket disconnected)
			return rcvd_total;
		}
		rcvd_last = ret;

		// Call the callback
		// should return -1 if error, 1 to continue, 0 to stop
		// rcvd_last is the size of what was read in the network buffer
		// but it might get updated to a smaller value if we only want to process the begining of the data
		// the callback can know what size of the buffer was written: rcvd_total + *prcvd_size == rcvd_total + rcvd_last
		ret = recvacb(buf, rcvd_total, &rcvd_last, recvacb_data);
		if (ret < 0)
			return -1;
		retacb = ret;

		if (rcvd_last > 0) {
			// Now we process rcvd_last bytes exactly (no peeking)
			ret = recvl(socket, (void*)p, rcvd_last, 0);
			if (ret < 0)
				return -1;
			// ret should be equal to rcvd_last, if it not the case the socket may have been closed, so it will exit at next recv()
			rcvd_last = ret;

			rcvd_total += rcvd_last;
			p += rcvd_last;
		}

		if (retacb == 0 || rcvd_total == maxsize) // we should stop receiving according to the callback (it's done) OR because we are full
			return rcvd_total; // this is the total number of bytes read from the network buffer
	}
}


/**
 * This function receives data in a similar way as recva
The main difference is that the buf is overwritten everytime new data is received
(it is not appended at the end)
This is useful if we don't need to save all the data into buf but we
 can process the data as soon as it is available and drop it when we are done
 For example in a file transfer we can write the data we receive to a FILE pointer and then
 we don't care about the data so its more memory efficient to overwrite it with newer data.

 So here the callback plays a much more important role since
 it has to process the data as it receives it and it has to determine when to stop
 for recva, the callback only determines when to stop but it is assumed that the processing
 is done afterwards.
 buf's data should not be used after using recvw because all the processing should be done

 the maxsize parameter there can be set to something lower than the buffer size if you want to limit
 the number of bytes processed by a single recv() call

 in this case the only difference in the prototype is that the callback does not have a rcvd_total parameter
 because it only looks at the most recent data

 returns the number of bytes processed
 */
int recvw(int socket, void* buf, int maxsize, int (*recvwcb)(void* buf, int* prcvd_last, void* data), void* recvwcb_data)
{
	int ret = 0;
	int retwcb;
	int rcvd_total;
	int rcvd_last;

	rcvd_total = 0;
	while (1)
	{
		// First only peek received data because we don't know yet if we want to process it all
		ret = recv(socket, buf, maxsize, MSG_PEEK);
		if (ret < 0)
			return -1;
		if (ret == 0) {
			// exit because no data available (usually means socket disconnected)
			return rcvd_total;
		}
		rcvd_last = ret;

		// Call the callback
		// should return -1 if error, 1 to continue, 0 to stop
		// rcvd_last is the size of what was read in the network buffer
		// but it might get updated to a smaller value if we only want to process the begining of the data
		// the callback can know what size of the buffer was written: rcvd_total + *prcvd_size == rcvd_total + rcvd_last
		ret = recvwcb(buf, &rcvd_last, recvwcb_data);
		if (ret < 0)
			return -1;
		retwcb = ret;

		if (rcvd_last > 0) {
			// Now we process rcvd_last bytes exactly (no peeking)
			ret = recvl(socket, buf, rcvd_last, 0);
			if (ret < 0)
				return -1;
			// ret should be equal to rcvd_last, if it not the case the socket may have been closed, so it will exit at next recv()
			rcvd_last = ret;

			rcvd_total += rcvd_last;
		}

		if (retwcb == 0) // we should stop receiving according to the callback (it's done). This time no need of:  || rcvd_total == maxsize because buffer is overwritten
			return rcvd_total; // this is the total number of bytes read from the network buffer
	}
}

int sendw(int socket, void* buf, int (*sendwcb)(void* buf, int* pto_send, void* data), void* sendwcb_data)
{
	int ret = 0;
	int sent_total;
	int to_send;

	sent_total = 0;
	while (1)
	{
		// First call the callback
		// should return -1 if error, 1 to continue, 0 to stop
		// to_send is the size of what will be written in the network buffer and sent
		ret = sendwcb(buf, &to_send, sendwcb_data);
		if (ret < 0)
			return -1;

		if (to_send > 0) {
			// Now we process sent_last bytes exactly
			ret = sendl(socket, buf, to_send, 0);
			if (ret < 0)
				return -1;

			sent_total += to_send;

			// ret should be equal to to_send, if it not the case the socket may have been closed, so it will exit
			if (ret != to_send) {
				return sent_total;
			}
		}

		if (ret == 0) // we should stop receiving according to the callback (it's done)
			return sent_total; // this is the total number of bytes read from the network buffer
	}
}


typedef struct recvf_bag {
	FILE* f;
	int recv_size;
	int rcvd_total;

	int rcvd_total_lastcb;
	int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data);
	int interval_ms;
	int tmstart_ms;
	int tmlastcb_ms;
	int nbofcb;
	double speed_Bps;
	void* tcb_data;
} recvf_bag;
int recvf_wcb(void* buf, int* prcvd_last, void* data) {
	int ret = 0;
	recvf_bag *pbag = (recvf_bag*)data;
	FILE* f = pbag->f;
	int recv_size = pbag->recv_size;   // total size to receive
	int rcvd_total = pbag->rcvd_total; // total size received and processed
	int rcvd_last = *prcvd_last;       // last size received (total size received now is rcvd_total+rcvd_last)
	int tm;
	int delta_tm;

	if (recv_size != 0) // if its == 0 don't correct
		rcvd_last = min(rcvd_last, recv_size - rcvd_total); // correct the size we are processing

	ret = fwrite((const void*)buf, rcvd_last, 1, f);
	if (ret <= 0) {
		return -1; // error
	}

	rcvd_total += rcvd_last;
	pbag->rcvd_total = rcvd_total;
	*prcvd_last = rcvd_last; // output the exact size we processed

	tm = get_ticks_ms();
	if (tm - pbag->tmstart_ms >= pbag->nbofcb * pbag->interval_ms) {
		pbag->nbofcb++;
		delta_tm = tm - pbag->tmlastcb_ms;
		if (delta_tm > 0) { // update value if dt > 0 since last callback
			pbag->speed_Bps = 1000.0*(double)(rcvd_total - pbag->rcvd_total_lastcb)/(double)delta_tm;
		}
		pbag->tmlastcb_ms = tm;
		if (pbag->rtcb != NULL && delta_tm > 0) {
			ret = pbag->rtcb(rcvd_total, recv_size, tm - pbag->tmstart_ms, pbag->speed_Bps, pbag->tcb_data);
			if (ret < 0) {
				return -1;
			}
		}
		pbag->rcvd_total_lastcb = rcvd_total;
	}

	if (recv_size != 0 && rcvd_total >= recv_size) {
		// we are done receiving
		return 0;
	}
	else {
		// receive more data
		return 1;
	}
}
// int size
// number of bytes to send
// if size == 0 then it will receive until the socket gets closed
// since the file size has to be known it can be useful to send it first with sendint for example
int recvf(int socket, FILE* f, int recv_size, int bufsize, int (*rtcb)(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data) {
	int ret = 0;
	recvf_bag bag;
	void* buf;
	int rcvd_total;
	int tm;
	int delta_tm;

	bag.f = f;
	bag.recv_size = recv_size;
	bag.rcvd_total = 0;
	bag.rtcb = rtcb;
	bag.interval_ms = interval_ms;
	bag.tcb_data = tcb_data;

	buf = malloc(bufsize);
	if (buf == NULL) {
		return -1;
	}


	tm = get_ticks_ms();
	bag.tmstart_ms = tm;
	bag.nbofcb = 1;
	bag.speed_Bps = 0.0;

	bag.tmlastcb_ms = tm;
	if (rtcb != NULL) {
		ret = rtcb(0, recv_size, 0, 0.0, tcb_data); // first time call it
		if (ret < 0) {
			free(buf);
			return -1;
		}
	}
	bag.rcvd_total_lastcb = 0;

	ret = recvw(socket, buf, bufsize, recvf_wcb, (void*)&bag);
	if (ret < 0) {
		free(buf);
		return -1;
	}
	rcvd_total = ret;

	tm = get_ticks_ms();
	bag.nbofcb++;
	delta_tm = tm - bag.tmlastcb_ms;
	if (delta_tm > 0) { // update value if dt > 0 since last callback
		bag.speed_Bps = 1000.0*(double)(rcvd_total - bag.rcvd_total_lastcb)/(double)delta_tm;
	}
	bag.tmlastcb_ms = tm;
	if (rtcb != NULL && delta_tm > 0) {
		ret = rtcb(rcvd_total, (recv_size > 0 ? recv_size : rcvd_total), tm - bag.tmstart_ms, bag.speed_Bps, tcb_data); // last time call it
		if (ret < 0) {
			free(buf);
			return -1;
		}
	}
	bag.rcvd_total_lastcb = rcvd_total;

	free(buf);

	return rcvd_total;
}



typedef struct sendf_bag {
	FILE* f;
	int send_size;
	int sent_total;
	int buf_size;

	int sent_total_lastcb;
	int (*stcb)(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data);
	int interval_ms;
	int tmstart_ms;
	int tmlastcb_ms;
	int nbofcb;
	double speed_Bps;
	void* tcb_data;
} sendf_bag;
int sendf_wcb(void* buf, int* pto_send, void* data) {
	int ret = 0;
	sendf_bag *pbag = (sendf_bag*)data;
	FILE* f = pbag->f;
	int send_size = pbag->send_size;   // total size to receive
	int sent_total = pbag->sent_total; // total size received and processed
	int to_send;
	int tm;
	int delta_tm;

	to_send = min(pbag->buf_size, send_size - sent_total);

	ret = fread((void*)buf, to_send, 1, f);
	if (ret <= 0) {
		return -1; // error
	}


	tm = get_ticks_ms();
	if (tm - pbag->tmstart_ms >= pbag->nbofcb * pbag->interval_ms) {
		pbag->nbofcb++;
		delta_tm = tm - pbag->tmlastcb_ms;
		if (delta_tm > 0) { // update value if dt > 0 since last callback
			pbag->speed_Bps = 1000.0*(double)(sent_total - pbag->sent_total_lastcb)/(double)delta_tm;
		}
		pbag->tmlastcb_ms = tm;
		if (pbag->stcb != NULL && delta_tm > 0) {
			ret = pbag->stcb(sent_total, send_size, tm - pbag->tmstart_ms, pbag->speed_Bps, pbag->tcb_data);
			if (ret < 0) {
				return -1;
			}
		}
		pbag->sent_total_lastcb = sent_total;
	}


	sent_total += to_send;
	pbag->sent_total = sent_total;
	*pto_send = to_send; // output the exact size we processed

	if (sent_total >= send_size) {
		// we are done receiving
		return 0;
	}
	else {
		// receive more data
		return 1;
	}
}
// int size
// number of bytes to send
// if size == 0 then it will send until EOF is reached or the socket is closed
// since the file size has to be known it can be useful to send it first with sendint for example
int sendf(int socket, FILE* f, int send_size, int bufsize, int (*stcb)(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data), int interval_ms, void* tcb_data) {
	int ret = 0;
	sendf_bag bag;
	void* buf;
	int sent_total;
	int tm;
	int delta_tm;
	int pos;

	if (send_size == 0) {
		pos = ftell(f);
		fseek(f, 0, SEEK_END);
		send_size = ftell(f) - pos;
		fseek(f, pos, SEEK_SET);
	}

	bag.f = f;
	bag.send_size = send_size;
	bag.sent_total = 0;
	bag.buf_size = bufsize;
	bag.stcb = stcb;
	bag.interval_ms = interval_ms;
	bag.tcb_data = tcb_data;

	buf = malloc(bufsize);
	if (buf == NULL) {
		return -1;
	}


	tm = get_ticks_ms();
	bag.tmstart_ms = tm;
	bag.nbofcb = 1;
	bag.speed_Bps = 0.0;

	bag.tmlastcb_ms = tm;
	if (stcb != NULL) {
		ret = stcb(0, send_size, 0, 0.0, tcb_data); // first time call it
		if (ret < 0) {
			free(buf);
			return -1;
		}
	}
	bag.sent_total_lastcb = 0;

	ret = sendw(socket, buf, sendf_wcb, (void*)&bag);
	if (ret < 0) {
		free(buf);
		return -1;
	}
	sent_total = ret;

	tm = get_ticks_ms();
	bag.nbofcb++;
	delta_tm = tm - bag.tmlastcb_ms;
	if (delta_tm > 0) { // update value if dt > 0 since last callback
		bag.speed_Bps = 1000.0*(double)(sent_total - bag.sent_total_lastcb)/(double)delta_tm;
	}
	bag.tmlastcb_ms = tm;
	if (stcb != NULL && delta_tm > 0) {
		ret = stcb(sent_total, send_size, tm - bag.tmstart_ms, bag.speed_Bps, tcb_data); // last time call it
		if (ret < 0) {
			free(buf);
			return -1;
		}
	}
	bag.sent_total_lastcb = sent_total;

	free(buf);

	return sent_total;
}







typedef struct recvmm_bag {
	const void* needle;
	int needle_len;
} recvmm_bag;

int recvmm_acb(void* buf, int rcvd_processed, int* prcvd_last, void* data) {
	recvmm_bag *pbag = (recvmm_bag*)data;
	const void* needle = pbag->needle;
	int needle_len = pbag->needle_len;
	void* begin;

	int rcvd_last = *prcvd_last;
	int buf_size = rcvd_processed + rcvd_last;

	begin = memmem2(buf, buf_size, needle, needle_len);
	if (begin == NULL) {
		return 1; // continue to receive because the needle was not found
	}
	else {
		// needle was found, recompute the rcvd_size and exit
		rcvd_last = (char*)begin - (char*)buf;
		rcvd_last -= rcvd_processed;
		rcvd_last += needle_len; // include the needle in the received message
		*prcvd_last = rcvd_last;
		return 0; // stop
	}
}

int recvmm(int socket, void* buf, int maxsize, const void* needle, int needle_len) {
	int ret;
	recvmm_bag bag;
	bag.needle = needle;
	bag.needle_len = needle_len;
	ret = recva(socket, buf, maxsize, recvmm_acb, &bag);
	return ret;
}


int recvs(int socket, char* s, int maxsize) {
	int ret;
	ret = recvmm(socket, (void*)s, maxsize, (const void*)"\0", 1);
	s[ret] = '\0'; // if the recvmm failed to return all the data add a '\0' to make sure the string is terminated
	return ret;
}

int sends(int socket, const char* s) {
	int ret;
	ret = sendl(socket, (const void*)s, strlen(s)+1, 0);
	return ret;
}

int sendpl(int socket, const void* buf, int size) {
	int ret;

	ret = sendint(socket, size);
	if (ret < 0) {
		return -1;
	}
	if (ret != sizeof(int)) {
		return 0;
	}

	ret = sendl(socket, buf, size, 0);

	return ret;
}

int recvpl(int socket, void* buf, int maxsize) {
	int ret;
	int size;

	ret = recvint(socket, &size);
	if (ret < 0) {
		return -1;
	}
	if (ret != sizeof(int)) {
		return 0;
	}

	ret = recvl(socket, buf, min(size, maxsize), 0);

	return ret;
}

int sendchar(int socket, const char c) {
	int ret = 0;
	ret = sendl(socket, (const void*)&c, sizeof(char), 0);
	return ret;
}

int recvchar(int socket, char* pc) {
	int ret = 0;
	ret = recvl(socket, (void*)pc, sizeof(char), 0);
	return ret;
}

int sendshort(int socket, const short s) {
	int ret = 0;
	ret = sendl(socket, (const void*)&s, sizeof(short), 0);
	return ret;
}

int recvshort(int socket, short* ps) {
	int ret = 0;
	ret = recvl(socket, (void*)ps, sizeof(short), 0);
	return ret;
}

int sendint(int socket, const int i) {
	int ret = 0;
	ret = sendl(socket, (const void*)&i, sizeof(int), 0);
	return ret;
}

int recvint(int socket, int* pi) {
	int ret = 0;
	ret = recvl(socket, (void*)pi, sizeof(int), 0);
	return ret;
}

int sendlonglong(int socket, const long long i) {
	int ret = 0;
	ret = sendl(socket, (const void*)&i, sizeof(long long), 0);
	return ret;
}

int recvlonglong(int socket, long long* pi) {
	int ret = 0;
	ret = recvl(socket, (void*)pi, sizeof(long long), 0);
	return ret;
}

int sendfloat(int socket, const float f) {
	int ret = 0;
	ret = sendl(socket, (const void*)&f, sizeof(float), 0);
	return ret;
}

int recvfloat(int socket, float* pf) {
	int ret = 0;
	ret = recvl(socket, (void*)pf, sizeof(float), 0);
	return ret;
}

int senddouble(int socket, const double f) {
	int ret = 0;
	ret = sendl(socket, (const void*)&f, sizeof(double), 0);
	return ret;
}

int recvdouble(int socket, double* pf) {
	int ret = 0;
	ret = recvl(socket, (void*)pf, sizeof(double), 0);
	return ret;
}


int setsocktimeouts(int socket, int sendto_sec, int recvto_sec) {
	int ret;
	struct timeval timeout;

	timeout.tv_sec = recvto_sec;
	timeout.tv_usec = 0;
	ret = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(timeout));
	if (ret < 0)
		return -1;

	timeout.tv_sec = sendto_sec;
	timeout.tv_usec = 0;
	ret = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout, sizeof(timeout));
	if (ret < 0)
		return -1;

	return 0;
}


int setsockblocking(int socket, int nonblock)
{
#ifdef OS_WINDOWS
	int opts = (nonblock != 0);
	return ioctlsocket(socket, FIONBIO, (u_long*)&opts);
#endif
#ifdef OS_LINUX
	int opts;
	opts = fcntl(socket, F_GETFL);
	if (opts < 0)
		return -1;
	if (nonblock != 0)
		opts |= O_NONBLOCK;
	else
		opts &= (~O_NONBLOCK);
	return fcntl(socket, F_SETFL, opts);
#endif
	return -1;
}

int socketreadable(int socket, int seconds)
{
#ifdef OS_WINDOWS
	fd_set fds;
	struct timeval timeout;
	timeout.tv_usec = 0;
	timeout.tv_sec  = seconds;
	fds.fd_count = 1;
	fds.fd_array[0] = socket;
	if (seconds >= 0)
		return select(0, &fds, NULL, NULL, &timeout);
	else
		return select(0, &fds, NULL, NULL, NULL);
#endif
#ifdef OS_LINUX
	int r;
	fd_set fds;
	struct timeval timeout;
	timeout.tv_usec = 0;
	timeout.tv_sec  = seconds;
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	if (seconds >= 0)
		r = select(socket + 1, &fds, NULL, NULL, &timeout);
	else
		r = select(socket + 1, &fds, NULL, NULL, NULL);

	if (r < 0)
		return -1;

	return FD_ISSET(socket, &fds);
#endif
	return -1;
}


int setreuseaddr(int socket)
{
	int opt = 1;
	int ret;
	ret = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));
	if (ret < 0)
		return -1;
	return 0;
}




u_long hnametoipv4(const char* address)
{
	struct hostent* rhost = NULL;
	int i;

	if (address == NULL)
		return htonl(0);
	if (address[0] == '\0')
		return htonl(0);

	for (i = 0; i < strlen(address); i++)
	{
		if ((((address[i] < '0') || (address[i] > '9')) && (address[i] != '.')) || (i >= 16))
		{
			rhost = gethostbyname(address);
			if (rhost != NULL)
				return *(u_long*)(rhost->h_addr_list[0]);
			else
				return htonl(0);
		}
	}

	return inet_addr(address);
}

void ipv4tostr(char* ips, int ips_size, u_long ip_address)
{
	struct in_addr in;
	memset(&in, 0, sizeof(struct in_addr));
	in.s_addr = ip_address;
	strncpy(ips, inet_ntoa(in), ips_size);
	return;
}


/* Return the first occurrence of NEEDLE in HAYSTACK.  */
void* memmem2(const void* haystack, size_t haystack_len, const void* needle,  size_t needle_len)
{
	const char *begin;
	const char *const last_possible = (const char *) haystack + haystack_len - needle_len;

	if (needle_len == 0)
		/* The first occurrence of the empty string is deemed to occur at
       the beginning of the string.  */
		return (void *) haystack;

	/* Sanity check, otherwise the loop might search through the whole
     memory.  */
	if (__builtin_expect (haystack_len < needle_len, 0))
		return NULL;

	for (begin = (const char *) haystack; begin <= last_possible; ++begin)
		if (begin[0] == ((const char *) needle)[0] &&
				!memcmp ((const void *) &begin[1],
						(const void *) ((const char *) needle + 1),
						needle_len - 1))
			return (void *) begin;

	return NULL;
}

