/*
 * BU6424AF voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#include "lens_info.h"


#define AF_DRVNAME "BU6424AF_DRV"
#define AF_I2C_SLAVE_ADDR        0x7C

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...) pr_debug(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif


#define OIS_7B 1
#define OIS_5B 2
static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;

static __u32 rs_flag;
static bool OIS_status=1;
static unsigned long g_u4AF_INF;
//<2015/10/29-kylechang, AF range 0~511
static unsigned long g_u4AF_MACRO = 511;
//>2015/10/29-kylechang
static unsigned long g_u4TargetPosition;
static unsigned long g_u4CurrPosition;
static int OIS_DATA_FLAG=0;


u8 bu24239_dl_data1[952] ={	
0x46,	0x0F,	0xB5,	0xFE,	0x21,	0x0,	0x4C,	0x46,	0x4D,	0x45,	0x4E,	0x44,	0x91,	0x0,	0x34,	0xB4,
0xDA,	0x5E,	0x28,	0x0,	0x18,	0x40,	0x49,	0x43,	0x28,	0x1,	0xD0,	0x54,	0x28,	0x4,	0xD0,	0x54,
0xF0,	0x0,	0xD1,	0x7A,	0x2F,	0x0,	0xF9,	0x31,	0x48,	0x3F,	0xD0,	0x6,	0x60,	0x1,	0x6B,	0x79,
0x60,	0x41,	0x6B,	0xB9,	0x60,	0x81,	0x6B,	0x39,	0x21,	0x20,	0x48,	0x3C,	0x21,	0x2,	0x81,	0x1,
0x47,	0xB0,	0x20,	0x1,	0x6,	0xC9,	0x21,	0x19,	0x49,	0x39,	0x63,	0xC8,	0x61,	0x8,	0x20,	0x0,
0x49,	0x38,	0x48,	0x37,	0x61,	0x81,	0x38,	0xC0,	0x78,	0x7,	0x48,	0x37,	0x20,	0x1,	0x21,	0x0C,
0x7,	0x1,	0x47,	0xB0,	0x6,	0x0,	0x0F,	0x9,	0x91,	0x1,	0x0F,	0x0,	0xD1,	0x53,	0x28,	0x2,
0xD0,	0x4,	0x2F,	0x13,	0xD0,	0x6,	0x2F,	0x41,	0xD0,	0x9,	0x2F,	0x42,	0x4C,	0x2F,	0xE0,	0x0C,
0x34,	0xB4,	0x4D,	0x2E,	0x4D,	0x2D,	0xE0,	0x8,	0x35,	0x8,	0x4C,	0x2C,	0xE0,	0x3,	0x34,	0xBC,
0x4C,	0x2A,	0x4D,	0x23,	0x3C,	0x84,	0x35,	0x14,	0x20,	0x1,	0x21,	0x0D,	0x49,	0x22,	0x47,	0xB0,
0x47,	0x88,	0x68,	0x89,	0x47,	0xA8,	0x20,	0x0B,	0x47,	0xA0,	0x20,	0x13,	0x47,	0xA8,	0x20,	0x0C,
0x47,	0xA0,	0x20,	0x80,	0x28,	0x1,	0x98,	0x1,	0x20,	0x1F,	0xD1,	0x2E,	0x20,	0x8,	0x47,	0xA8,
0xE0,	0x29,	0x47,	0xA0,	0xE0,	0x0,	0x49,	0x1E,	0x46,	0x38,	0x49,	0x1E,	0x90,	0x0,	0x47,	0x88,
0x23,	0xFF,	0xE0,	0x22,	0x1A,	0xC2,	0x33,	0x4A,	0xD0,	0x10,	0x42,	0x98,	0x28,	0xFC,	0xDC,	0x8,
0x28,	0xFD,	0xD0,	0x14,	0x32,	0x0A,	0xD0,	0x11,	0x2A,	0x9,	0xD0,	0x10,	0xE0,	0x0D,	0xD1,	0x14,
0xD0,	0x0B,	0x2A,	0x1,	0xD0,	0x4,	0x2A,	0x2,	0xD1,	0x0D,	0x2A,	0x15,	0x49,	0x12,	0xE0,	0x3,
0x49,	0x12,	0xE0,	0x4,	0x49,	0x12,	0xE0,	0x2,	0x49,	0x12,	0xE0,	0x0,	0x54,	0x17,	0x4A,	0x0A,
0xD0,	0x1,	0x29,	0x0,	0x47,	0x88,	0xB2,	0xF8,	0xBD,	0xFE,	0x98,	0x0,	0x0,	0x0,	0x6,	0xF1,
0x0,	0x0,	0x29,	0xD9,	0x7F,	0xFF,	0xFF,	0xD6,	0x10,	0x0,	0x3,	0xA8,	0x10,	0x0,	0x1A,	0x80,
0x50,	0x0,	0x60,	0xC0,	0x30,	0x48,	0x30,	0x48,	0x10,	0x0,	0x19,	0x40,	0x0,	0x0,	0x2B,	0x25,
0x10,	0x0,	0x1,	0xAB,	0x10,	0x0,	0x1,	0x5D,	0x10,	0x0,	0x2,	0x99,	0x10,	0x0,	0x3,	0x5,
0x10,	0x0,	0x3,	0x51,	0x10,	0x0,	0x3,	0x8D,	0x47,	0x70,	0xBA,	0x40,	0x4B,	0x2E,	0xB5,	0x0,
0x68,	0xD9,	0xB0,	0x87,	0xD0,	0x1,	0x28,	0x0,	0xD1,	0x1B,	0x28,	0x4,	0xD1,	0x19,	0x29,	0x1,
0x7,	0x89,	0x20,	0x0,	0x7,	0x89,	0x68,	0x9,	0x49,	0x28,	0xD0,	0x0E,	0x49,	0x28,	0x91,	0x4,
0x90,	0x1,	0x91,	0x5,	0x90,	0x2,	0x49,	0x27,	0x90,	0x3,	0x91,	0x6,	0x22,	0x1,	0x68,	0x1B,
0xA8,	0x4,	0xA9,	0x1,	0xE0,	0x5,	0x47,	0x98,	0x61,	0x8,	0x49,	0x23,	0x63,	0x88,	0x49,	0x23,
0x63,	0x88,	0x49,	0x23,	0xB0,	0x7,	0x20,	0x0,	0xB5,	0x70,	0xBD,	0x0,	0x4C,	0x21,	0x49,	0x22,
0xB0,	0x88,	0x78,	0x9,	0x0F,	0x49,	0x7,	0x49,	0x28,	0x1,	0x4D,	0x17,	0xD1,	0x26,	0x60,	0xE9,
0xD0,	0x1,	0x29,	0x0,	0xD1,	0x22,	0x29,	0x4,	0x47,	0xA0,	0x20,	0x0E,	0x20,	0x0C,	0x46,	0x6,
0x4,	0x0,	0x47,	0xA0,	0x21,	0x1,	0x43,	0x30,	0x68,	0x9,	0x7,	0x89,	0x21,	0x0,	0x7,	0x8A,
0xD0,	0x0E,	0x2A,	0x0,	0x92,	0x4,	0x4A,	0x0D,	0x92,	0x5,	0x4A,	0x0D,	0x92,	0x6,	0x4A,	0x0D,
0x91,	0x3,	0x91,	0x2,	0x68,	0x2B,	0x90,	0x1,	0xA9,	0x1,	0x22,	0x3,	0x47,	0x98,	0xA8,	0x4,
0x4A,	0x9,	0xE0,	0x5,	0x48,	0x9,	0x61,	0x10,	0x48,	0x9,	0x63,	0x81,	0x20,	0x0,	0x63,	0x81,
0xBD,	0x70,	0xB0,	0x8,	0x0,	0x0,	0x47,	0x70,	0x10,	0x0,	0x3,	0xA8,	0x0,	0x0,	0x60,	0xD0,
0x0,	0x0,	0x41,	0x38,	0x0,	0x0,	0x51,	0x38,	0x50,	0x0,	0x60,	0xC0,	0x50,	0x0,	0x41,	0x0,
0x50,	0x0,	0x51,	0x0,	0x0,	0x0,	0x7,	0x7B,	0x10,	0x0,	0x19,	0x60,	0x48,	0x10,	0xB5,	0x70,
0x4E,	0x0F,	0x69,	0xC5,	0x36,	0x98,	0x4B,	0x0E,	0x3B,	0x18,	0x1C,	0x71,	0x1D,	0x1A,	0x1C,	0xCC,
0x1D,	0x10,	0x78,	0x36,	0xD0,	0x11,	0x2E,	0x0,	0x78,	0x9,	0x68,	0x24,	0x6,	0x64,	0x19,	0x64,
0x29,	0x0,	0x0E,	0x64,	0x2C,	0x10,	0xD0,	0x0A,	0x49,	0x6,	0xD1,	0x8,	0x2C,	0x0,	0x7F,	0xCC,
0x68,	0x0,	0xD0,	0x4,	0x60,	0x10,	0x60,	0x18,	0x77,	0xC8,	0x20,	0x0,	0x0,	0x0,	0xBD,	0x70,
0x10,	0x0,	0x17,	0x40,	0x10,	0x0,	0x1A,	0x60,	0x48,	0x1,	0x49,	0x2,	0x47,	0x70,	0x62,	0x48,
0x10,	0x0,	0x2,	0x3D,	0x10,	0x0,	0x1B,	0x48,	0x48,	0x15,	0xB5,	0x7C,	0x89,	0x0,	0x4C,	0x13,
0xFF,	0x5A,	0xF7,	0xFF,	0x14,	0x5,	0x4,	0x0,	0x47,	0xA0,	0x20,	0x2C,	0x14,	0x0,	0x4,	0x0,
0x49,	0x10,	0x43,	0x68,	0x42,	0x88,	0x13,	0x40,	0x43,	0xC9,	0xDC,	0x2,	0xDA,	0x0,	0x42,	0x88,
0xB2,	0x84,	0x46,	0x8,	0x4B,	0x0D,	0x48,	0x0C,	0x68,	0x1B,	0x90,	0x1,	0x46,	0x69,	0x22,	0x1,
0x47,	0x98,	0xA8,	0x1,	0x4B,	0x0A,	0x98,	0x0,	0x4,	0x0,	0x0C,	0x0,	0x90,	0x0,	0x43,	0x20,
0x22,	0x1,	0x68,	0x1B,	0xA8,	0x1,	0x46,	0x69,	0xBD,	0x7C,	0x47,	0x98,	0x0,	0x0,	0x7,	0x7B,
0x10,	0x0,	0x1A,	0x80,	0x0,	0x0,	0x7F,	0xFF,	0x0,	0x0,	0x61,	0x74,	0x10,	0x0,	0x3,	0xAC,
0x10,	0x0,	0x3,	0xA8,	0x48,	0x0F,	0xB5,	0x70,	0x89,	0x40,	0x4C,	0x0D,	0xFF,	0x24,	0xF7,	0xFF,
0x15,	0x85,	0x5,	0x80,	0x47,	0xA0,	0x20,	0x2A,	0x38,	0xFF,	0x38,	0xFF,	0x5,	0x80,	0x38,	0x2,
0x49,	0x9,	0x15,	0x80,	0x42,	0x88,	0x19,	0x40,	0x43,	0xC9,	0xDC,	0x2,	0xDA,	0x0,	0x42,	0x88,
0x30,	0xFF,	0x46,	0x8,	0x49,	0x5,	0x30,	0xFF,	0x60,	0x88,	0x30,	0x2,	0x0,	0x0,	0xBD,	0x70,
0x0,	0x0,	0x7,	0x7B,	0x10,	0x0,	0x1A,	0x80,	0x0,	0x0,	0x1,	0xFF,	0xC8,	0x0,	0x0,	0x40,
0x7,	0xC0,	0xB5,	0x1C,	0xD0,	0x1,	0x0F,	0xC0,	0xE0,	0x0,	0x48,	0x8,	0x4B,	0x9,	0x48,	0x8,
0x68,	0x1B,	0x90,	0x0,	0xA9,	0x1,	0x22,	0x1,	0x47,	0x98,	0x46,	0x68,	0x49,	0x6,	0x98,	0x1,
0x77,	0x0A,	0x0E,	0x2,	0x77,	0x48,	0x0C,	0x0,	0x0,	0x0,	0xBD,	0x1C,	0x0,	0x0,	0x50,	0x54,
0x0,	0x0,	0x40,	0x54,	0x10,	0x0,	0x3,	0xAC,	0x10,	0x0,	0x1A,	0x80,	0x48,	0x4,	0xB5,	0x10,
0xF7,	0xFF,	0x8B,	0x80,	0x5,	0x80,	0xFE,	0xE1,	0x0D,	0x80,	0x49,	0x2,	0xBD,	0x10,	0x60,	0x48,
0x10,	0x0,	0x1A,	0x20,	0xC8,	0x0,	0x1,	0x0,	0x10,	0x0,	0x2,	0x15,	0x10,	0x0,	0x2,	0x15,
0x10,	0x0,	0x2,	0x15,	0x0,	0x0,	0x0,	0x0	

};

u8 bu24239_dl_data2_7B[448] ={	
0x0,	0x0,	0x77,	0x7E,	0x2,	0x0F,	0x24,	0x1B,	0x0,	0x1,	0x0,	0x0,	0x0,	0x66,	0x4,	0x0,
0x2,	0x54,	0x0,	0x0,	0x2,	0x0,	0x2,	0x14,	0x0,	0x1,	0x0,	0x1,	0x8,	0x0,	0x0,	0x2,
0x10,	0x10,	0x10,	0x0,	0x3,	0xEC,	0x3,	0xEC,	0x4,	0x0,	0x4,	0x0,	0x9,	0x23,	0x7,	0x0,
0x0,	0xF0,	0x22,	0x71,	0x2,	0x27,	0x0,	0xF0,	0x83,	0xC0,	0x5,	0x1,	0x2,	0x32,	0x0,	0x2,
0x0,	0xC3,	0x4,	0x0,	0x0,	0x20,	0x0,	0x0,	0xD0,	0x0,	0xD0,	0x0,	0xD0,	0x0,	0xD0,	0x0,
0xA0,	0xA0,	0x50,	0x50,	0x32,	0x32,	0x7,	0x80,	0x4,	0x88,	0x4,	0x0,	0x0,	0x0,	0x0,	0x0,
0x20,	0x0,	0x20,	0x0,	0x4,	0x0,	0x4,	0x0,	0x0,	0x4,	0x0,	0x4,	0x0,	0x0,	0x0,	0x4,
0x0,	0x2,	0x2,	0x0,	0x82,	0xA7,	0x3D,	0x59,	0x82,	0xAD,	0x3D,	0x53,	0x3F,	0xFE,	0x3F,	0xFC,
0x3F,	0xFF,	0x3F,	0xFF,	0x40,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x3E,	0xFC,	0xC1,	0x0A,
0x7F,	0xF8,	0x7F,	0xF8,	0x1A,	0x5E,	0x1,	0xA5,	0x4,	0x65,	0x0,	0xFF,	0x0,	0xD9,	0x14,	0x0,
0x0,	0x2,	0x4,	0x65,	0x1,	0x51,	0x1,	0x0,	0x3F,	0xF2,	0x3F,	0xF2,	0x3F,	0xE0,	0x7F,	0xFF,
0x4,	0x65,	0x4,	0x65,	0x1,	0x0,	0x11,	0x9A,	0x60,	0x38,	0xA0,	0x8C,	0x7F,	0x90,	0x25,	0xD0,
0xB6,	0x26,	0x24,	0x1E,	0x7F,	0x24,	0x81,	0xA4,	0x3D,	0xE3,	0x85,	0x0E,	0x3D,	0xE3,	0x7A,	0xA7,
0x89,	0x8,	0x6F,	0x40,	0xF0,	0x0,	0xF0,	0x0,	0xF0,	0x0,	0xF0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x10,	0x0,	0x10,	0x0,	0x1,	0x18,	0x10,	0x0,
0x0,	0x30,	0x0,	0x40,	0x0,	0x5,	0x0,	0x2,	0x1,	0x0,	0x7,	0xC0,	0x0,	0x0,	0x0,	0x0,
0x7F,	0xFF,	0x3,	0xFF,	0x0,	0x80,	0x31,	0xC3,	0x40,	0x0,	0x3F,	0xB5,	0x83,	0xAE,	0x3C,	0xA6,
0x7C,	0x50,	0xC3,	0xA2,	0x40,	0x0,	0x0,	0x0,	0x40,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x3B,	0x3B,	0x89,	0xB5,
0x76,	0x4B,	0xC9,	0x8A,	0x39,	0x79,	0x8D,	0x48,	0x72,	0xB8,	0xCD,	0x0E,	0x3A,	0x50,	0x8B,	0x93,
0x74,	0x6D,	0xCB,	0x60,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x2,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x1,	0x23,	0x6,	0x54,	0x5,	0x42,	0x24,	0x1,	0x1E,	0x21,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0xFF,	0xB8,	0x0,	0x48,	0x0,	0x32,	0x2,	0x0,	0x0,	0x38,	0x0,	0x14,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
};


u8 bu24239_dl_data2_5B[448] ={	
0x0,	0x0,	0x77,	0x7E,	0x2,	0x0F,	0x24,	0x1B,	0x0,	0x1,	0x0,	0x0,	0x0,	0x66,	0x4,	0x0,
0x2,	0x54,	0x0,	0x0,	0x2,	0x0,	0x2,	0x14,	0x0,	0x1,	0x0,	0x1,	0x8,	0x0,	0x0,	0x2,
0x10,	0x10,	0x10,	0x0,	0x3,	0xEC,	0x3,	0xEC,	0x4,	0x0,	0x4,	0x0,	0x9,	0x23,	0x5,	0x0,
0x0,	0xF0,	0x22,	0x71,	0x2,	0x27,	0x0,	0xF0,	0x83,	0xC0,	0x5,	0x1,	0x2,	0x32,	0x0,	0x2,
0x0,	0xC3,	0x4,	0x0,	0x0,	0x20,	0x0,	0x0,	0xD0,	0x0,	0xD0,	0x0,	0xD0,	0x0,	0xD0,	0x0,
0xA0,	0xA0,	0x50,	0x50,	0x32,	0x32,	0x7,	0x80,	0x7,	0xC0,	0x4,	0x0,	0x0,	0x0,	0x0,	0x0,
0x20,	0x0,	0x20,	0x0,	0x4,	0x0,	0x4,	0x0,	0x0,	0x4,	0x0,	0x4,	0x0,	0x0,	0x0,	0x4,
0x0,	0x2,	0x2,	0x0,	0x82,	0xA7,	0x3D,	0x59,	0x82,	0xAD,	0x3D,	0x53,	0x3F,	0xFE,	0x3F,	0xFC,
0x3F,	0xFF,	0x3F,	0xFF,	0x40,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x3E,	0xFC,	0xC1,	0x0A,
0x7F,	0xF8,	0x7F,	0xF8,	0x1A,	0x5E,	0x1,	0xA5,	0x4,	0x65,	0x0,	0xFF,	0x0,	0xD9,	0x14,	0x0,
0x0,	0x2,	0x4,	0x65,	0x1,	0x51,	0x1,	0x0,	0x3F,	0xF2,	0x3F,	0xF2,	0x3F,	0xE0,	0x7F,	0xFF,
0x4,	0x65,	0x4,	0x65,	0x1,	0x0,	0x11,	0x9A,	0x60,	0x38,	0xA0,	0x8C,	0x7F,	0x90,	0x25,	0xD0,
0xB6,	0x26,	0x24,	0x1E,	0x7F,	0x24,	0x81,	0xA4,	0x3D,	0xE3,	0x85,	0x0E,	0x3D,	0xE3,	0x7A,	0xA7,
0x89,	0x8,	0x6F,	0x40,	0xF0,	0x0,	0xF0,	0x0,	0xF0,	0x0,	0xF0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x10,	0x0,	0x10,	0x0,	0x1,	0x18,	0x10,	0x0,
0x0,	0x30,	0x0,	0x40,	0x0,	0x5,	0x0,	0x2,	0x1,	0x0,	0x7,	0xC0,	0x0,	0x0,	0x0,	0x0,
0x7F,	0xFF,	0x3,	0xFF,	0x0,	0x80,	0x31,	0xC3,	0x40,	0x0,	0x3F,	0xB5,	0x83,	0xAE,	0x3C,	0xA6,
0x7C,	0x50,	0xC3,	0xA2,	0x40,	0x0,	0x0,	0x0,	0x40,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x3B,	0x3B,	0x89,	0xB5,
0x76,	0x4B,	0xC9,	0x8A,	0x39,	0x79,	0x8D,	0x48,	0x72,	0xB8,	0xCD,	0x0E,	0x3A,	0x50,	0x8B,	0x93,
0x74,	0x6D,	0xCB,	0x60,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x2,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x1,	0x23,	0x6,	0x54,	0x5,	0x42,	0x24,	0x1,	0x1E,	0x21,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0xFF,	0xB8,	0x0,	0x48,	0x0,	0x32,	0x2,	0x0,	0x0,	0x38,	0x0,	0x14,	0x0,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0


};

static int DATE_OIS_EEPROM(u16 eeprom_reg)
{
	int i;
	u8 u8data = 0;
	u8 eeprom_data[44];
	u8 puSendCmd2[10];
	//u16 date_reg = 0x0011;

	g_pstAF_I2Cclient->addr = 0x51;
	for(i=0; i<=2; i++)
	{
		puSendCmd2[0] = (u8)((eeprom_reg>>8)&0xFF);
		puSendCmd2[1] = (u8)(eeprom_reg&0xFF);
		
		if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 2) < 0) {
			LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
			return -1;
		}

		if (i2c_master_recv(g_pstAF_I2Cclient, &u8data, 1) < 0) {
			printk("ReadI2C OIS EEPROM failed!!\n");
			return -1;
		}
		eeprom_data[i]=u8data;
		printk("simon 2 Get OIS EEPROM 0x%x%x=%x\n", puSendCmd2[0], puSendCmd2[1], eeprom_data[i]);
		
		eeprom_reg=eeprom_reg+1;
		
	}
	
	if(eeprom_data[0]>=16 && eeprom_data[1]>=4){
		OIS_DATA_FLAG = OIS_7B;
	}else{
		OIS_DATA_FLAG = OIS_5B;
	}
	
	return 0;
}
static int Download_OIS_EEPROM(u16 eeprom_reg)
{
	int i, j;
	u8 u8data = 0;

	u8 eeprom_data[44];
	u8 puSendCmd2[10];

	g_pstAF_I2Cclient->addr = 0x51;

	for(i=0; i<=43; i++)
	{
		puSendCmd2[0] = (u8)((eeprom_reg>>8)&0xFF);
		puSendCmd2[1] = (u8)(eeprom_reg&0xFF);
		
		if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 2) < 0) {
			LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
			return -1;
		}

		if (i2c_master_recv(g_pstAF_I2Cclient, &u8data, 1) < 0) {
			printk("ReadI2C OIS EEPROM failed!!\n");
			return -1;
		}
		eeprom_data[i]=u8data;
		printk("Get OIS EEPROM 0x%x%x=%x\n", puSendCmd2[0], puSendCmd2[1], eeprom_data[i]);
		eeprom_reg=eeprom_reg+1;
	}

	eeprom_reg=0x1DC0;
	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR) >> 1;
	//g_pstAF_I2Cclient->ext_flag = I2C_WR_FLAG | I2C_RS_FLAG;

	
	for( i=0; i<11; i++)
	{
		puSendCmd2[0]= (u8)((eeprom_reg>>8)&0xFF) ;
		puSendCmd2[1]= (u8)( eeprom_reg&0xFF) ;
		puSendCmd2[2] = eeprom_data[i*4];
		puSendCmd2[3] = eeprom_data[i*4+1];
		puSendCmd2[4] = eeprom_data[i*4+2];
		puSendCmd2[5] = eeprom_data[i*4+3];
	
		for(j=0; j<=5; j++)
		{
			printk("Write OIS Register; puSendCmd[%d]=0x%x\n", j, puSendCmd2[j]);
		}
	
		if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 6) < 0) {
					printk("OIS_WriteI2C Download 51 failed!!\n");
					return -1;
		}
		eeprom_reg=eeprom_reg+4;
	}

	return 0;
}


static int OIS_DownloadData(u8 data_no)
{
	int i;
	u8 puSendCmd[18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	//u8 puSendCmd2[6] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF), (u8)((data>>24)&0xFF), (u8)((data>>16)&0xFF), (u8)((data>>8)&0xFF), (u8)(data&0x0FF) };
	//printk("WriteI2C 0x%x, 0x%x, 0x%x\n", data_no, ois_reg, data);
	u16 data1_start_addr=0x0000;
	
	//g_pstAF_I2Cclient->addr = (I2C_SLAVE_ADDRESS) >> 1;
	//g_pstAF_I2Cclient->ext_flag = I2C_WR_FLAG | I2C_RS_FLAG;

	printk("+++KYLE+++ OIS_DownloadData START_%d!!\n", (int)data_no);

	if (data_no == 1)
	{
		for( i=0; i<238; i++)
		{
			puSendCmd[0]= (u8)((data1_start_addr>>8)&0xFF) ;
			puSendCmd[1]= (u8)(data1_start_addr&0xFF) ;
			puSendCmd[2] = bu24239_dl_data1[i*4];
			puSendCmd[3] = bu24239_dl_data1[i*4+1];
			puSendCmd[4] = bu24239_dl_data1[i*4+2];
			puSendCmd[5] = bu24239_dl_data1[i*4+3];

			//for(j=0; j<=5; j++)
			//{
			//	printk("Write OIS I2C puSendCmd[%d]=0x%x\n", j, puSendCmd[j]);
			//}


			if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 6) < 0) {
						printk("OIS_WriteI2C Download 51 failed!!\n");
						return -1;
				}
			data1_start_addr=data1_start_addr+4;
		}			

	}
	else // 2
	{
		data1_start_addr = 0x1C00;
		
		for( i=0; i<112; i++)
		{
			puSendCmd[0]= (u8)((data1_start_addr>>8)&0xFF) ;
			puSendCmd[1]= (u8)(data1_start_addr&0xFF) ;
			if(OIS_DATA_FLAG == OIS_7B){
			puSendCmd[2] = bu24239_dl_data2_7B[i*4];
			puSendCmd[3] = bu24239_dl_data2_7B[i*4+1];
			puSendCmd[4] = bu24239_dl_data2_7B[i*4+2];
			puSendCmd[5] = bu24239_dl_data2_7B[i*4+3];
			}else if(OIS_DATA_FLAG == OIS_5B){
			puSendCmd[2] = bu24239_dl_data2_5B[i*4];
			puSendCmd[3] = bu24239_dl_data2_5B[i*4+1];
			puSendCmd[4] = bu24239_dl_data2_5B[i*4+2];
			puSendCmd[5] = bu24239_dl_data2_5B[i*4+3];	
			}else{
			puSendCmd[2] = bu24239_dl_data2_7B[i*4];
			puSendCmd[3] = bu24239_dl_data2_7B[i*4+1];
			puSendCmd[4] = bu24239_dl_data2_7B[i*4+2];
			puSendCmd[5] = bu24239_dl_data2_7B[i*4+3];	
			}
				
			//for(j=0; j<=5; j++)
			//{
			//	printk("Write OIS I2C puSendCmd[%d]=0x%x\n", j, puSendCmd[j]);
			//}

			if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 6) < 0) {
						printk("OIS_WriteI2C Download 51 failed!!\n");
						return -1;
				}
			
			data1_start_addr=data1_start_addr+4;
		}

			

	}
	printk("+++KYLE+++ OIS_DownloadData END!!\n");

	return 0;
}


static int OIS_ReadI2C_32bit(u16 ois_reg, u32 *data)
{
	u8 pBuff[10] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF), 0, 0, 0, 0, 0, 0, 0, 0 };
	//u8 u8data = 0;

	//u8 puSendCmd2[2] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF) };


	g_pstAF_I2Cclient->addr = ((AF_I2C_SLAVE_ADDR) >> 1)& I2C_MASK_FLAG;
	g_pstAF_I2Cclient->ext_flag = I2C_WR_FLAG | I2C_RS_FLAG;


	if (i2c_master_send(g_pstAF_I2Cclient, pBuff, (4 << 8 | 2)) < 0) {//ALPS02370889, Read 4Byte<< 8, Write 2 Byte
		printk("[CAMERA SENSOR] OIS_ReadI2C_32bit failed!!\n");
		return -1;
	}

		//if (i2c_master_recv(g_pstAF_I2Cclient, pBuff, 4) < 0) {
		//	printk("OIS_ReadI2C_32bit i2c_master_recv failed!!\n");
		//	return -1;
		//}
		*data = (((u32) pBuff[0]) << 8) + ((u32) pBuff[1]);


	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR) >> 1;
	g_pstAF_I2Cclient->ext_flag = rs_flag;

	printk("OIS_ReadI2C_32bit; pBuff 0~4=0x%x %x %x %x %x\n", pBuff[0], pBuff[1], pBuff[2], pBuff[3], pBuff[4]);

	printk("OIS_ReadI2C_32bit; 0x%x, 0x%x\n", ois_reg, *data);

	return 0;
}



static int OIS_ReadI2C(u8 length, u16 ois_reg, u16 *data)
{
	u8 pBuff[10] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF), 0, 0, 0, 0, 0, 0, 0, 0 };
	u8 u8data = 0;

	u8 puSendCmd2[2] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF) };

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;
	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;
	g_pstAF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
	rs_flag=g_pstAF_I2Cclient->ext_flag;

	if (length == 0)
	{
		//g_pstAF_I2Cclient->addr = (I2C_SLAVE_ADDRESS) >> 1;
		if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 2) < 0) {
			LOG_INF("[OIS_ReadI2C] read I2C send failed!!\n");
			return -1;
		}

		if (i2c_master_recv(g_pstAF_I2Cclient, &u8data, 1) < 0) {
			printk("[OIS_ReadI2C] i2c_master_recv failed!!\n");
			return -1;
		}
		*data = u8data;
	}
	else if (length == 1)
	{
		g_pstAF_I2Cclient->addr = ((AF_I2C_SLAVE_ADDR) >> 1)& I2C_MASK_FLAG;
		g_pstAF_I2Cclient->ext_flag = I2C_WR_FLAG | I2C_RS_FLAG;

		if (i2c_master_send(g_pstAF_I2Cclient, pBuff, (2 << 8 | 2)) < 0) {//ALPS02370889, Read 2Byte<< 8, Write 2 Byte
			printk("OIS_ReadI2C I2C_RS_FLAG failed!!\n");
			return -1;
		}
		printk("OIS_ReadI2C AF; pBuff[0]=0x%x, pBuff[1]=0x%x, pBuff[2]=0x%x, pBuff[3]=0x%x, pBuff[4]=0x%x \n", pBuff[0], pBuff[1], pBuff[2], pBuff[3], pBuff[4]);

		*data = (((u16) pBuff[0]) << 8) + ((u16) pBuff[1]);
		
		g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR) >> 1;
		g_pstAF_I2Cclient->ext_flag = rs_flag;
	}
	printk("ReadI2C 0x%x, 0x%x, 0x%x\n", length, ois_reg, *data);

	return 0;
}

 int Check_OIS_Ready(void)
{
	u16 OIS_Ready=0;
//<2016/04/21-ShermanWei, for ESTA 3A startPreview timeout
	///int retry=1000;
	int retry=100;
//>2016/04/21-ShermanWei,
	while(retry>0)
	{
		OIS_ReadI2C(0, 0x6024, &OIS_Ready);
		printk("+++KYLE+++Check_OIS_Ready, 0x6024=%d retry = %d\n", (int)OIS_Ready,retry);
	
		if(OIS_Ready==1)
			break;
		mdelay(5);
		retry--;
	}
	return OIS_Ready;
}

 int OIS_WriteI2C(u8 length, u16 ois_reg, u32 data)
{
	//int i;
	u8 puSendCmd[3] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF), (u8)(data&0x0FF) };
	u8 puSendCmd2[6] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF), (u8)((data>>24)&0xFF), (u8)((data>>16)&0xFF), (u8)((data>>8)&0xFF), (u8)(data&0x0FF) };
	printk("WriteI2C 0x%x, 0x%x, 0x%x\n", length, ois_reg, data);

	//for(i=0; i<=5; i++)
	//{
	//	printk("Write OIS I2C puSendCmd2[%d]=0x%x\n", i, puSendCmd2[i]);
	//}

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR) >> 1;///<2017/05/23-ShermanWei,
	/// For ERROR,545: id=1,addr: 51, transfer error, Ex.WriteI2C 0x5, 0xf020, 0x1011810
	//g_pstAF_I2Cclient->ext_flag = I2C_WR_FLAG | I2C_RS_FLAG;

	if (length == 0)
	{
		if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3) < 0) {
			printk("OIS_WriteI2C 1Byte failed!!\n");
			return -1;
		}
	}
	else
	{
		if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 6) < 0)
		{
				printk("OIS_WriteI2C failed!!\n");
				return -1;
		}
	}

	return 0;
}


static int AF_Position_WriteI2C(u16 ois_reg, u16 data)
{
	int i;
	u8 puSendCmd2[4] = { (u8)((ois_reg>>8)&0xFF), (u8)(ois_reg&0xFF), (u8)((data>>8)&0xFF), (u8)(data&0x0FF) };
	printk("AF_Position_WriteI2C 0x%x=0x%x\n", ois_reg, data);

	for(i=0; i<=3; i++)
	{
		printk("AF_Position_WriteI2C puSendCmd2[%d]=0x%x\n", i, puSendCmd2[i]);
	}

	//g_pstAF_I2Cclient->addr = (I2C_SLAVE_ADDRESS) >> 1;
	//g_pstAF_I2Cclient->ext_flag = I2C_WR_FLAG | I2C_RS_FLAG;


	if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 4) < 0)
	{
			printk("OIS_WriteI2C failed!!\n");
			return -1;
	}

	return 0;
}

#if 0
static int s4AF_ReadReg(unsigned short *a_pu2Result)
{
	int i4RetValue = 0;
	char pBuff[2];

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, pBuff, 2);

	if (i4RetValue < 0) {
		LOG_INF("I2C read failed!!\n");
		return -1;
	}

	*a_pu2Result = (((u16)(pBuff[0] & 0x03)) << 8) + pBuff[1];

	return 0;
}

static int s4AF_WriteReg(u16 a_u2Data)
{
	int i4RetValue = 0;

	char puSendCmd[2] = {(char)(((a_u2Data >> 8) & 0x03) | 0xc0), (char)(a_u2Data & 0xff)};

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);

	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return -1;
	}

	return 0;
}
#endif
static inline int getAFInfo(__user stAF_MotorInfo * pstMotorInfo)
{
	stAF_MotorInfo stMotorInfo;

	stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
	stMotorInfo.u4InfPosition = g_u4AF_INF;
	stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	stMotorInfo.bIsSupportSR = 1;

	stMotorInfo.bIsMotorMoving = 1;

	if (*g_pAF_Opened >= 1)
		stMotorInfo.bIsMotorOpen = 1;
	else
		stMotorInfo.bIsMotorOpen = 0;

	if (copy_to_user(pstMotorInfo, &stMotorInfo, sizeof(stAF_MotorInfo)))
		LOG_INF("copy to user failed when getting motor information\n");

	return 0;
}

static inline int moveAF(unsigned long a_u4Position)
{
	int ret = 0;


	int OIS_Ready=3;
	u32 OIS_chksum=0x00015B74;
	//int retry=5;

	//u16 InitPos1;

	LOG_INF("+++KYLE+++ BU6424AF moveAF !!! \n");
	printk("+++KYLE+++g_pstAF_I2Cclient->timing=%d \n", g_pstAF_I2Cclient->timing);

	if ((a_u4Position > g_u4AF_MACRO) || (a_u4Position < g_u4AF_INF)) {
		LOG_INF("out of range\n");
		return -EINVAL;
	}

	if (*g_pAF_Opened == 1) 
	{
		unsigned short InitPos;

		OIS_Ready=Check_OIS_Ready();
		printk("+++KYLE+++AF_Open Before OIS_ReadI2C; OIS_Ready=%d \n", OIS_Ready);
				
		//<2015/11/07-kylechang, Fix capture time 30s
		OIS_ReadI2C(1, 0x60F2, &InitPos);
		if(InitPos>0)
		{
			printk("Pos %x>0, Skip OIS Init\n", InitPos);
		}
		else //Init OIS
		{
			printk("+++KYLE+++ Init OIS Only One time~\n");
			printk("++++++++\n");
			DATE_OIS_EEPROM(0x0011);
			printk("--------\n");

			OIS_WriteI2C(5, 0xF020, 0x01011810);
			OIS_WriteI2C(5, 0xF024, 0x00010002);
			OIS_WriteI2C(5, 0xF028, 0x000000E1);
			OIS_WriteI2C(5, 0xF02C, 0x0B130000);
		
			mdelay(1);
			OIS_WriteI2C(5, 0xF02C, 0x0C800000);
		
			OIS_Ready=Check_OIS_Ready();
			printk("+++KYLE+++AF_Open Init Gyro OK; OIS_Ready=%d \n", OIS_Ready);
		
			OIS_WriteI2C(0, 0xF010, 0);
			mdelay(1);

			OIS_DownloadData(1);
			OIS_DownloadData(2);
		
			OIS_ReadI2C_32bit(0xF008, &OIS_chksum);
			
			Download_OIS_EEPROM(0x0780);
			
			OIS_WriteI2C(0, 0xF006, 0);
			
			OIS_Ready=Check_OIS_Ready();
			//printk("+++KYLE+++AF_Open Check Download Complete; OIS_Ready=%d \n", OIS_Ready);
		
			OIS_Ready=Check_OIS_Ready();
			printk("+++KYLE+++AF_Open Check Download Complete; OIS_Ready=%d \n", OIS_Ready);
		
			OIS_WriteI2C(0, 0x6020, 1);
			mdelay(105);
		
			OIS_Ready=Check_OIS_Ready();
			printk("+++KYLE+++AF_Open Servo On; OIS_Ready=%d \n", OIS_Ready);
			//AF Init
			OIS_WriteI2C(0, 0x60F1, 6);
			//<2015/10/29-kylechang, Workaround: AF range 0~254
			AF_Position_WriteI2C(0x60F2, 50);//0x104=260
			//>2015/10/29-kylechang
			OIS_Ready=Check_OIS_Ready();
			printk("+++KYLE+++AF_Open AF Init OK; OIS_Ready=%d \n", OIS_Ready);
			//
			OIS_WriteI2C(0, 0x6023, 0);
		
			OIS_Ready=Check_OIS_Ready();
			printk("+++KYLE+++AF_Open Gyro On; OIS_Ready=%d \n", OIS_Ready);
			
			OIS_WriteI2C(0, 0x6021, 0x03);//zero shutter
		
			OIS_Ready=Check_OIS_Ready();
			printk("+++KYLE+++AF_Open Still Mode; OIS_Ready=%d \n", OIS_Ready);
		
			OIS_WriteI2C(0, 0x6020, 2);
			
			OIS_Ready=Check_OIS_Ready();
			printk("+++KYLE+++AF_Open OIS On; OIS_Ready=%d \n", OIS_Ready);
		
			/*
			while(1)
			{
				//mdelay(2000);
				//OIS_ReadI2C(1, 0x60F2, &InitPos1);
				//printk("AF_Open Loop Pos 0xa=0x%x\n", InitPos1);
		
			
				AF_Position_WriteI2C(0x60F2, 0xab);
				mdelay(1);
				
				OIS_ReadI2C(1, 0x60F2, &InitPos1);
				printk("AF_Open Loop Pos 0x00ab=0x%x\n", InitPos1);
				
				//AF_Position_WriteI2C(0x60F2, 10);
				
			}
			*/

		}
		//>2015/11/07-kylechang

		//ret = s4AF_ReadReg(&InitPos);
		ret = OIS_ReadI2C(1, 0x60F2, &InitPos);

		if (ret == 0) {
			LOG_INF("Init Pos %6d\n", InitPos);

			spin_lock(g_pAF_SpinLock);
			g_u4CurrPosition = (unsigned long)InitPos;
			spin_unlock(g_pAF_SpinLock);

		} else {
			spin_lock(g_pAF_SpinLock);
			g_u4CurrPosition = 0;
			spin_unlock(g_pAF_SpinLock);
		}

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 2;
		spin_unlock(g_pAF_SpinLock);
	}

	if (g_u4CurrPosition == a_u4Position)
		return 0;

	spin_lock(g_pAF_SpinLock);
	g_u4TargetPosition = a_u4Position;
	spin_unlock(g_pAF_SpinLock);

	/* LOG_INF("move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition); */

	printk("+++KYLE+++ Move OIS AF to Pos %d\n", (int)g_u4TargetPosition);
