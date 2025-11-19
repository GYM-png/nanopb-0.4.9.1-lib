#include "nanopb_ende.h"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#include "flexi_log.h"
#define FLOG_TAG "nanopb_ende"


/**
 * @brief CRC校验
 * @param data 数据
 * @param len 数据长度
 * @return CRC校验值
 */
static uint16_t modbus_crc(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for(uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 0x01)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    } 
    return crc;
}

/**
 * @brief 编码
 * @param feame 帧
 * @param buffer 编码缓冲区
 * @param buffer_size 编码缓冲区大小
 * @return 编码长度
 */
static uint16_t nanopb_encode(Frame *feame, uint8_t *buffer, uint16_t buffer_size)
{
    pb_ostream_t stream;
    stream = pb_ostream_from_buffer(buffer, buffer_size);
    if(pb_encode(&stream, Frame_fields, feame))
    {
        return stream.bytes_written;
    }
    else
    {
        loge("nanopb_packed error encode %s", PB_GET_ERROR(&stream));
        return 0;
    }
} 

/**
 * @brief 解码
 * @param buffer 缓冲区
 * @param buffer_size 缓冲区大小
 * @param frame 解码帧
 * @return true 成功 false 失败
 */
static bool nanopb_decode(uint8_t *buffer, uint16_t buffer_size, Frame *frame)
{
    pb_istream_t stream;
    stream = pb_istream_from_buffer(buffer, buffer_size);
    bool res = pb_decode(&stream, Frame_fields, frame);
    if (!res)
    {
        loge("nanopb_decode error: %s", PB_GET_ERROR(&stream));
    }
    return res;
}


/**
 * @brief 打包
 * @param frame 帧
 * @param packed 打包帧
 * @param addr 地址
 * @return 打包长度
 */
uint16_t nanopb_packed(Frame *frame, packed_t *packed, uint8_t addr)
{
    uint16_t pb_len = 0;
    pb_len = nanopb_encode(frame, packed->nanopb_data, sizeof(packed->nanopb_data));
    if (pb_len > 0)
    {
        packed->nanopb_len = pb_len;
        packed->address = addr;
        packed->head = NANOPB_PACK_MESSAGE_HEAD;
        packed->nanopb_crc16 = modbus_crc(packed->nanopb_data, pb_len);
        return pb_len + NANOPB_PACK_HEAD_LEN;
    }
    else
    {
        return 0;
    }    
}

/**
 * @brief 解包
 * @param packed 
 * @param frame 
 * @return true 成功 false 失败
 */
bool nanopb_unpacked(packed_t *packed, Frame *frame)
{
    if (packed->head == NANOPB_PACK_MESSAGE_HEAD)
    {
        bool de_red = nanopb_decode(packed->nanopb_data, packed->nanopb_len, frame);
        if (de_red)
        {
            uint16_t crc_cal = modbus_crc(packed->nanopb_data, packed->nanopb_len);
            return (crc_cal == packed->nanopb_crc16);
        }
        else
        {
            loge("nanopb_unpacked error decode");
            return false;
        }
    }
    else
    {
        loge("nanopb_unpacked error head: %04x", packed->head);
        return false;
    }
}

/**
 * @brief 字符串格式化
 * @param field 字符串
 * @param field_size 字符串长度
 * @param format 格式
 */
void nanopb_sprintf(char *field, uint16_t field_size, char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(field, field_size, format, args);
    va_end(args);
}
