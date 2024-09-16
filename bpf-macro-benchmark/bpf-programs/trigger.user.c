#include <stdio.h>
#include <unistd.h>

int main() {
    char buf[256];
    getcwd(buf, 256);
    return 0;
}