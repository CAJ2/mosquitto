/*
Support for PROXY v1 from a load balancer.
*/

#include "config.h"

#ifdef WITH_BROKER
#  include "mosquitto_broker_internal.h"
#  ifdef WITH_WEBSOCKETS
#    include <libwebsockets.h>
#  endif
#else
#  include "read_handle.h"
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "mosquitto_internal.h"
#include "mqtt3_protocol.h"
#include "net_mosq.h"
#include "proxy_mosq.h"
#include "packet_mosq.h"
#include "net_mosq.h"
#include "memory_mosq.h"
#include "logging_mosq.h"

/* PROXY v1:
 * As of MQTT v3.1.1:
 * If the PROXY header exists, the first char is the same as
 * a PUBREC command (the flags are all reserved and set to 0).
 * The minimum size for PROXY header is much larger than the size
 * of a PUBREC command, which is always 4 bytes. 
 */
int proxy__read_header(struct mosquitto *mosq) {
    uint8_t byte;
    uint8_t header[5] = {0x50, 0x52, 0x4f, 0x58, 0x59};
    ssize_t read_length;
    int8_t proxy_out;

    if (mosq->in_packet.proxy <= 0 || mosq->in_packet.proxy >= PROXY_READING) {
        do {
            if (mosq->in_packet.proxy >= PROXY_READING) {
                /* If reading proxy, try to read the max size minus what we have.
                 * This may capture some MQTT bytes too, but the parsing code can just
                 * use the buffer instead of calling net__read.
                 */
		if (PROXY_MIN_SIZE - mosq->in_packet.bufSize > 0) {
			read_length = net__read(mosq, mosq->in_packet.buffer + mosq->in_packet.bufSize, PROXY_MIN_SIZE - mosq->in_packet.bufSize);
		} else {
			read_length = net__read(mosq, mosq->in_packet.buffer + mosq->in_packet.bufSize, 1);
		}
            } else {
                read_length = net__read(mosq, &byte, 1);
            }
            /* If reading the proxy and we actually got some bytes */
            if (mosq->in_packet.proxy >= PROXY_READING && read_length > 0) {
                mosq->in_packet.bufSize += read_length;
                proxy_out = proxy__verify_header(mosq);
                if (proxy_out > 0) {
                    return MOSQ_ERR_SUCCESS;
                } else if (proxy_out < 0) {
                    return MOSQ_ERR_PROXY;
                }
            }
            else if (read_length == 1) {
                /* Throw the byte into the buffer */
                mosq->in_packet.buffer[mosq->in_packet.bufSize] = byte;
                mosq->in_packet.bufSize++;
                /* Check if byte matches header */
                if (byte == header[mosq->in_packet.proxy * -1]) {
                    mosq->in_packet.proxy--;
                }
                /* Proxy exists if first two bytes match, explained above */
                if (mosq->in_packet.proxy == -2) {
                    mosq->in_packet.proxy = PROXY_READING;
                }
                /* We have two bytes without matching, so no proxy */
                else if (mosq->in_packet.bufSize >= 2) {
                    mosq->in_packet.proxy = PROXY_INVALID;
                }
            }
            else {
            #ifdef WIN32
                errno = WSAGetLastError();
            #endif
                if(errno == EAGAIN || errno == COMPAT_EWOULDBLOCK){
                    if(mosq->in_packet.to_process > 1000){
                        /* Update last_msg_in time if more than 1000 bytes left to
                            * receive. Helps when receiving large messages.
                            * This is an arbitrary limit, but with some consideration.
                            * If a client can't send 1000 bytes in a second it
                            * probably shouldn't be using a 1 second keep alive. */
                        pthread_mutex_lock(&mosq->msgtime_mutex);
                        mosq->last_msg_in = mosquitto_time();
                        pthread_mutex_unlock(&mosq->msgtime_mutex);
                    }
                    return MOSQ_ERR_SUCCESS;
                }else{
                    switch(errno){
                        case COMPAT_ECONNRESET:
                            return MOSQ_ERR_CONN_LOST;
                        default:
                            return MOSQ_ERR_ERRNO;
                    }
                }
            }
        } while (mosq->in_packet.proxy <= 0 || mosq->in_packet.proxy >= PROXY_READING);
    }

    return MOSQ_ERR_SUCCESS;
}

/* Checks the buffer for a valid proxy header. If may be valid but is not yet,
 * returns 0. If it is invalid, returns -1.
 * If the header is valid, it sets the remote_host and remote_port, and shifts
 * out the proxy line from the buffer, leaving any MQTT data still in the buffer.
 */
int8_t proxy__verify_header(struct mosquitto *mosq) {
    char *loc;
    char *loc2;
    char *end;

    if (mosq->in_packet.bufSize >= PROXY_MAX_SIZE) {
        return -1;
    }
    if (mosq->in_packet.bufSize >= 8 && memcmp(mosq->in_packet.buffer, "PROXY", 5) == 0) {
        /* Search for end of proxy line */
        loc = memchr(mosq->in_packet.buffer, PROXY_CR, mosq->in_packet.bufSize - 1);
        if (!loc) {
            return 0;
        }
	else if (loc && loc[1] != PROXY_LF) {
	    return -1;
	}
        *(loc - 1) = '\0';
        end = loc + 2;
        /* Search for first space to get family */
        loc = memchr(mosq->in_packet.buffer, ' ', mosq->in_packet.bufSize - 1);
        if (!loc) return -1;
        *loc = '\0';
        if (memcmp(loc + 1, "TCP4", 4) == 0) {
            mosq->remote_af = AF_INET;
        }
        else if (memcmp(loc + 1, "TCP6", 4) == 0) {
            mosq->remote_af = AF_INET6;
        }
        else {
            return -1;
        }
        /* Search for second space for src */
        loc = memchr(mosq->in_packet.buffer, ' ', mosq->in_packet.bufSize - 1);
        if (!loc) return -1;
        *loc = '\0';
        loc2 = memchr(mosq->in_packet.buffer, ' ', mosq->in_packet.bufSize - 1);
        if (!loc2) return -1;
        *loc2 = '\0';
	if (mosq->remote_host == NULL) {
		mosq->remote_host = mosquitto__malloc(PROXY_HOST_SIZE);
		if (!mosq->remote_host) return MOSQ_ERR_NOMEM;
	}
	if (strcpy(mosq->remote_host, loc + 1) == NULL) {
	    return -1;
        }
        /* Get src port */
        loc = memchr(mosq->in_packet.buffer, ' ', mosq->in_packet.bufSize - 1);
        if (!loc) return -1;
        *loc = '\0';
        loc2 = memchr(mosq->in_packet.buffer, ' ', mosq->in_packet.bufSize - 1);
        if (!loc2) return -1;
        *loc2 = '\0';
        if ((mosq->remote_port = strtol(loc + 1, NULL, 10)) == 0) {
            return -1;
        }
        /* Shift back buffer for extra data */
        memmove(mosq->in_packet.buffer, end, mosq->in_packet.bufSize - ((uint8_t *)end - mosq->in_packet.buffer));
        mosq->in_packet.bufSize -= (uint8_t *)end - mosq->in_packet.buffer;
	mosq->in_packet.proxy = PROXY_VALID;

	log__printf(mosq, MOSQ_LOG_INFO, "New connection has remote address %s on port %i.", mosq->remote_host, mosq->remote_port);
    }

    return 0;
}
