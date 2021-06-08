#ifndef __DEF_COMMON_H__
	#define __DEF_COMMON_H__

#include <stdio.h>   	//printf
#include <string.h>	//strncpy
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>	//close
#include <sys/time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/input.h>
#include <linux/keyboard.h>
#include <linux/kd.h>
#include <net/if.h>   //ifreq
#include <sys/stat.h>
#include <list>
#include <map>
#include <algorithm>
#include <stdarg.h>
#include <iostream>
#include <syslog.h>
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <asm-generic/ioctls.h>
#include <sstream>
#include <vector>
#include <assert.h>

//formula
#include <memory>
#include <array>

//share memory
#include <sys/ipc.h>
#include <sys/shm.h>

/* include json library */
//#include <json/json.h>

/* include sqlite3 library */
//#include <sqlite3.h>

using namespace std;

/* ----------------------------------------------------------------------------
 * 定義
 --------------------------------------------------------------------------- */
#define DEF_SW_VERSION "0.0.1"
#define DEF_LOOP_UNIT_TIME 20		//unit is ms
typedef bool (*net_call_back)(  int ulen, char*, char*, int* );		//for net call back function

enum __ERROR_NO__ {
	DEF_ERROR_NONE                             ,
	DEF_SUCCESS = DEF_ERROR_NONE               ,  // 成功
	DEF_RUNING_FISISH = DEF_SUCCESS            ,  // 執行完成
	DEF_FAIL                                   ,  // 失敗
	DEF_IS_RUNING = DEF_FAIL                   ,  // 執行中
	DEF_ERROR_POINTER_IS_NULL                  ,
	DEF_ERROR_OPEN_FILE_FAIL                   ,

	/* json */
	DEF_ERROR_JSON_NEW_FAIL                    , // json crate new doc fail
	DEF_ERROR_JSON_NO_INPUT_FILE               , // no input file handle
	DEF_ERROR_JSON_NO_DOC_POINTER              , // no json doc pointer
	DEF_ERROR_JSON_ADD_FAIL                    , // add data to doc fail

	/* config */
	DEF_ERROR_CONFIG_FORMAT_FAIL               , // 資料格式錯誤

	/* conf_base */
	DEF_ERROR_CONF_BASE_FILENAME_IS_NULL       ,
	DEF_ERROR_CONF_BASE_OPEN_FILE_FAIL         ,

	/* texture manager */
	DEF_ERROR_TEX_MANAGER_REGISTER_GROUP_FAIL  ,
	DEF_ERROR_TEX_MANAGER_HAD_REGISTER         ,
	DEF_ERROR_TEX_MANAGER_NEW_TEXTURE_MAP_FAIL ,
} ;

#endif
