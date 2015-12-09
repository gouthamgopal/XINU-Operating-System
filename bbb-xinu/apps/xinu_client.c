#include <xinu.h>
#include <stdlib.h>

int netserver()
{
	int val = -1;
	int	i;			/* index into buffer		*/
	int	retval;			/* return value			*/
	/* message to send	*/
	char	msg[] = "Sending message to LINUX Server"; 
	char	inbuf[1500];		/* buffer for incoming reply	*/
	int32	slot;			/* UDP slot to use		*/
	int32	msglen;			/* length of outgoing message	*/
	uint16	echoport= 8888;		/* port number for UDP echo	*/
	uint16	locport	= 52743;	/* local port to use		*/
	int32	retries	= 3;		/* number of retries		*/
	int32	delay	= 2000;		/* reception delay in ms	*/
	uint32 rmtIp; 			/* remote IP address to use	*/


	dot2ip("192.168.1.100",&rmtIp);

	// register UDP port to the udp table
	slot = udp_register(rmtIp, echoport, locport);
	if (slot == SYSERR) {
		return 1;
	}

	msglen = strnlen(msg, 1200);

	for (i=0; i<retries; i++) {
		retval = udp_send(slot, msg, msglen);
		if (retval == SYSERR) {
			return 1;
		}

		retval = udp_recv(slot, inbuf, sizeof(inbuf), delay);

		//printf("RetVal : %d",retval);
		//printf("Timeout : %d",TIMEOUT);
		//printf("Timeout : %d",SYSERR);
		if (retval == TIMEOUT) {	
			//printf("RetVal : %d",retval);
			continue;
		} else if (retval == SYSERR) {
			udp_release(slot);	//release from the udp table
			return 1;
		}
		break;
	}

	udp_release(slot);
	if (retval == TIMEOUT) {	
		return 1;
	}
	
	// Check received packet
	if (retval != msglen) {	
		return 1;
	}
	
	printf("Data Received : %s \n",inbuf);
	val = atoi(inbuf);
	
	return val;
}
