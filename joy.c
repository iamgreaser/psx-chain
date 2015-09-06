volatile uint16_t pad_data_now = 0xFFFF;
volatile uint16_t pad_id_now = 0xFFFF;
volatile int pad_stage = 0;
uint16_t pad_data = 0x0000;
uint16_t pad_id = 0xFFFF;
uint16_t pad_old_data = 0x0000;

void joy_update()
{
	volatile int k;
	volatile int lag;

	k = JOY_RX_DATA;

	switch(pad_stage)
	{
		case 5:
			// Get low input
			pad_data_now = (pad_data_now & 0x00FF) | (((int)k)<<8);

			// Restart transfer
			JOY_CTRL = 0x0000;
			pad_stage = 0-1;
			break;
		default:
			pad_stage = 0;
			// FALL THROUGH
		case 0:
			JOY_TX_DATA = 0x01;
			break;
		case 1:
			JOY_TX_DATA = 0x42;
			break;
		case 2:
			pad_id_now = (pad_id_now & 0xFF00) | (((int)k)<<0);
			JOY_TX_DATA = 0x00;
			break;
		case 3:
			pad_id_now = (pad_id_now & 0x00FF) | (((int)k)<<8);
			JOY_TX_DATA = 0x00;
			break;
		case 4:
			pad_data_now = (pad_data_now & 0xFF00) | (((int)k)<<0);
			JOY_CTRL = 0x0803;
			JOY_TX_DATA = 0x00;
			break;
	}

	pad_stage++;
}

void joy_poll(void)
{
	volatile int lag;

	if(pad_stage == 5 && (JOY_STAT & 0x0002) != 0)
	//if((JOY_STAT & 0x0002) != 0)
	{
		joy_update();
	}

	if(pad_stage == 0)
	{
		JOY_CTRL = 0x1003;
		for(lag = 0; lag < 30; lag++) {}
		joy_update();
	}
}

void joy_init(void)
{
	// Set up parameters
	JOY_CTRL = 0x0000;
	JOY_MODE = 0x000D;
	JOY_BAUD = 0x0088;

	// Enable joypad interrupt
	I_STAT = ~0x0080;
	I_MASK |=  0x0080;
	JOY_CTRL |= 0x0010;

	// Begin joypad read
	joy_poll();
}



