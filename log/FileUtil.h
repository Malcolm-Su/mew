#ifndef MEW_FILEUTIL_H
#define MEW_FILEUTIL_H

#include <string>
namespace mew {

class AppendFile {
private:
    FILE *fp_;
    char buffer_[64 * 1024];
private:
    AppendFile(const AppendFile&)            = delete;
    AppendFile& operator=(const AppendFile&) = delete;

    size_t Write(const char *logline, size_t len);
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    
    void Append(const char *logline, const size_t len);
    void Flush();
};


} // namespace mew 
#endif // MEW_FILEUTIL_H