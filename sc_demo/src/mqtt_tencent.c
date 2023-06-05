/**
  ******************************************************************************
  * @file    mqtt_tencent.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source code for mqtt on Tencent cloud service.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "simcom_os.h"
#include "simcom_debug.h"

#define MAX_SIZE_OF_CLIENT_ID (80)
#define HOST_STR_LENGTH 64  /* Max size of a host name */
#define MAX_CONN_ID_LEN (6) /* Max size of conn Id  */
#define QCLOUD_IOT_MQTT_DIRECT_DOMAIN  "iotcloud.tencentdevices.com"
#define QCLOUD_IOT_MQTT_US_EAST_DOMAIN "us-east.iotcloud.tencentdevices.com"
//#define QCLOUD_IOT_DEVICE_SDK_APPID "21010406" /* IoT C-SDK APPID 12010126*/
#define QCLOUD_IOT_DEVICE_SDK_APPID "12010126"

#define KEY_IOPAD_SIZE 64

#define MD5_DIGEST_SIZE  16
#define SHA1_DIGEST_SIZE 20
#ifndef IOT_SHA1_GET_UINT32_BE
#define IOT_SHA1_GET_UINT32_BE(n, b, i)                                                                     \
    {                                                                                                       \
        (n) = ((uint32_t)(b)[(i)] << 24) | ((uint32_t)(b)[(i) + 1] << 16) | ((uint32_t)(b)[(i) + 2] << 8) | \
              ((uint32_t)(b)[(i) + 3]);                                                                     \
    }
#endif

#ifndef IOT_SHA1_PUT_UINT32_BE
#define IOT_SHA1_PUT_UINT32_BE(n, b, i)            \
    {                                              \
        (b)[(i)]     = (unsigned char)((n) >> 24); \
        (b)[(i) + 1] = (unsigned char)((n) >> 16); \
        (b)[(i) + 2] = (unsigned char)((n) >> 8);  \
        (b)[(i) + 3] = (unsigned char)((n));       \
    }
#endif

