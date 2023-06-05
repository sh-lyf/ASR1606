#include "token.h"
#include "onenet_base64.h"
#include "onenet_hmac_sha1.h"
#include "onenet_md5.h"

#include <stdio.h>
#include <string.h>
#include "simcom_os.h"
#include "simcom_debug.h"



#define OTA_IP		"183.230.40.50"

#define OTA_PORT	"80"


OTA_INFO ota_info;


//static iapfun jump2app; 

static unsigned char OTA_UrlEncode(char *sign)
{

    char sign_t[40];
    unsigned char i = 0, j = 0;
    unsigned char sign_len = strlen(sign);

    if(sign == (void *)0 || sign_len < 28)
    	return 1;

    for(; i < sign_len; i++)
    {
        sign_t[i] = sign[i];
        sign[i] = 0;
    }
    sign_t[i] = 0;

    for(i = 0, j = 0; i < sign_len; i++)
    {
    switch(sign_t[i])
    {
        case '+':
            strcat(sign + j, "%2B");j += 3;
        break;

        case ' ':
            strcat(sign + j, "%20");j += 3;
        break;

        case '/':
            strcat(sign + j, "%2F");j += 3;
        break;

        case '?':
            strcat(sign + j, "%3F");j += 3;
        break;

        case '%':
            strcat(sign + j, "%25");j += 3;
        break;

        case '#':
            strcat(sign + j, "%23");j += 3;
        break;

        case '&':
            strcat(sign + j, "%26");j += 3;
        break;

        case '=':
            strcat(sign + j, "%3D");j += 3;
        break;

        default:
            sign[j] = sign_t[i];j++;
        break;
    }
    }

    sign[j] = 0;

    return 0;

}


#define METHOD		"sha1"
unsigned char OTA_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *authorization_buf, unsigned short authorization_buf_len)
{
    
    sAPI_Debug( "func[%s] line[%d] MQTT get ver:%s,res:%s,et:%d,access_key:%s", __FUNCTION__,__LINE__,ver,res,et,access_key);

    size_t olen = 0;

    char sign_buf[40];
    char hmac_sha1_buf[41];
    char access_key_base64[40];
    char string_for_signature[100];

    //----------------------------------------------------judge the range--------------------------------------------------------------------
    if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
        || authorization_buf == (void *)0 || authorization_buf_len < 120)
        return 1;

    //----------------------------------------------------decode the access_key----------------------------------------------------
    memset(access_key_base64, 0, sizeof(access_key_base64));
    BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
    int n = 0;
    for(n = 0;n<40;n++)
    {
        sAPI_Debug( "func[%s] line[%d],access_key_base64: %x", __FUNCTION__,__LINE__,access_key_base64[n]);
    }
    //----------------------------------------------------construct the string_for_signature-----------------------------------------------------
    memset(string_for_signature, 0, sizeof(string_for_signature));
    snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\n%s\n%s", et, METHOD, res, ver);

    sAPI_Debug( "func[%s] line[%d],string_for_signature: %s", __FUNCTION__,__LINE__,string_for_signature);

    //----------------------------------------------------hmac the buf-------------------------------------------------------------------------
    memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));

    hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
                (unsigned char *)string_for_signature, strlen(string_for_signature),
                (unsigned char *)hmac_sha1_buf);
    int m = 0;
    for(m = 0; m < 40;m++)
    {
        sAPI_Debug( "func[%s] line[%d],hmac_sha1_buf: %x", __FUNCTION__,__LINE__,hmac_sha1_buf[n]);
    }
    //----------------------------------------------------encode the hmac buf------------------------------------------------------
    olen = 0;
    memset(sign_buf, 0, sizeof(sign_buf));
    BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

    //----------------------------------------------------url code---------------------------------------------------
    OTA_UrlEncode(sign_buf);
    OTA_UrlEncode(res);
    sAPI_Debug( "func[%s] line[%d],sign_buf: %s", __FUNCTION__,__LINE__,sign_buf);

    //----------------------------------------------------construct the token buff--------------------------------------------------------------------
    snprintf(authorization_buf, authorization_buf_len, "version=%s&res=%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
    //snprintf(authorization_buf, authorization_buf_len, "vers&ion=&%s&res=%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);

    sAPI_Debug( "func[%s] line[%d],authorization_buf: %s,%d,%d ", __FUNCTION__,__LINE__,authorization_buf,authorization_buf_len,strlen(authorization_buf));

    return 0;

}

