/* clkhandler.c - clkhandler */

#include <xinu.h>

int clear_arp(void);
struct arpentry arpcache[ARP_SIZ];

/*-----------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *-----------------------------------------------------------------------
 */
void	clkhandler()
{

	static uint32 count1000 = 1000;
	volatile struct am335x_timer1ms *csrptr = 0x44E31000;

	if((csrptr->tisr & AM335X_TIMER1MS_TISR_OVF_IT_FLAG) == 0) {
		return;
	}

	csrptr->tisr = AM335X_TIMER1MS_TISR_OVF_IT_FLAG;

	count1000--;

	if(count1000 == 0) {
		clktime++;
		remove_arp_cache_entry();
		count1000 = 1000;
	}

	if(!isempty(sleepq)) {
		if((--queuetab[firstid(sleepq)].qkey) == 0) {
			wakeup();
		}
	}

	if((--preempt) == 0) {
		preempt = QUANTUM;
		resched();
	}
}

/*
The remove_arp_cache_entry() functions clears the ARP entries after ARP_TIMEOUT.
ARP_TIMEOUT defined in arp.h
*/

int remove_arp_cache_entry(void)
{
	int32 arp_index;
	for(arp_index=0;arp_index<ARP_SIZ;arp_index++){
		if(arpcache[arp_index].arstate != AR_FREE){  
			if((clktime - arpcache[arp_index].time) >= ARP_TIMEUP){
				arpcache[arp_index].arstate = AR_FREE;           
			}
        	}
 	}
    return 1;
}

