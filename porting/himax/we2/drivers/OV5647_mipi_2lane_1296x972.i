	{HX_CIS_I2C_Action_W, 0x3000, 0x0f},
	{HX_CIS_I2C_Action_W, 0x3001, 0xff},
	{HX_CIS_I2C_Action_W, 0x3002, 0xe4},
	{HX_CIS_I2C_Action_S, 0x0040, 0x00},  // delay 64ms
	{HX_CIS_I2C_Action_W, 0x0100, 0x00},
	{HX_CIS_I2C_Action_W, 0x0103, 0x01},
	{HX_CIS_I2C_Action_W, 0x3034, 0x1a},
	{HX_CIS_I2C_Action_W, 0x3035, 0x21},
	{HX_CIS_I2C_Action_W, 0x3036, 0x62},
	{HX_CIS_I2C_Action_W, 0x303c, 0x11},
	{HX_CIS_I2C_Action_W, 0x3106, 0xf5},
	{HX_CIS_I2C_Action_W, 0x3827, 0xec},
	{HX_CIS_I2C_Action_W, 0x370c, 0x03},
	{HX_CIS_I2C_Action_W, 0x3612, 0x59},
	{HX_CIS_I2C_Action_W, 0x3618, 0x00},
	{HX_CIS_I2C_Action_W, 0x5000, 0x89},
	{HX_CIS_I2C_Action_W, 0x5002, 0x41},
	{HX_CIS_I2C_Action_W, 0x5003, 0x08},
	{HX_CIS_I2C_Action_W, 0x5a00, 0x08},
	{HX_CIS_I2C_Action_W, 0x3000, 0x00},
	{HX_CIS_I2C_Action_W, 0x3001, 0x00},
	{HX_CIS_I2C_Action_W, 0x3002, 0x00},
	{HX_CIS_I2C_Action_W, 0x3016, 0x08},
	{HX_CIS_I2C_Action_W, 0x3017, 0xe0},
	{HX_CIS_I2C_Action_W, 0x3018, 0x44},
	{HX_CIS_I2C_Action_W, 0x301c, 0xf8},
	{HX_CIS_I2C_Action_W, 0x301d, 0xf0},
	{HX_CIS_I2C_Action_W, 0x3a18, 0x00},
	{HX_CIS_I2C_Action_W, 0x3a19, 0xf8},
	{HX_CIS_I2C_Action_W, 0x3c01, 0x80},
	{HX_CIS_I2C_Action_W, 0x3b07, 0x0c},
	{HX_CIS_I2C_Action_W, 0x3800, 0x00},
	{HX_CIS_I2C_Action_W, 0x3801, 0x00},
	{HX_CIS_I2C_Action_W, 0x3802, 0x00},
	{HX_CIS_I2C_Action_W, 0x3803, 0x00},
	{HX_CIS_I2C_Action_W, 0x3804, 0x0a},
	{HX_CIS_I2C_Action_W, 0x3805, 0x3f},
	{HX_CIS_I2C_Action_W, 0x3806, 0x07},
	{HX_CIS_I2C_Action_W, 0x3807, 0xa3},
	{HX_CIS_I2C_Action_W, 0x3808, 0x05},
	{HX_CIS_I2C_Action_W, 0x3809, 0x10},
	{HX_CIS_I2C_Action_W, 0x380a, 0x03},
	{HX_CIS_I2C_Action_W, 0x380b, 0xcc},

	// for 4x quant
	// {HX_CIS_I2C_Action_W, 0x380c, 0x0c}, // 0x0a 4:0 12:8 H
	// {HX_CIS_I2C_Action_W, 0x380d, 0x68}, // 0x8c 7:0 7:0  H

	{HX_CIS_I2C_Action_W, 0x380c, 0x0a}, // 0x0a 4:0 12:8 H
	{HX_CIS_I2C_Action_W, 0x380d, 0x8c}, // 0x8c 7:0 7:0  H

	{HX_CIS_I2C_Action_W, 0x3811, 0x0c},
	{HX_CIS_I2C_Action_W, 0x3813, 0x06},
	{HX_CIS_I2C_Action_W, 0x3814, 0x31},
	{HX_CIS_I2C_Action_W, 0x3815, 0x31},
	{HX_CIS_I2C_Action_W, 0x3630, 0x2e},
	{HX_CIS_I2C_Action_W, 0x3632, 0xe2},
	{HX_CIS_I2C_Action_W, 0x3633, 0x23},
	{HX_CIS_I2C_Action_W, 0x3634, 0x44},
	{HX_CIS_I2C_Action_W, 0x3636, 0x06},
	{HX_CIS_I2C_Action_W, 0x3620, 0x64},
	{HX_CIS_I2C_Action_W, 0x3621, 0xe0},
	{HX_CIS_I2C_Action_W, 0x3600, 0x37},
	{HX_CIS_I2C_Action_W, 0x3704, 0xa0},
	{HX_CIS_I2C_Action_W, 0x3703, 0x5a},
	{HX_CIS_I2C_Action_W, 0x3715, 0x78},
	{HX_CIS_I2C_Action_W, 0x3717, 0x01},
	{HX_CIS_I2C_Action_W, 0x3731, 0x02},
	{HX_CIS_I2C_Action_W, 0x370b, 0x60},
	{HX_CIS_I2C_Action_W, 0x3705, 0x1a},
	{HX_CIS_I2C_Action_W, 0x3f05, 0x02},
	{HX_CIS_I2C_Action_W, 0x3f06, 0x10},
	{HX_CIS_I2C_Action_W, 0x3f01, 0x0a},
	{HX_CIS_I2C_Action_W, 0x3a08, 0x01},
	{HX_CIS_I2C_Action_W, 0x3a09, 0x28},
	{HX_CIS_I2C_Action_W, 0x3a0a, 0x00},
	{HX_CIS_I2C_Action_W, 0x3a0b, 0xf6},
	{HX_CIS_I2C_Action_W, 0x3a0d, 0x08},

	{HX_CIS_I2C_Action_W, 0x3a0e, 0x06}, // 0x07 1:0 9:8 V

	// LENC GAIN
	{HX_CIS_I2C_Action_W, 0x583E, 0xFF}, //MAX 
	{HX_CIS_I2C_Action_W, 0x583F, 0x80}, // MIN

	// AEC/AGC Area
	{HX_CIS_I2C_Action_W, 0x3a0f, 0x30}, 
	{HX_CIS_I2C_Action_W, 0x3a10, 0x20},
	{HX_CIS_I2C_Action_W, 0x3a1b, 0x30},
	{HX_CIS_I2C_Action_W, 0x3a1e, 0x20},

	// AGC/AEC Average Area & weight
	{HX_CIS_I2C_Action_W, 0x5680, 0x00},
	{HX_CIS_I2C_Action_W, 0x5681, 0x00},
	{HX_CIS_I2C_Action_W, 0x5682, 0x00},
	{HX_CIS_I2C_Action_W, 0x5683, 0x00},
	// center weight for AEC 
	{HX_CIS_I2C_Action_W, 0x568A, 0x21},
	{HX_CIS_I2C_Action_W, 0x588B, 0x12},
	{HX_CIS_I2C_Action_W, 0x588C, 0x21},
	{HX_CIS_I2C_Action_W, 0x588D, 0x12},


	{HX_CIS_I2C_Action_W, 0x3a11, 0x60},
	{HX_CIS_I2C_Action_W, 0x3a1f, 0x28},
	{HX_CIS_I2C_Action_W, 0x4001, 0x02},
	{HX_CIS_I2C_Action_W, 0x4004, 0x04},
	{HX_CIS_I2C_Action_W, 0x4000, 0x09},
	{HX_CIS_I2C_Action_W, 0x4837, 0x16},
	{HX_CIS_I2C_Action_W, 0x4800, 0x24},
	{HX_CIS_I2C_Action_W, 0x3503, 0x00},

	{HX_CIS_I2C_Action_W, 0x3820, 0x41},
	{HX_CIS_I2C_Action_W, 0x3821, 0x00}, // LR flip

	{HX_CIS_I2C_Action_W, 0x350a, 0x00},
	{HX_CIS_I2C_Action_W, 0x350b, 0x10},
	{HX_CIS_I2C_Action_W, 0x3500, 0x00},
	{HX_CIS_I2C_Action_W, 0x3501, 0x1a},
	{HX_CIS_I2C_Action_W, 0x3502, 0xf0},
	{HX_CIS_I2C_Action_W, 0x3212, 0xa0},
	{HX_CIS_I2C_Action_W, 0x4800, OV5647_MIPI_CTRL_OFF},
	{HX_CIS_I2C_Action_W, 0x4202, 0x0F},
	{HX_CIS_I2C_Action_W, 0x0100, 0x01},