////<2016/05/16-ShermanWei, workaround for CTS verifier Switch resolution too slow 
	if (g_u4TargetPosition == 0)
	g_u4TargetPosition = 1;	
////>2016/05/16-ShermanWei,

	//if (s4AF_WriteReg((unsigned short)g_u4TargetPosition) == 0) {
	if (AF_Position_WriteI2C( 0x60F2, (u16)g_u4TargetPosition) == 0) {
		spin_lock(g_pAF_SpinLock);
		g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
		spin_unlock(g_pAF_SpinLock);
	} else {
		LOG_INF("set I2C failed when moving the motor\n");
		printk("set I2C failed when moving the OIS AF motor\n");
	}

	return 0;
}

static inline int setAFInf(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_INF = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_MACRO = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}
//<2016/01/26-simonlin ALPS02524870 OIS control flow
static inline int setOIS(unsigned long a_u4Position)
{
	u16 OIS_Ready=0;
	printk("*****setOIS*******\n");
	if(a_u4Position == 1)
	{
		OIS_status=1;
		OIS_Ready=Check_OIS_Ready();
		printk("OIS_Switch_store Sitch=%d, OIS_Ready=%d\n", OIS_status, OIS_Ready);
		OIS_WriteI2C(0, 0x6021, 0x03);//zero shutter
		
		OIS_Ready=Check_OIS_Ready();
		printk("OIS_Switch_store OIS Mode, Sitch=%d, OIS_Ready=%d\n", OIS_status, OIS_Ready);
		
		OIS_WriteI2C(0, 0x6020, 2);//OIS on
	}else if(a_u4Position == 0){
		OIS_status=0;
		OIS_Ready=Check_OIS_Ready();
		printk("OIS_Switch_store Sitch=%d, OIS_Ready=%d\n", OIS_status, OIS_Ready);
		
		OIS_WriteI2C(0, 0x6020, 1);//OIS off
	}
		
	return 0;
}
//>2016/01/26
/* ////////////////////////////////////////////////////////////// */
long BU6424AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command, unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_G_MOTORINFO:
		i4RetValue = getAFInfo((__user stAF_MotorInfo *) (a_u4Param));
		break;

	case AFIOC_T_MOVETO:
		i4RetValue = moveAF(a_u4Param);
		break;

	case AFIOC_T_SETINFPOS:
		i4RetValue = setAFInf(a_u4Param);
		break;

	case AFIOC_T_SETMACROPOS:
		i4RetValue = setAFMacro(a_u4Param);
		break;
	//<2016/01/26-simonlin ALPS02524870 OIS control flow
	 case BU6424AFIOC_T_SETPARA:          
		 i4RetValue = setOIS(a_u4Param);     
	//>2016/01/26-simonlin
	 break;		
	default:
		LOG_INF("No CMD\n");
		i4RetValue = -EPERM;
		break;
	}

	return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
