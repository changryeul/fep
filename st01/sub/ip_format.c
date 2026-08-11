/*------------------------------------------------------------------------
#   Module  : Shared utility functions
#   File    : ip_format.c
------------------------------------------------------------------------*/

#include    "fep_fepp.h"

/*------------------------------------------------------------------------
    Function    : format_ip_addr
    Description : Convert 12-byte packed IP (3 chars per octet) to dotted notation
    Parameters  : packed_ip   - 12-byte packed IP string (e.g. "010001002003")
                  dotted_ip   - output buffer for "x.x.x.x" string
                  buf_size    - size of output buffer
------------------------------------------------------------------------*/
void format_ip_addr(const char *packed_ip, char *dotted_ip, int buf_size) {
    int     in1, in2, in3, in4;

    in1 = AtoIf((char *)packed_ip,      3);
    in2 = AtoIf((char *)&packed_ip[3],  3);
    in3 = AtoIf((char *)&packed_ip[6],  3);
    in4 = AtoIf((char *)&packed_ip[9],  3);

    memset(dotted_ip, 0x00, buf_size);
    sprintf(dotted_ip, "%d.%d.%d.%d", in1, in2, in3, in4);
}
