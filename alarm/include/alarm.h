#ifndef ALRAM_H
#define ALRAM_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// msg¿« ±∏º∫ 
// datahdr(SERACH_HDR) + body 

int alert_msg( int msgno, char *msg, int msg_len);

#endif
