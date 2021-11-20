#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdhci.h"
#include "syscon.h"
#include "debug_log.h"

static uint32_t rca = 0;

void sdcard_init(void)
{
	uint32_t resp[4] = {0};

	/* init sdhci */
    sdhci_init(SDHCI_ADDR);

	/* send cmd0 */
    sdhci_send_cmd(SDHCI_ADDR,0,0x0,SDHCI_CMD_RESP_NONE,NULL);

	/* send cmd8 */
    sdhci_send_cmd(SDHCI_ADDR,8,0x000001aa,SDHCI_CMD_RESP_SHORT,NULL);
	//sdhci_read_resp(SDHCI_ADDR,resp,SDHCI_CMD_RESP_SHORT);
    //debug_log("cmd8 resp 0x%x\n",resp[0]);

	do {
		/* send cmd55 */
		sdhci_send_cmd(SDHCI_ADDR,55,0x0,SDHCI_CMD_RESP_SHORT,NULL);
		//sdhci_read_resp(SDHCI_ADDR,resp,SDHCI_CMD_RESP_SHORT);
		//debug_log("cmd55 resp 0x%x\n",resp[0]);

		/* send acmd41 */
		sdhci_send_cmd(SDHCI_ADDR,41,0x40300000,SDHCI_CMD_RESP_SHORT,NULL);
		sdhci_read_resp(SDHCI_ADDR,resp,SDHCI_CMD_RESP_SHORT);
		//debug_log("acmd41 resp 0x%x\n",resp[0]);
		if(resp[0]>>31) {
			break;
		}
	}while(1);

	/* send cmd2 */
    sdhci_send_cmd(SDHCI_ADDR,2,0x0,SDHCI_CMD_RESP_LONG,NULL);
	//sdhci_read_resp(SDHCI_ADDR,resp,SDHCI_CMD_RESP_LONG);
    //debug_log("cmd2 resp 0x%x 0x%x 0x%x 0x%x\n",resp[0],resp[1],resp[2],resp[3]);

	/* send cmd3 */
    sdhci_send_cmd(SDHCI_ADDR,3,0x0,SDHCI_CMD_RESP_SHORT,NULL);
	sdhci_read_resp(SDHCI_ADDR,resp,SDHCI_CMD_RESP_SHORT);
    //debug_log("cmd3 resp 0x%x\n",resp[0]);
	rca=resp[0]&0xFFFF0000;

	/* send cmd7 */
    sdhci_send_cmd(SDHCI_ADDR,7,rca,SDHCI_CMD_RESP_SHORT,NULL);
	//sdhci_read_resp(SDHCI_ADDR,resp,SDHCI_CMD_RESP_SHORT);
    //debug_log("cmd7 resp 0x%x\n",resp[0]);

    //debug_log("sdcard_init done\n");
}

void sdcard_load(uint64_t addr, uint32_t offset, uint32_t size)
{
    uint8_t *buffer = (uint8_t *)addr;
	//uint32_t resp = 0;

    debug_log("Load address: 0x%lx\nLoading: ",addr);
	
	/* send cmd16 */
	sdhci_send_cmd(SDHCI_ADDR,16,512,SDHCI_CMD_RESP_SHORT,NULL);
	//sdhci_read_resp(SDHCI_ADDR,&resp,SDHCI_CMD_RESP_SHORT);
    //debug_log("cmd16 resp 0x%x\n",resp);

    for(int i=0;i < size; i+=512) {
        /* send cmd17 */
		sdhci_send_cmd(SDHCI_ADDR,17,offset+i,SDHCI_CMD_RESP_SHORT,buffer+i);
		//sdhci_read_resp(SDHCI_ADDR,&resp,SDHCI_CMD_RESP_SHORT);
		//debug_log("cmd17 resp 0x%x\n",resp);
        if((i%(size/64))==0){
            debug_log("#");
        }
    }
    if(size >= 1024*1024) {
        debug_log(" %dMiB\n",size/1024/1024);
    } else if(size >= 1024) {
        debug_log(" %dKiB\n",size/1024);
    } else {
        debug_log(" %dB\n",size);
    }
}