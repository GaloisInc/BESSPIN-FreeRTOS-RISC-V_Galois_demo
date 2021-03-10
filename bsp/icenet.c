#include "icenet.h"

// This driver has been adapted from the original Linux driver available here:
// https://github.com/firesim/icenet-driver/blob/master/icenet.c

static inline int send_req_avail(IceNetEthernet *nic)
{
	return ioread32(nic->BaseAddress + ICENET_COUNTS) & 0xff;
}

static inline int recv_req_avail(struct IceNetEthernet *nic)
{
	return (ioread32(nic->BaseAddress+ ICENET_COUNTS) >> 8) & 0xff;
}

static inline int send_comp_avail(struct IceNetEthernet *nic)
{
	return (ioread32(nic->BaseAddress + ICENET_COUNTS) >> 16) & 0xff;
}

int recv_comp_avail(struct IceNetEthernet *nic)
{
	return (ioread32(nic->BaseAddress + ICENET_COUNTS) >> 24) & 0xff;
}

void icenet_set_intmask(struct IceNetEthernet *nic, uint32_t mask)
{
    // This could / should be done with atomic instructions if available
    int val = ioread32(nic->BaseAddress + ICENET_INTMASK);
    iowrite32(val | mask, nic->BaseAddress + ICENET_INTMASK);
}

void icenet_clear_intmask(struct IceNetEthernet *nic, uint32_t mask)
{
    // This could / should be done with atomic instructions if available
    int val = ioread32(nic->BaseAddress + ICENET_INTMASK);
    iowrite32(val & ~mask, nic->BaseAddress + ICENET_INTMASK);
}

static inline IceEthernetFrame * sk_buff_cq_push(struct sk_buff_cq *cq)
{
	IceEthernetFrame *val = cq->entries[cq->head].data;
	cq->head = (cq->head + 1) & (CONFIG_ICENET_RING_SIZE - 1);
    return val;
}

static inline IceEthernetFrame *sk_buff_cq_pop(struct sk_buff_cq *cq)
{
	IceEthernetFrame *val = cq->entries[cq->tail].data;
	cq->tail = (cq->tail + 1) & (CONFIG_ICENET_RING_SIZE - 1);

	return val;
}

static inline void post_recv(struct IceNetEthernet *nic)
{
	IceEthernetFrame *addr = sk_buff_cq_push(&nic->recv_cq);
	iowrite64((uint32_t) addr, nic->BaseAddress + ICENET_SHADOW_RECV_REQ_LO);
}

void icenet_alloc_recv(struct IceNetEthernet *nic)
{
	int hw_recv_cnt = recv_req_avail(nic);
	int sw_recv_cnt = SK_BUFF_CQ_SPACE(nic->recv_cq);
	int recv_cnt = (hw_recv_cnt < sw_recv_cnt) ? hw_recv_cnt : sw_recv_cnt;

	for ( ; recv_cnt > 0; recv_cnt--) {
		post_recv(nic);
	}
}

static inline int send_space(struct IceNetEthernet *nic, int nfrags)
{
	return (send_req_avail(nic) >= nfrags) &&
		(SK_BUFF_CQ_SPACE(nic->send_cq) > 0);
}

int icenet_get_tx_buffer(struct IceNetEthernet *nic, IceEthernetFrame **frameAddr) {
    *frameAddr = nic->send_cq.entries[nic->send_cq.head].data;
    return send_space(nic, 1);
}


void icenet_complete_send(struct IceNetEthernet *nic)
{
	int n, nsegs;
    nsegs = 1;

	for (n = send_comp_avail(nic); n > 0; n -= nsegs) {
		// assert(SK_BUFF_CQ_COUNT(nic->send_cq) != 0);
		ioread16(nic->BaseAddress + ICENET_SEND_COMP);
		sk_buff_cq_pop(&nic->send_cq);
	}
}

int icenet_recv(struct IceNetEthernet *nic, IceEthernetFrame **frameAddr, int *len)
{
    // Not sure this is a great place to do this, but sends need to be "completed"
    // by reading from the send completion register. Adding it here now so everything
    // works and the NIC doesn't backup, but this may make more sense elsewhere
    icenet_complete_send(nic);

	int i;

    // Currently hardcoding this to receive a single frame. This chould be improved to
    // return an array of received frames for faster processing, but benefit is unclear
	for (i = 0; i < 1; i++) {
		*len = ioread16(nic->BaseAddress + ICENET_RECV_COMP);
		*frameAddr = sk_buff_cq_pop(&nic->recv_cq);
	}

	return 1;
}

static inline void post_send(
		struct IceNetEthernet *nic, IceEthernetFrame *addr, int len)
{
	uint64_t packet;

	packet = (((uint64_t) len) << 48) | (((uint32_t) addr) & 0xffffffffffffL);
	iowrite64(packet, nic->BaseAddress + ICENET_SHADOW_SEND_REQ_LO);
	
	sk_buff_cq_push(&nic->send_cq);
}

int icenet_start_xmit(struct IceNetEthernet *nic, IceEthernetFrame *addr, int len)
{
    // This function now assumes the caller has properly placed the network data in to the
    // data space currently referenced at the head of send_cq    
	post_send(nic, addr, len);
    return 0;
}

int icenet_open(struct IceNetEthernet *nic)
{
    // Simple routine to fill in addresses in the two circular buffers
    for (int i = 0; i < CONFIG_ICENET_RING_SIZE; i=i+1) {
        // Would be better to not hardcode this value.
        // Making it MAX_FRAME_SIZE + 2 (NET_IP_ALIGN), then aligned to nearest 0x40 boundary (0x600)
        nic->recv_cq.entries[i].data = (IceEthernetFrame *) (RxFrameBufRef + i*0x600);
        nic->send_cq.entries[i].data = (IceEthernetFrame *) (TxFrameBufRef + i*0x600);
    }

	icenet_alloc_recv(nic);
	icenet_set_intmask(nic, ICENET_INTMASK_RX);
	printf("IceNet: opened device\n");

	return 0;
}
