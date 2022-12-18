/**
 * File: crypt.cc
 * --------------
 * Presents a short program that leverages the
 * CryptFile class to create encrypted files from
 * plaintext ones and vice versa.
 * 
 * This program does less error checking than you
 * should in Project 5, but only because we want to
 * keep the example as short and focused as possible.
 */

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include "cryptfile.hh"

class PlaintextFile {
public:
    PlaintextFile(const char *path, mode_t mode)
        : path_(path), fd_(open(path, mode, 0666)) {}
    ~PlaintextFile() { close(fd_); }
    int aligned_pread(void *dst, size_t len, size_t offset) {
        return pread(fd_, dst, len, offset);
    }
    int aligned_pwrite(const void *src, size_t len, size_t offset) {
        return pwrite(fd_, src, len, offset);
    }
    
private:
    const char *path_;
    int fd_;
};

template<typename In, typename Out>
static void copy(In &in, Out &out) {
    // for you to implement
    char buf[8192];
    size_t n = sizeof(buf);
    for (size_t offset = 0; n == sizeof(buf); offset += sizeof(buf)) {
        n = in.aligned_pread(buf, sizeof(buf), offset);
        size_t m = n;
        if (size_t remainder = m % CryptFile::blocksize) {
            m += CryptFile::blocksize - remainder;
            memset(buf + n, '/0', m - n);
        }
        out.aligned_pwrite(buf, m, offset);
    }
}

[[noreturn]] static void usage(const char *progname) {
    std::cerr << "usage: " << progname
              << " {-d | -e} infile outfile" << std::endl;
    exit(1);
}

int main(int argc, char *argv[]) {
    const char *progname = "crypt";
    if (argc != 4) usage(progname);
    if (strcmp(argv[1], "-d") != 0 &&
        strcmp(argv[1], "-e") != 0) usage(progname);
    bool decrypt = strcmp(argv[1], "-d") == 0;
    
    Key key("cs111-project-5-key"); // arbitrary
    try {
        if (decrypt) {
            CryptFile in(key, argv[2]);
            PlaintextFile out(argv[3], O_CREAT | O_WRONLY | O_TRUNC);
            copy(in, out);
        } else {
            PlaintextFile in(argv[2], O_RDONLY);
            CryptFile out(key, argv[3]);
            copy(in, out);
        }
    } catch (const std::exception &e) {
        std:: cerr << progname << ": " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
