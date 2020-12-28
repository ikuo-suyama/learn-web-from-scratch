#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define EXIT_FAILURE 1

#define AF_INET 2 // Internet IP protocol

#define SOCK_STREAM 1 // for TCP
#define SOCK_DGRAM 2 // for UDP
#define PROT_ICMP 1

/**
 * type and structs
 **/
typedef uint32_t in_addr_t;

struct __attribute__((packed)) ICMPMessage {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t identifier;
  uint16_t sequence;
};

// c.f.
// https://elixir.bootlin.com/linux/v4.15/source/include/uapi/linux/in.h#L85
struct in_addr {
  uint32_t s_addr;
};

struct socketaddr_in {
  uint16_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  uint8_t padding[8];
}

void Print(const char* s) {
  write(1, s, strlen(s));
}

/**
 * functions
 **/
in_addr_t MakeIPv4AddrFromString(const char* s) {
  // "a.b.c.d" -> in_addr_t (=uint32_t)
  uint8_t buf[4];
  for (int i = 0;; i++) {
    buf[i] = StrToByte(s, &s);
    if (i == 3)
      break;
    assert(*s == '.');
    s++;
  }
  return *(uint32_t*)buf;
}

uint16_t CalcChecksum(void* buf, size_t start, size_t end) {
  // https://tools.ietf.org/html/rfc1071
  uint8_t* p = buf;
  uint32_t sum = 0;
  for (size_t i = start; i < end; i += 2) {
    sum += ((uint16_t)p[i + 0]) << 8 | p[i + 1];
  }
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  sum = ~sum;
  return ((sum >> 8) & 0xFF) | ((sum & 0xFF) << 8);
}


/**
 * main 
 **/
int main(int argc, char**argv) {
  if (argc != 2) {
    Print("Usage: ");
    Print(argv[0]);
    Print(" <ip addr>\n");
    exit(EXIT_FAILURE);
  }

  int soc = socket(AF_INET, SOCK_DGRAM, PROT_ICMP);

  struct ICMPMessage icmp;
  memset(&icmp, 0, sizeof(icmp));
  icmp.type = 8; /* Echo Request */
  icmp.checksum = CalcChecksum(&icmp, 0, sizeof(icmp));

  struct socketaddr_in addr;
  addr.sin_family = AF_INET;
//  addr.sin_addr.s_addr ;

}