int BU6424AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
		//char puSendCmd[2];

		//puSendCmd[0] = (char)(0x00);
		//puSendCmd[1] = (char)(0x00);
		//i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);
		LOG_INF("Wait\n");
		/*s4AF_WriteReg(200);
		msleep(20);
		s4AF_WriteReg(100);
		msleep(20);*/
	}

	if (*g_pAF_Opened) {
		LOG_INF("Free\n");

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 0;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("End\n");

	return 0;
}

void BU6424AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient, spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;
	
	g_pstAF_I2Cclient->timing = 400;
}
 ssize_t OIS_Switch_show (struct device *dev, struct device_attribute *attr, char *buf)
{		
		ssize_t   ret = 0;
		sprintf( buf, "OIS_Switch=%d\n", OIS_status );
		ret = strlen( buf) + 1;
		return ret;
}

 ssize_t  OIS_Switch_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int OIS_Ready=3;
	//bool Sitch;// 2017/06/02, Rickliu, fix coverity issue
	int Sitch;
	//int b = a-'0';
	Sitch=buf[0]-'0';
	//GloveSitch=simple_strtol(buf[0],0,10);
	if( Sitch == 1)
	{
		OIS_status=1;
		OIS_Ready=Check_OIS_Ready();
		printk("OIS_Switch_store Sitch=%d, OIS_Ready=%d\n", OIS_status, OIS_Ready);
		OIS_WriteI2C(0, 0x6021, 0x03);//zero shutter
		
		OIS_Ready=Check_OIS_Ready();
		printk("OIS_Switch_store OIS Mode, Sitch=%d, OIS_Ready=%d\n", OIS_status, OIS_Ready);
		
		OIS_WriteI2C(0, 0x6020, 2);
	}
	else if(Sitch == 0)
	{
		OIS_status=0;
		OIS_Ready=Check_OIS_Ready();
		printk("OIS_Switch_store Sitch=%d, OIS_Ready=%d\n", OIS_status, OIS_Ready);
		
		OIS_WriteI2C(0, 0x6020, 1);
	}
	else
	{
		printk("you send neither 1 nor 0 buf[0]=%d\n",buf[0]);
	}
	printk("OIS_Switch_store enter......\n");
	printk("buf[store]=%d %d %d %d\n",buf[0],buf[1],buf[2],buf[3]);
	//	ret = strlen(buf) + 1;
	return count;
}

 ssize_t OIS_Shutter_store (struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t   ret = 0;
	printk("OIS_Shutter_store\n");
//	sprintf( buf, "OIS_ZERO_SHUTTER_MODE \n" );
//	ret = strlen( buf) + 1;
	

	OIS_WriteI2C(0, 0x6020, 0x01);//OIS off

	OIS_WriteI2C(0, 0x6021, 0x03);//zero shutter on

	mdelay(10);

	OIS_WriteI2C(0, 0x6020, 0x02);//OIS on

	OIS_status =1;

	return ret;
}

 ssize_t OIS_Movie_store (struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t   ret = 0;
	printk("OIS_Movie_store\n");
//	sprintf( buf, "OIS_MOVIE_MODE \n" );
//	ret = strlen( buf) + 1;

	OIS_WriteI2C(0, 0x6020, 0x01);//OIS off

	OIS_WriteI2C(0, 0x6021, 0x61);//movie mode on

	mdelay(10);

	OIS_WriteI2C(0, 0x6020, 0x02);//OIS on

	OIS_status =1;

	return ret;
}
 ssize_t FrontAF_store (struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{
	int step=0;

	sscanf(buf,"%d",&step);
	printk("------- step = %d\n",step);
	//Sitch=buf[0]-'0';

	//<2015/10/29-kylechang, Workaround: AF range 0~254
	AF_Position_WriteI2C(0x60F2, step);//0x104=260
	//>2015/10/29-kylechang

	return count;
}
