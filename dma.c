
#define DMA_QUEUE_MAX_POW (12 + 3)
#define DMA_QUEUE_MAX (1<<DMA_QUEUE_MAX_POW)
#define DMA_QUEUE_MAX_MASK (DMA_QUEUE_MAX-1)

uint32_t dma_queue_head = 0;
uint32_t dma_queue_tail = 0;
uint32_t dma_queue[DMA_QUEUE_MAX];
void dma_send_prim(uint32_t count, uint32_t *data)
{
	int i;

	// TODO: shove this into DMA
	gpu_send_control_gp0(data[0]);
	for(i = 1; i < count; i++)
		gpu_send_data(data[i]);

}

