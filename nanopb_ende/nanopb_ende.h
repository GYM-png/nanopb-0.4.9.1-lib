#ifndef __NANOPB_ENDE_H__
#define __NANOPB_ENDE_H__

#include "frame.pb.h"
#include "stdbool.h"
#include "stdint.h"

#define NANOPB_MAX_MESSAGE_SIZE FRAME_PB_H_MAX_SIZE
#define NANOPB_PACK_MESSAGE_HEAD (0xA5A5)
#define NANOPB_PACK_HEAD_LEN  (offsetof(packed_t, nanopb_data))


#pragma pack(1)
typedef struct 
{
    uint16_t head;
    uint16_t address;
    uint16_t which_load;
    uint16_t nanopb_len;
    uint16_t nanopb_crc16;
    uint8_t nanopb_data[NANOPB_MAX_MESSAGE_SIZE];
}packed_t;
#pragma pack()



#endif