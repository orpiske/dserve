/*
 * defaults.h
 *
 *  Created on: Jan 13, 2011
 *      Author: otavio
 */

#ifndef DEFAULTS_H_
#define DEFAULTS_H_

#define OPT_MAX_STR_SIZE 256
#define OPT_MAX_STR_SIZE_MASK "%256s"

// Server
#define OPT_DEFAULT_MAX_CONCURRENT_DOWNLOADS 20
#define OPT_DEFAULT_Q_RETRY_COUNT 10
#define OPT_DEFAULT_Q_RETRY_WAIT 100
#define OPT_DOWNLOAD_DIR "Downloads"
#define OPT_LOG_DIR "logs"
#define OPT_DEFAULT_ALTERNATE_DIR "tmp"
#define DS_USER_AGENT "dserver"
#define DS_VERSION "0.0.1"

// Both
#define OPT_DEFAULT_PORT 62434
#define OPT_DEFAULT_TIMEOUT 10
#ifdef DSSERVER
 #define DS_DEFAULT_DIR ".dsserver"
 #define OPT_DEFAULT_HOST "0.0.0.0"
 #define DS_CONFIG_FILE "dsserver.conf"
#else
 #define OPT_DEFAULT_HOST "127.0.0.1"
 #define DS_DEFAULT_DIR ".dsclient"
 #define DS_CONFIG_FILE "dsclient.conf"
#endif

#ifndef FILE_SEPARATOR
#define FILE_SEPARATOR "/"
#endif





#endif /* DEFAULTS_H_ */
