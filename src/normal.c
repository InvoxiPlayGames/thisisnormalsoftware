#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define DUMP_PAGE_SIZE 4096

int write_to_process(pid_t pid, uint64_t address, void *buffer, size_t len)
{
    struct iovec local = {0};
    struct iovec remote = {0};
    local.iov_base = buffer;
    local.iov_len = len;
    remote.iov_base = (void *)address;
    remote.iov_len = len;
    int r = process_vm_writev(pid, &local, 1, &remote, 1, 0);
    return r;
}

int read_from_process(pid_t pid, uint64_t address, void *buffer, size_t len)
{
    struct iovec local = {0};
    struct iovec remote = {0};
    local.iov_base = buffer;
    local.iov_len = len;
    remote.iov_base = (void *)address;
    remote.iov_len = len;
    int r = process_vm_readv(pid, &local, 1, &remote, 1, 0);
    return r;
}

int main(int argc, char **argv)
{
    // -- initialisation and parsing arguments --
    bool isWrite;
    size_t address = 0x140000000;
    size_t length = 0x20000000;
    pid_t pid = 1337;

    // argv  = name, mode, pid, address
    // read  =                         , length, outfile
    // write =                         , infile
    if (argc < 4) {
        printf("usage: %s [mode] [pid] [address] [args...]\n", argv[0]);
        return -1;
    }

    char *mode = argv[1];
    char *pidStr = argv[2];
    char *addressStr = argv[3];
    char *lengthStr = NULL;
    char *filename = NULL;
    if (strcasecmp(mode, "read") == 0) {
        isWrite = false;
        if (argc < 6) {
            printf("usage: %s read [pid] [address] [length] [outfile]\n", argv[0]);
            return -1;
        }
        lengthStr = argv[4];
        filename = argv[5];
    } else if (strcasecmp(mode, "write") == 0) {
        isWrite = true;
        if (argc < 5) {
            printf("usage: %s write [pid] [address] [outfile]\n", argv[0]);
            return -1;
        }
        filename = argv[4];
    } else {
        printf("unknown mode\n");
        return -1;
    }

    int pidParsed = strtol(pidStr, NULL, 0);
    if (errno == EINVAL || errno == ERANGE) {
        printf("failed to parse PID: %s\n", pidStr);
        return -1;
    }
    pid = (pid_t)pidParsed;

    long long addressParsed = strtoll(addressStr, NULL, 16);
    if (errno == EINVAL || errno == ERANGE) {
        printf("failed to parse address: %s\n", addressStr);
        return -1;
    }
    address = (size_t)addressParsed;

    FILE *fp = fopen(filename, isWrite ? "rb" : "wb");
    if (fp == NULL) {
        printf("failed to open file: %s\n", filename);
        return -1;
    }
    
    if (isWrite == false)
    {
        // Read mode code
        long long lengthParsed = strtoll(lengthStr, NULL, 0);
        if (errno == EINVAL || errno == ERANGE) {
            printf("failed to parse address: %s\n", addressStr);
            return -1;
        }
        length = (size_t)lengthParsed;

        printf("Reading 0x%llx bytes from 0x%llx in %i to '%s'...", length, address, pid, filename);

        uint8_t dumpbuf[DUMP_PAGE_SIZE];
        size_t offset = 0;
        while (offset < length) {
            int r = read_from_process(pid, address + offset, dumpbuf, DUMP_PAGE_SIZE);
            fseek(fp, offset, SEEK_SET);
            if (r > 0)
                fwrite(dumpbuf, 1, r, fp);
            offset += DUMP_PAGE_SIZE;
        }
        printf("ok.\n", offset);
    } else
    {
        // Write mode code
        printf("Writing 0x%llx bytes from '%s' to 0x%llx in %i...", length, filename, address, pid);
        uint8_t readbuf[DUMP_PAGE_SIZE];
        size_t offset = 0;
        size_t r = DUMP_PAGE_SIZE;
        while (r > 0) {
            r = fread(readbuf, 1, DUMP_PAGE_SIZE, fp);
            if (r > 0) {
                write_to_process(pid, address + offset, readbuf, r);
            }
            offset += r;
        }
        printf("ok (0x%x).\n", offset);
    }

    fclose(fp);

    return 0;
}
