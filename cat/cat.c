#include <unistd.h>
int main() {
    char buf[1024];
    ssize_t bytesRead = -1;
    do {
        bytesRead = read(STDIN_FILENO, buf, sizeof(buf));
        if (bytesRead > 0) {
            ssize_t bytesWritten = 0;
            while(bytesWritten - bytesRead != 0) {
                ssize_t bytes = write(STDOUT_FILENO, buf + bytesWritten, bytesRead - bytesWritten);
                if(bytes == -1) {
                    return 1;
                }
                bytesWritten += bytes;
            }
        } else if (bytesRead < 0) {
            return 1;
        }
    } while (bytesRead != 0);
    return 0;
}
