#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "simcom_os.h"
#include "simcom_common.h"
#include "sc_app_update.h"
#include "simcom_file.h"

#define APP_FILE_NAME   "customer_app.bin"
#define LOG_ERR()         do{printf("%s error, line: %d\r\n", __func__, __LINE__);}while (0)

void AppUpdateDemo(void)
{
    int ret = -1;
    SCFILE *fp = NULL;
    size_t read_len = 0;
    SCAppPackageInfo pInfo = {0};

    void *read_data = (void *)malloc(1024);
    if (!read_data) {
        LOG_ERR();
        goto error;
    }

    ret = sAPI_FsSwitchDir(APP_FILE_NAME, DIR_C_TO_SIMDIR); /* swich the customwer_app.bin path from c:/ to c:/simdir, or the API of the file system cannot operate it*/
    if (ret) {
        LOG_ERR();
        goto error;
    }

    fp = sAPI_fopen("c:/customer_app.bin", "rb");
    if (fp == NULL) {
        LOG_ERR();
        goto error;
    }
    if (sAPI_AppPackageOpen("w") != 0) {
        LOG_ERR();
        goto error;
    }
    while (1) {
        memset(read_data, 0, 1024);
        read_len = sAPI_fread(read_data, 1024, 1, fp);
        if (read_len == 0)
            break;
        if (sAPI_AppPackageWrite(read_data, read_len) != 0) {
            LOG_ERR();
            goto error;
        }
    }

    if (sAPI_AppPackageClose() != 0) {
        LOG_ERR();
        goto error;
    }
    if (sAPI_AppPackageCrc(&pInfo) != 0) {
        LOG_ERR();
        goto error;
    }
    printf("app write suc --> reset --> update!");
error:
    if (fp)
        sAPI_fclose(fp);
    if (read_data)
        free(read_data);
    return;
}