static const unsigned char iot_sha1_padding[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                   0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned char base64_dec_map[128] = {
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 62,
    127, 127, 127, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  127, 127, 127, 64,  127, 127, 127, 0,
    1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  127, 127, 127, 127, 127, 127, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  127, 127, 127, 127, 127};

typedef struct _RegionDomain_ {
    const char *region;
    const char *domain;
} RegionDomain;

/*mqtt domain*/
static RegionDomain sg_iot_mqtt_domain[] = {
    {.region = "china", .domain = QCLOUD_IOT_MQTT_DIRECT_DOMAIN},    /* China */
    {.region = "us-east", .domain = QCLOUD_IOT_MQTT_US_EAST_DOMAIN}, /* Eastern US*/
};

/**
  * @brief  Get domain address according to region.
  * @param  region,china or us-east
  * @note   
  * @retval RegionDomain
  */
const char *iot_get_mqtt_domain(char *region)
{
    const char *pDomain = NULL;
    int         i;

    if (!region) {
        goto end;
    }

    for (i = 0; i < sizeof(sg_iot_mqtt_domain) / sizeof(sg_iot_mqtt_domain[0]); i++) {
        if (0 == strcmp(region, sg_iot_mqtt_domain[i].region)) {
            pDomain = sg_iot_mqtt_domain[i].domain;
            break;
        }
    }

end:
    if (!pDomain) {
        pDomain = sg_iot_mqtt_domain[0].domain;
    }

    return pDomain;
}

/**
  * @brief  Get system tick.
  * @param  void
  * @note   
  * @retval system time
  */
uint32_t HAL_GetTimeMs(void)
{
    return sAPI_GetTicks();
}

/**
  * @brief  Get next connect id.
  * @param  conn_id
  * @note   Randomly get according to system tick.
  * @retval void
  */
void get_next_conn_id(char *conn_id)
{
    int i;
    
    sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
    srand((unsigned)HAL_GetTimeMs());
    
    sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
    for (i = 0; i < 5; i++) {
        int flag = rand() % 3;
        switch (flag) {
            case 0:
                conn_id[i] = (rand() % 26) + 'a';
                break;
            case 1:
                conn_id[i] = (rand() % 26) + 'A';
                break;
            case 2:
                conn_id[i] = (rand() % 10) + '0';
                break;
        }
    }

    conn_id[5] = '\0';
}

typedef struct {
    uint32_t      total[2];   /*!< number of bytes processed  */
    uint32_t      state[5];   /*!< intermediate digest state  */
    unsigned char buffer[64]; /*!< data block being processed */
} iot_sha1_context;

/**
  * @brief  utils initial.
  * @param  ctx
  * @note   
  * @retval void
  */
void utils_sha1_init(iot_sha1_context *ctx)
{
    memset(ctx, 0, sizeof(iot_sha1_context));
}

/**
  * @brief  utils starts.
  * @param  ctx
  * @note   
  * @retval void
  */
void utils_sha1_starts(iot_sha1_context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

/**
  * @brief  utils process.
  * @param  ctx
  * @param  data
  * @note   
  * @retval void
  */
void utils_sha1_process(iot_sha1_context *ctx, const unsigned char data[64])
{
    uint32_t temp, W[16], A, B, C, D, E;

    IOT_SHA1_GET_UINT32_BE(W[0], data, 0);
    IOT_SHA1_GET_UINT32_BE(W[1], data, 4);
    IOT_SHA1_GET_UINT32_BE(W[2], data, 8);
    IOT_SHA1_GET_UINT32_BE(W[3], data, 12);
    IOT_SHA1_GET_UINT32_BE(W[4], data, 16);
    IOT_SHA1_GET_UINT32_BE(W[5], data, 20);
    IOT_SHA1_GET_UINT32_BE(W[6], data, 24);
    IOT_SHA1_GET_UINT32_BE(W[7], data, 28);
    IOT_SHA1_GET_UINT32_BE(W[8], data, 32);
    IOT_SHA1_GET_UINT32_BE(W[9], data, 36);
    IOT_SHA1_GET_UINT32_BE(W[10], data, 40);
    IOT_SHA1_GET_UINT32_BE(W[11], data, 44);
    IOT_SHA1_GET_UINT32_BE(W[12], data, 48);
    IOT_SHA1_GET_UINT32_BE(W[13], data, 52);
    IOT_SHA1_GET_UINT32_BE(W[14], data, 56);
    IOT_SHA1_GET_UINT32_BE(W[15], data, 60);

#define S(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t) \
    (temp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^ W[(t - 14) & 0x0F] ^ W[t & 0x0F], (W[t & 0x0F] = S(temp, 1)))

#define P(a, b, c, d, e, x)                \
    {                                      \
        e += S(a, 5) + F(b, c, d) + K + x; \
        b = S(b, 30);                      \
    }

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];

#define F(x, y, z) (z ^ (x & (y ^ z)))
#define K          0x5A827999

    P(A, B, C, D, E, W[0]);
    P(E, A, B, C, D, W[1]);
    P(D, E, A, B, C, W[2]);
    P(C, D, E, A, B, W[3]);
    P(B, C, D, E, A, W[4]);
    P(A, B, C, D, E, W[5]);
    P(E, A, B, C, D, W[6]);
    P(D, E, A, B, C, W[7]);
    P(C, D, E, A, B, W[8]);
    P(B, C, D, E, A, W[9]);
    P(A, B, C, D, E, W[10]);
    P(E, A, B, C, D, W[11]);
    P(D, E, A, B, C, W[12]);
    P(C, D, E, A, B, W[13]);
    P(B, C, D, E, A, W[14]);
    P(A, B, C, D, E, W[15]);
    P(E, A, B, C, D, R(16));
    P(D, E, A, B, C, R(17));
    P(C, D, E, A, B, R(18));
    P(B, C, D, E, A, R(19));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K          0x6ED9EBA1

    P(A, B, C, D, E, R(20));
    P(E, A, B, C, D, R(21));
    P(D, E, A, B, C, R(22));
    P(C, D, E, A, B, R(23));
    P(B, C, D, E, A, R(24));
    P(A, B, C, D, E, R(25));
    P(E, A, B, C, D, R(26));
    P(D, E, A, B, C, R(27));
    P(C, D, E, A, B, R(28));
    P(B, C, D, E, A, R(29));
    P(A, B, C, D, E, R(30));
    P(E, A, B, C, D, R(31));
    P(D, E, A, B, C, R(32));
    P(C, D, E, A, B, R(33));
    P(B, C, D, E, A, R(34));
    P(A, B, C, D, E, R(35));
    P(E, A, B, C, D, R(36));
    P(D, E, A, B, C, R(37));
    P(C, D, E, A, B, R(38));
    P(B, C, D, E, A, R(39));

#undef K
#undef F

#define F(x, y, z) ((x & y) | (z & (x | y)))
#define K          0x8F1BBCDC

    P(A, B, C, D, E, R(40));
    P(E, A, B, C, D, R(41));
    P(D, E, A, B, C, R(42));
    P(C, D, E, A, B, R(43));
    P(B, C, D, E, A, R(44));
    P(A, B, C, D, E, R(45));
    P(E, A, B, C, D, R(46));
    P(D, E, A, B, C, R(47));
    P(C, D, E, A, B, R(48));
    P(B, C, D, E, A, R(49));
    P(A, B, C, D, E, R(50));
    P(E, A, B, C, D, R(51));
    P(D, E, A, B, C, R(52));
    P(C, D, E, A, B, R(53));
    P(B, C, D, E, A, R(54));
    P(A, B, C, D, E, R(55));
    P(E, A, B, C, D, R(56));
    P(D, E, A, B, C, R(57));
    P(C, D, E, A, B, R(58));
    P(B, C, D, E, A, R(59));

#undef K
#undef F

#define F(x, y, z) (x ^ y ^ z)
#define K          0xCA62C1D6

    P(A, B, C, D, E, R(60));
    P(E, A, B, C, D, R(61));
    P(D, E, A, B, C, R(62));
    P(C, D, E, A, B, R(63));
    P(B, C, D, E, A, R(64));
    P(A, B, C, D, E, R(65));
    P(E, A, B, C, D, R(66));
    P(D, E, A, B, C, R(67));
    P(C, D, E, A, B, R(68));
    P(B, C, D, E, A, R(69));
    P(A, B, C, D, E, R(70));
    P(E, A, B, C, D, R(71));
    P(D, E, A, B, C, R(72));
    P(C, D, E, A, B, R(73));
    P(B, C, D, E, A, R(74));
    P(A, B, C, D, E, R(75));
    P(E, A, B, C, D, R(76));
    P(D, E, A, B, C, R(77));
    P(C, D, E, A, B, R(78));
    P(B, C, D, E, A, R(79));

#undef K
#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}

/**
  * @brief  SHA-1 process buffer
  * @param  ctx
  * @param  input
  * @param  ilen
  * @note   
  * @retval void
  */
void utils_sha1_update(iot_sha1_context *ctx, const unsigned char *input, size_t ilen)
{
    size_t   fill;
    uint32_t left;

    if (ilen == 0) {
        return;
    }

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += (uint32_t)ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if (ctx->total[0] < (uint32_t)ilen) {
        ctx->total[1]++;
    }

    if (left && ilen >= fill) {
        memcpy((void *)(ctx->buffer + left), input, fill);
        utils_sha1_process(ctx, ctx->buffer);
        input += fill;
        ilen -= fill;
        left = 0;
    }

    while (ilen >= 64) {
        utils_sha1_process(ctx, input);
        input += 64;
        ilen -= 64;
    }

    if (ilen > 0) {
        memcpy((void *)(ctx->buffer + left), input, ilen);
    }
}

/**
  * @brief  SHA-1 final digest
  * @param  ctx
  * @param  output
  * @note   
  * @retval void
  */
void utils_sha1_finish(iot_sha1_context *ctx, unsigned char output[20])
{
    uint32_t      last, padn;
    uint32_t      high, low;
    unsigned char msglen[8];

    high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
    low  = (ctx->total[0] << 3);

    IOT_SHA1_PUT_UINT32_BE(high, msglen, 0);
    IOT_SHA1_PUT_UINT32_BE(low, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);

    utils_sha1_update(ctx, iot_sha1_padding, padn);
    utils_sha1_update(ctx, msglen, 8);

    IOT_SHA1_PUT_UINT32_BE(ctx->state[0], output, 0);
    IOT_SHA1_PUT_UINT32_BE(ctx->state[1], output, 4);
    IOT_SHA1_PUT_UINT32_BE(ctx->state[2], output, 8);
    IOT_SHA1_PUT_UINT32_BE(ctx->state[3], output, 12);
    IOT_SHA1_PUT_UINT32_BE(ctx->state[4], output, 16);
}

/**
  * @brief  hb2hex
  * @param  hb
  * @note   
  * @retval void
  */
int8_t utils_hb2hex(uint8_t hb)
{
    hb = hb & 0xF;
    return (int8_t)(hb < 10 ? '0' + hb : hb - 10 + 'a');
}

/**
  * @brief  hmac
  * @param  msg
  * @param  msg_len
  * @param  digest
  * @param  key
  * @param  key_len
  * @note   
  * @retval void
  */
void utils_hmac_sha1(const char *msg, int msg_len, char *digest, const char *key, int key_len)
{
    if ((NULL == msg) || (NULL == digest) || (NULL == key)) {
        sAPI_Debug("parameter is Null,failed!");
        return;
    }

    if (key_len > KEY_IOPAD_SIZE) {
        sAPI_Debug("key_len > size(%d) of array", KEY_IOPAD_SIZE);
        return;
    }

    iot_sha1_context context;
    unsigned char    k_ipad[KEY_IOPAD_SIZE]; /* inner padding - key XORd with ipad */
    unsigned char    k_opad[KEY_IOPAD_SIZE]; /* outer padding - key XORd with opad */
    unsigned char    out[SHA1_DIGEST_SIZE];
    int              i;

    /* start out by storing key in pads */
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);

    /* XOR key with ipad and opad values */
    for (i = 0; i < KEY_IOPAD_SIZE; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    /* perform inner SHA */
    utils_sha1_init(&context);                                  /* init context for 1st pass */
    utils_sha1_starts(&context);                                /* setup context for 1st pass */
    utils_sha1_update(&context, k_ipad, KEY_IOPAD_SIZE);        /* start with inner pad */
    utils_sha1_update(&context, (unsigned char *)msg, msg_len); /* then text of datagram */
    utils_sha1_finish(&context, out);                           /* finish up 1st pass */

    /* perform outer SHA */
    utils_sha1_init(&context);                           /* init context for 2nd pass */
    utils_sha1_starts(&context);                         /* setup context for 2nd pass */
    utils_sha1_update(&context, k_opad, KEY_IOPAD_SIZE); /* start with outer pad */
    utils_sha1_update(&context, out, SHA1_DIGEST_SIZE);  /* then results of 1st hash */
    utils_sha1_finish(&context, out);                    /* finish up 2nd pass */

    for (i = 0; i < SHA1_DIGEST_SIZE; ++i) {
        digest[i * 2]     = utils_hb2hex(out[i] >> 4);
        digest[i * 2 + 1] = utils_hb2hex(out[i]);
    }
}

/**
  * @brief  base64decode
  * @param  dst
  * @param  dlen
  * @param  olen
  * @param  src
  * @param  slen
  * @note   
  * @retval 0--decode succefully   -1:decode failed
  */
int qcloud_iot_utils_base64decode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen)
{
    size_t         i, n;
    uint32_t       j, x;
    unsigned char *p;

    /* First pass: check for validity and get output length */
    for (i = n = j = 0; i < slen; i++) {
        /* Skip spaces before checking for EOL */
        x = 0;
        while (i < slen && src[i] == ' ') {
            ++i;
            ++x;
        }

        /* Spaces at end of buffer are OK */
        if (i == slen)
            break;

        if ((slen - i) >= 2 && src[i] == '\r' && src[i + 1] == '\n')
            continue;

        if (src[i] == '\n')
            continue;

        /* Space inside a line is an error */
        if (x != 0)
            return (-1);

        if (src[i] == '=' && ++j > 2)
            return (-1);

        if (src[i] > 127 || base64_dec_map[src[i]] == 127)
            return (-1);

        if (base64_dec_map[src[i]] < 64 && j != 0)
            return (-1);

        n++;
    }

    if (n == 0) {
        *olen = 0;
        return (-1);
    }

    n = ((n * 6) + 7) >> 3;
    n -= j;

    if (dst == NULL || dlen < n) {
        *olen = n;
        return (-1);
    }

    for (j = 3, n = x = 0, p = dst; i > 0; i--, src++) {
        if (*src == '\r' || *src == '\n' || *src == ' ')
            continue;

        j -= (base64_dec_map[*src] == 64);
        x = (x << 6) | (base64_dec_map[*src] & 0x3F);

        if (++n == 4) {
            n = 0;
            if (j > 0)
                *p++ = (unsigned char)(x >> 16);
            if (j > 1)
                *p++ = (unsigned char)(x >> 8);
            if (j > 2)
                *p++ = (unsigned char)(x);
        }
    }

    *olen = p - dst;

    return (0);
}

/**
  * @brief  Get configuration parameter.
  * @param  product_id
  * @param  device_name
  * @param  device_secret
  * @param  region
  * @param  cfg_client_id
  * @param  cfg_host_addr
  * @param  cfg_username
  * @param  cfg_password
  * @note   
  * @retval void
  */
void Get_Cfg_Parameters(char *product_id,char *device_name,char *device_secret,char *region,  char *cfg_client_id,char *cfg_host_addr,char *cfg_username, char *cfg_password)
{
    char *client_id;
    char *host_addr;
    char *username;
    char *password;
    int username_len;
    char conn_id[6]; 

    /*1,produce the clientID*/
    client_id = (char*)sAPI_Malloc(MAX_SIZE_OF_CLIENT_ID+1);
    memset(client_id, 0, MAX_SIZE_OF_CLIENT_ID + 1);                                     
    int size = snprintf(client_id, MAX_SIZE_OF_CLIENT_ID, "%s%s", product_id, device_name);
    sAPI_Debug("func[%s] line[%d] size:%d,MQTT get cfg client_id:%s", __FUNCTION__,__LINE__,size, client_id);
    memcpy(cfg_client_id, client_id, MAX_SIZE_OF_CLIENT_ID + 1);
    
    /**********************************************************************************/
    /*2,produce the URL*/    
    host_addr = (char *)sAPI_Malloc(HOST_STR_LENGTH);
    size = snprintf(host_addr, HOST_STR_LENGTH, "tcp://%s.%s:1883", product_id,
    iot_get_mqtt_domain(region));
    sAPI_Debug("func[%s] line[%d] size:%d,MQTT get cfg host_addr:%s", __FUNCTION__,__LINE__, size, host_addr);
    memcpy(cfg_host_addr, host_addr, strlen(host_addr));
    
    /**************************************************************************************************/
    /*3,get username */
    sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
    //SCsysTime_t currUtcTime;
    
    sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
    //sAPI_GetSysLocalTime(&currUtcTime);
    
    //sAPI_Debug("func[%s] line[%d] currUtcTime.tm_year:%d", __FUNCTION__,__LINE__,currUtcTime.tm_year);
    long time = 1608336000;//((currUtcTime.tm_year - 1970)*365*24*60*60) + (currUtcTime.tm_mon*30*24*3600) + (currUtcTime.tm_mday *24*3600) +(currUtcTime.tm_hour*3600) +(currUtcTime.tm_min*60)+currUtcTime.tm_sec;
    sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);

    long cur_timesec = time;
    sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
    long cur_timesec_bak = cur_timesec;
    int  cur_timesec_len = 0;
    while (cur_timesec_bak != 0) 
    {
        cur_timesec_bak /= 10;
        ++cur_timesec_len;
    }
    
    username_len = strlen(client_id) + strlen(QCLOUD_IOT_DEVICE_SDK_APPID) + MAX_CONN_ID_LEN + cur_timesec_len + 4;
    username = (char *)sAPI_Malloc(username_len);
    if (username == NULL) 
    {
        sAPI_Debug("func[%s] line[%d] sAPI_Malloc username failed!", __FUNCTION__,__LINE__);
    }
    get_next_conn_id(conn_id);
    snprintf(username, username_len, "%s;%s;%s;%ld", client_id, QCLOUD_IOT_DEVICE_SDK_APPID,conn_id, cur_timesec);
    sAPI_Debug("func[%s] line[%d] MQTT get cfg username:%s", __FUNCTION__,__LINE__,username);
    //cfg_username = username;
    memset(cfg_username,0, 200);
    snprintf(cfg_username, 200,"%s",username);
   // memcpy(cfg_username, username, strlen(username));
    
    sAPI_Debug("func[%s] line[%d]cfg_username:%s ", __FUNCTION__,__LINE__,cfg_username);
    
 
/***************************************************************/
/*4,get password*/
     size_t src_len = strlen(device_secret);
     sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
     unsigned char psk_decode[48];
     sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
     
     size_t len;
     sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
     int device_secret_len; 
     sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
     
     memset(psk_decode, 0x00, 48);
     
     sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
     qcloud_iot_utils_base64decode(psk_decode, 48, &len,\
                                 (unsigned char *)device_secret, src_len);
     
                                 sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
     device_secret     = (char *)psk_decode;
     device_secret_len = len;
     
     sAPI_Debug("func[%s] line[%d] ", __FUNCTION__,__LINE__);
     if (device_secret != NULL && username != NULL) 
     {
         char sign[41] = {0};
         utils_hmac_sha1(username, strlen(username), sign, device_secret,
                         device_secret_len);
         password = (char *)sAPI_Malloc(51);
         if (password == NULL) 
         {
             sAPI_Debug("func[%s] line[%d] sAPI_Malloc password failed!", __FUNCTION__,__LINE__);
         }
         snprintf(password, 51, "%s;hmacsha1", sign);
         sAPI_Debug("func[%s] line[%d] MQTT get cfg password:%s", __FUNCTION__,__LINE__,password);
         //cfg_password = password;
         memcpy(cfg_password, password, 51);
        
         sAPI_Debug("func[%s] line[%d] leave", __FUNCTION__,__LINE__);

         if(client_id != NULL)
             sAPI_Free(client_id);
         if(host_addr != NULL)
             sAPI_Free(host_addr);
         if(username != NULL)
             sAPI_Free(username);
         if(password != NULL)
             sAPI_Free(password);
     }
     
/*****************************************/

}

