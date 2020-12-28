#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // socket
#include <unistd.h>      // write

#define EXIT_FAILURE 1
#define PROT_ICMP 1

#ifndef AF_INET
#define AF_INET 2      // Internet IP protocol
#define SOCK_STREAM 1  // for TCP
#define SOCK_DGRAM 2   // for UDP
#endif

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
};

/**
 * functions
 **/
uint8_t StrToByte(const char* s, const char** next) {
  uint32_t v = 0;
  while ('0' <= *s && *s <= '9') {
    v = v * 10 + *s - '0';
    s++;
  }
  if (next) {
    *next = s;
  }
  return v;
}

in_addr_t MakeIPv4AddrFromString(const char* s) {
  // "a.b.c.d" -> in_addr_t (=uint32_t)
  uint8_t buf[4];
  for (int i = 0;; i++) {
    buf[i] = StrToByte(s, &s);
    if (i == 3) break;
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
int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: ");
    printf("%s", argv[0]);
    printf(" <ip addr>\n");
    exit(EXIT_FAILURE);
  }

  char* ip_addr_s = argv[1];
  printf("PING %s :\n", ip_addr_s);

  // open socket
  int soc = socket(AF_INET, SOCK_DGRAM, PROT_ICMP);
  if (soc == -1) {
    printf("[open socket failed] soc: %d / errno=%d\n", soc, errno);
    exit(EXIT_FAILURE);
  }

  struct ICMPMessage icmp;
  memset(&icmp, 0, sizeof(icmp));
  icmp.type = 8; /* Echo Request */
  icmp.checksum = CalcChecksum(&icmp, 0, sizeof(icmp));
  printf("icmp: len=%ld | %d %d %d %d %d\n", 
         sizeof(icmp),
         icmp.type,
         icmp.code,
         icmp.checksum,
         icmp.identifier,
         icmp.sequence);

  struct socketaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = MakeIPv4AddrFromString(ip_addr_s);

  int n = sendto(soc, &icmp, sizeof(icmp), 0, (struct sockaddr*)&addr,
                 sizeof(addr));
  if (n < 1) {
    printf("[sent packet failed] errno=%d\n", errno);
    exit(EXIT_FAILURE);
  }

  printf("soc: %d, n: %d\n", soc, n);

  uint8_t recv_buf[256];
  socklen_t addr_size;
  int recv_len = recvfrom(soc, &recv_buf, sizeof(recv_buf), 0,
                          (struct sockaddr*)&addr, &addr_size);
  if (recv_len < 1) {
    printf("[sent packet failed] errno=%d\n", errno);
  }

  printf("recvfrom returned: %d\n", recv_len);

  printf("ICMP packet received from %s\n", ip_addr_s);

  for (int i = 0; i < recv_len; i++) {
    printf("%02x ", recv_buf[i]);
    if (i != 0 && i % 8 == 0) {
      printf("\n");
    }
  }
  printf("\n");

  struct ICMPMessage* recv_icmp = (struct ICMPMessage*)(recv_buf);
  printf("checksum: (sent)%d (recv)%d\n", icmp.checksum, recv_icmp->checksum);

  printf("type: (sent)%d (recv)%d\n", icmp.type, recv_icmp->type);
}
