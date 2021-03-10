#ifndef ICENETETHERNET_H		/* prevent circular inclusions */
#define ICENETETHERNET_H		/* by using protection macros */

#include <stdint.h>
#include "icebase.h"


#define CONFIG_ICENET_MTU 1500
#define CONFIG_ICENET_RING_SIZE 64
// #define CONFIG_ICENET_CHECKSUM
#define CONFIG_ICENET_TX_THRESHOLD 16

#define ICENET_SEND_REQ 0
#define ICENET_RECV_REQ 8
#define ICENET_SEND_COMP 16
#define ICENET_RECV_COMP 18
#define ICENET_COUNTS 20
#define ICENET_MACADDR 24
#define ICENET_INTMASK 32
#define ICENET_TXCSUM_REQ 40
#define ICENET_RXCSUM_RES 48
#define ICENET_CSUM_ENABLE 49
#define ICENET_SHADOW_SEND_REQ_LO 64
#define ICENET_SHADOW_SEND_REQ_HI 68
#define ICENET_SHADOW_RECV_REQ_LO 72
#define ICENET_SHADOW_RECV_REQ_HI 76

#define ICENET_INTMASK_TX 1
#define ICENET_INTMASK_RX 2
#define ICENET_INTMASK_BOTH 3

#define ETH_HEADER_BYTES 14
#define ALIGN_BYTES 8
#define ALIGN_MASK 0x7
#define ALIGN_SHIFT 3
#define NET_IP_ALIGN 0
#define MAX_FRAME_SIZE (CONFIG_ICENET_MTU + ETH_HEADER_BYTES + NET_IP_ALIGN)
#define DMA_PTR_ALIGN(p) ((typeof(p)) (__ALIGN_KERNEL((uintptr_t) (p), ALIGN_BYTES)))
#define DMA_LEN_ALIGN(n) (((((n) - 1) >> ALIGN_SHIFT) + 1) << ALIGN_SHIFT)
#define MACADDR_BYTES 6

#define ICENET_TX_RETRIES 10
#define ICENET_TX_RETRY_DELAY_MS 300

/*
 * Define an aligned data type for an ethernet frame. This declaration is
 * specific to the GNU compiler
 */
typedef uint8_t IceEthernetFrame[MAX_FRAME_SIZE]; // __attribute__ ((aligned(0x40)));
// 0x80000000
static uint8_t * const TxFrameBufRef = (uint8_t *)0x80000000;
// Would be better to not hardcode this value.
// Making it MAX_FRAME_SIZE + 2 (NET_IP_ALIGN), then aligned to nearest 0x40 boundary (0x600)
static uint8_t * const RxFrameBufRef =(uint8_t *)(TxFrameBufRef + CONFIG_ICENET_RING_SIZE * 0x600);


// Bringing some code over from Linux Kernel

/* Return count in buffer.  */
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))

/* Return space available, 0..size-1.  We always leave one free char
   as a completely full buffer has head == tail, which is the same as
   empty.  */
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))

// End Linux Kernel Code

struct sk_buff_cq_entry {
	IceEthernetFrame * data;
};

struct sk_buff_cq {
	struct sk_buff_cq_entry entries[CONFIG_ICENET_RING_SIZE];
	int head;
	int tail;
};

#define SK_BUFF_CQ_COUNT(cq) CIRC_CNT(cq.head, cq.tail, CONFIG_ICENET_RING_SIZE)
#define SK_BUFF_CQ_SPACE(cq) CIRC_SPACE(cq.head, cq.tail, CONFIG_ICENET_RING_SIZE)


/**
 * This typedef contains configuration information for a IceNet Ethernet device.
 */

typedef struct IceNetEthernet {
	UINTPTR BaseAddress; /** HW Base Address **/
	u32 IsStarted;		 /**< Device is currently started */
    struct sk_buff_cq send_cq;
	struct sk_buff_cq recv_cq;
} IceNetEthernet;

int icenet_open(IceNetEthernet *nic);
int icenet_start_xmit(struct IceNetEthernet *nic, IceEthernetFrame *addr, int len);
int icenet_recv(struct IceNetEthernet *nic, IceEthernetFrame **frameAddr, int *len);
void icenet_alloc_recv(struct IceNetEthernet *nic);
int icenet_get_tx_buffer(struct IceNetEthernet *nic, IceEthernetFrame **frameAddr);
int recv_comp_avail(struct IceNetEthernet *nic);
void icenet_complete_send(struct IceNetEthernet *nic);
void icenet_set_intmask(struct IceNetEthernet *nic, uint32_t mask);
void icenet_clear_intmask(struct IceNetEthernet *nic, uint32_t mask);

#endif // ICENETETHERNET_H
