
#define DMA_QUEUE_OT_POW (10)
#define DMA_QUEUE_OT (1<<DMA_QUEUE_OT_POW)
#define DMA_QUEUE_OT_MASK (DMA_QUEUE_OT-1)
#define DMA_QUEUE_MAX_POW (12 + 3)
#define DMA_QUEUE_MAX (1<<DMA_QUEUE_MAX_POW)
#define DMA_QUEUE_MAX_MASK (DMA_QUEUE_MAX-1)

uint32_t dma_queue[DMA_QUEUE_MAX];
uint32_t dma_ot = 0; // start of order table
uint32_t dma_run = 0; // start of execution
uint32_t dma_end = 0; // end of queue
uint32_t *dma_post_tail = NULL; // for non-depth-tested stuff

void dma_init_block(void)
{
	int i;

	// Advance end
	dma_end += DMA_QUEUE_OT+1;
	if(dma_end >= DMA_QUEUE_MAX)
		dma_end = DMA_QUEUE_OT+1;

	// Set up order table
	// XXX: investigate into possibly making DMA6 do this
	dma_run = dma_end-1;
	dma_ot = dma_end-DMA_QUEUE_OT;
	dma_queue[dma_ot-1] = 0x00FFFFFF;
	dma_post_tail = &dma_queue[dma_ot-1];
	for(i = 0; i < DMA_QUEUE_OT; i++)
		dma_queue[dma_ot+i] = 0x00FFFFFF&(uint32_t)&dma_queue[dma_ot+i-1];
}

void dma_init(void)
{
	// Set up DMA2
	DMA_DPCR &= ~0xF00;
	DMA_n_CHCR(2) = 0x00000401;
	DMA_n_BCR(2) = 0;
	DMA_DPCR |=  0x800;

	// Set up block + order table
	dma_init_block();
}

void dma_wait(void)
{
	while((DMA_n_CHCR(2) & 0x01000000) != 0) {}
	while((GP1 & 0x10000000) == 0) {}
}

void dma_flush(void)
{
	// TODO: queue these flush requests
	// Wait until finished
	dma_wait();

	// Send DMA data
	DMA_n_MADR(2) = 0x00FFFFFF & (uint32_t)&dma_queue[dma_run];
	DMA_n_CHCR(2) |= 0x01000000;

	// Create new block
	dma_init_block();
}

void dma_send_prim(uint32_t count, uint32_t *data, int32_t otz)
{
	int i;

	// Advance end
	dma_end += count+1;
	if(dma_end >= DMA_QUEUE_MAX)
		dma_end = count+1;

	// Copy data into DMA
	memcpy(&dma_queue[dma_end-count], data, sizeof(uint32_t)*count);

	// Chain
	if(otz < 0)
	{
		// Painters algorithm, draw in front
		dma_queue[dma_end-(count+1)] = (0x00FFFFFF) | (count<<24);
		*dma_post_tail = (*dma_post_tail & 0xFF000000)
			| ((uint32_t)&dma_queue[dma_end-(count+1)] & 0x00FFFFFF);
		dma_post_tail = &dma_queue[dma_end-(count+1)];

	} else {
		// Order table
		otz &= DMA_QUEUE_MAX_MASK;
		dma_queue[dma_end-(count+1)] = dma_queue[dma_ot+otz] | (count<<24);
		dma_queue[dma_ot+otz] = (0x00FFFFFF & (uint32_t)&dma_queue[dma_end-(count+1)]);

	}


	// TODO: shove this into DMA
	/*
	gpu_send_control_gp0(data[0]);
	for(i = 1; i < count; i++)
		gpu_send_data(data[i]);
	*/

}

