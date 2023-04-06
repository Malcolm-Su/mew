#include "FileUtil.h"


///////////////////// implementations ////////////////////////////
namespace mew {

AppendFile::AppendFile(std::string filename) : fp_(fopen(filename.c_str(), "ae")) {
  // 用户提供缓冲区
  setbuffer(fp_, buffer_, sizeof(buffer_));
}

AppendFile::~AppendFile() { fclose(fp_); }

void AppendFile::Append(const char* logline, const size_t len) {
    size_t n = this->Write(logline, len);
    size_t remain = len - n;
    while (remain > 0) {
        size_t x = this->Write(logline + n, remain);
        if (x == 0) {
        int err = ferror(fp_);
        if (err) fprintf(stderr, "AppendFile::append() failed !\n");
        break;
        }
        n += x;
        remain = len - n;
    }
}

void AppendFile::Flush() { fflush(fp_); }

size_t AppendFile::Write(const char* logline, size_t len) {
    return fwrite_unlocked(logline, 1, len, fp_);
}


} // namespace mew