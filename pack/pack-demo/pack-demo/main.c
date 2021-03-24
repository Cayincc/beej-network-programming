//
//  main.c
//  pack-demo
//
//  Created by wan zhongwen on 2021/3/17.
//

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "pack.h"

typedef float float32_t;
typedef double float64_t;

int main(int argc, const char * argv[])
{
    unsigned char buf[1024];
    int8_t magic;
    int16_t monkeycount;
    int32_t altitude;
    float32_t absurdityfactor;
    char *s = "Great unmitigated Zot! You've found the Runestaff!";
    char s2[96];
    int16_t packetsize, ps2;

    memset(&buf, 0, sizeof(buf));
    
//    unsigned long int i2 = 0x12345678;
//    unsigned char *buf2 = (unsigned char *)buf;
//    *(buf2++) = i2 >> 24;
//    *buf2++ = i2 >> 16;
//    *buf2++ = i2 >> 8;
//    *buf2++ = i2;
    
    
    float32_t f1 = (float32_t)-3490.6677;
    
    packetsize = pack(buf, "chhlsf", (int8_t)'B', (int16_t)0, (int16_t)37, (int32_t)-5, s, f1);
    packi16(buf+1, packetsize); // store packet size in packet for kicks

    printf("packet is %" PRId16 " bytes\n", packetsize);

    unpack(buf, "chhl96sf", &magic, &ps2, &monkeycount, &altitude, s2,
       &absurdityfactor);

    printf("'%c' %" PRId16" %" PRId16 " %" PRId32
           " \"%s\" %.4f\n", magic, ps2, monkeycount,
           altitude, s2, absurdityfactor);

    return 0;
}
