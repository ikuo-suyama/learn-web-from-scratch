#include <stdlib.h>

struct __attribute__((packed)) ICMPMessage {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t identifier:
  uint16_t sequence;
}

#define EXIT_FAILURE 1

void Print(const char* s) {
  write(1, s, strlen(s));
}

int main(int argc, char**argv) {
  if (argc != 2) {
    Print("Usage: ");
    Print(argv[0]);
    Print(" <ip addr>\n");
    exit(EXIT_FAILURE);
  }
  Print(argv[0]);
  Print(argv[1]);
}

