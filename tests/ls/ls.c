// Used for basic input/output stream
#include <stdio.h>
// Used for handling directory files
#include <dirent.h>
// For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>

void _ls(const char* dir, int op_a, int op_l) {
    // Here we will list the directory
    struct dirent* d;
    DIR* dh = opendir(dir);
    if (!dh) {
        if (errno == ENOENT) {
            // If the directory is not found
            perror("Directory doesn't exist");
        } else {
            // If the directory is not readable then throw error and exit
            perror("Unable to read directory");
        }
        exit(EXIT_FAILURE);
    }
    // While the next entry is not readable we will print directory files
    while ((d = readdir(dh)) != NULL) {
        // If hidden files are found we continue
        if (!op_a && d->d_name[0] == '.') continue;
        printf("%s  ", d->d_name);
        if (op_l) printf("\n");
    }
    if (!op_l) printf("\n");
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        _ls(".", 0, 0);
    } else if (argc == 2) {
        if (argv[1][0] == '-') {
            // Checking if option is passed
            // Options supporting: a, l
            int op_a = 0, op_l = 0;
            char* p = (char*)(argv[1] + 1);
            while (*p) {
                if (*p == 'a')
                    op_a = 1;
                else if (*p == 'l')
                    op_l = 1;
                else {
                    perror("Option not available");
                    exit(EXIT_FAILURE);
                }
                p++;
            }
            _ls(".", op_a, op_l);
        }
    }
    return 0;
}
