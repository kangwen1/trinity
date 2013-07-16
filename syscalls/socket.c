/*
 * SYSCALL_DEFINE3(socket, int, family, int, type, int, protocol)
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/irda.h>
#include <linux/dn.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include "compat.h"
#include "log.h"
#include "net.h"
#include "random.h"
#include "sanitise.h"
#include "shm.h"
#include "config.h"
#include "params.h"

/* note: also called from generate_sockets() & sanitise_socketcall() */
void sanitise_socket(int childno)
{
	unsigned long family, type, protocol;
	struct proto_type pt;

	if (do_specific_proto == TRUE)
		family = specific_proto;
	else
		family = rand() % TRINITY_PF_MAX;

	type = rand() % TYPE_MAX;
	protocol = rand() % PROTO_MAX;

	switch (family) {

	case AF_APPLETALK:
		appletalk_rand_socket(&pt);
		type = pt.type;
		protocol = pt.protocol;
		break;

	case AF_AX25:
		ax25_rand_socket(&pt);
		type = pt.type;
		protocol = pt.protocol;
		break;

#ifdef USE_CAIF
	case AF_CAIF:
		caif_rand_socket(&pt);
		type = pt.type;
		protocol = pt.protocol;
		break;
#endif

	case AF_CAN:
		can_rand_socket(&pt);
		type = pt.type;
		protocol = pt.protocol;
		break;

	case AF_DECnet:
		decnet_rand_socket(&pt);
		type = pt.type;
		protocol = pt.protocol;
		break;

	case AF_INET:
		inet_rand_socket(&pt);
		type = pt.type;
		protocol = pt.protocol;
		break;

	case AF_INET6:
		inet6_rand_socket(&pt);
		type = pt.type;
		protocol = pt.protocol;
		break;

	case AF_IPX:
		type = SOCK_DGRAM;
		break;

	case AF_IRDA:
		switch (rand() % 3) {
		case 0:	type = SOCK_STREAM;
			break;
		case 1:	type = SOCK_SEQPACKET;
			break;
		case 2:	type = SOCK_DGRAM;
			switch (rand_bool()) {
			case 0: protocol = IRDAPROTO_ULTRA;
				break;
			case 1: protocol = IRDAPROTO_UNITDATA;
				break;
			default:break;
			}
			break;
		default:break;
		}
		break;

	case AF_LLC:
		switch (rand_bool()) {
		case 0:	type = SOCK_STREAM;
			break;
		case 1:	type = SOCK_DGRAM;
		default:break;
		}
		break;

	//TODO;
/*	case AF_IB:
		break;
*/
	case AF_NETLINK:
		switch (rand_bool()) {
		case 0:	type = SOCK_RAW;
			break;
		case 1:	type = SOCK_DGRAM;
		default:break;
		}
		protocol = rand() % (NETLINK_CRYPTO + 1);	// Current highest netlink socket.
		break;

	case AF_NFC:
		switch (rand_bool()) {
		case 0:	protocol = NFC_SOCKPROTO_LLCP;
			switch (rand() % 2) {
			case 0:	type = SOCK_DGRAM;
				break;
			case 1:	type = SOCK_STREAM;
				break;
			default: break;
			}
			break;

		case 1:	protocol = NFC_SOCKPROTO_RAW;
			type = SOCK_SEQPACKET;
			break;
		default:
			BUG("impossible.");
		}
		break;

	case AF_PACKET:
		protocol = htons(ETH_P_ALL);
		if (rand() % 8 == 0) {
			protocol = rand();
			if (rand_bool())
				protocol = (uint16_t) rand();
		}
		switch (rand() % 3) {
		case 0:	type = SOCK_DGRAM;
			break;
		case 1:	type = SOCK_RAW;
			break;
		case 2:	type = SOCK_PACKET;
			break;
		default: break;
		}
		break;

	case AF_PHONET:
		protocol = 0;
		switch (rand_bool()) {
		case 0:	type = SOCK_DGRAM;
			break;
		case 1:	type = SOCK_SEQPACKET;
			break;
		default: break;
		}
		break;

	case AF_RDS:
		protocol = 0;
		type = SOCK_SEQPACKET;
		break;

	case AF_TIPC:
		protocol = 0;
		switch (rand() % 3) {
		case 0:	type = SOCK_STREAM;
			break;
		case 1:	type = SOCK_SEQPACKET;
			break;
		case 2:	type = SOCK_DGRAM;
			break;
		default: break;
		}
		break;

	case AF_UNIX:
		protocol = PF_UNIX;
		switch (rand() % 3) {
		case 0:	type = SOCK_STREAM;
			break;
		case 1:	type = SOCK_DGRAM;
			break;
		case 2:	type = SOCK_SEQPACKET;
			break;
		default:break;
		}
		break;

	case AF_X25:
		type = SOCK_SEQPACKET;
		protocol = 0;
		break;

	default:
		switch (rand() % 6) {
		case 0:	type = SOCK_DGRAM;	break;
		case 1:	type = SOCK_STREAM;	break;
		case 2:	type = SOCK_SEQPACKET;	break;
		case 3:	type = SOCK_RAW;	break;
		case 4:	type = SOCK_RDM;	break;
		case 5:	type = SOCK_PACKET;	break;
		default: break;
		}

		break;
	}

	if ((rand() % 100) < 25)
		type |= SOCK_CLOEXEC;
	if ((rand() % 100) < 25)
		type |= SOCK_NONBLOCK;

	shm->a1[childno] = family;
	shm->a2[childno] = type;
	shm->a3[childno] = protocol;
}

struct syscall syscall_socket = {
	.name = "socket",
	.num_args = 3,
	.arg1name = "family",
	.arg2name = "type",
	.arg3name = "protocol",
	.sanitise = sanitise_socket,
};
