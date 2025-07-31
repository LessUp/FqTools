#ifndef GZSTREAM_H
#define GZSTREAM_H

#include <iostream>
#include <fstream>
#include <string>
#include <zlib.h>

class igzstream : public std::istream {
public:
    igzstream() : std::istream(nullptr), buffer(nullptr) {}
    explicit igzstream(const std::string& filename) : std::istream(nullptr), buffer(nullptr) {
        open(filename);
    }
    ~igzstream() { close(); }
    
    void open(const std::string& filename) {
        close();
        buffer = new gzstreambuf(filename);
        rdbuf(buffer);
    }
    
    void close() {
        if (buffer) {
            delete buffer;
            buffer = nullptr;
            rdbuf(nullptr);
        }
    }
    
    bool is_open() const { return buffer != nullptr; }
    
private:
    class gzstreambuf : public std::streambuf {
    public:
        explicit gzstreambuf(const std::string& filename) {
            file = gzopen(filename.c_str(), "rb");
            if (file) {
                setg(buffer, buffer, buffer);
            }
        }
        ~gzstreambuf() {
            if (file) gzclose(file);
        }
        
    protected:
        int underflow() override {
            if (gptr() < egptr()) return *gptr();
            
            int result = gzread(file, buffer, bufferSize);
            if (result <= 0) {
                return EOF;
            }
            
            setg(buffer, buffer, buffer + result);
            return *gptr();
        }
        
    private:
        gzFile file;
        static const int bufferSize = 8192;
        char buffer[bufferSize];
    };
    
    gzstreambuf* buffer;
};

class ogzstream : public std::ostream {
public:
    ogzstream() : std::ostream(nullptr), buffer(nullptr) {}
    explicit ogzstream(const std::string& filename) : std::ostream(nullptr), buffer(nullptr) {
        open(filename);
    }
    ~ogzstream() { close(); }
    
    void open(const std::string& filename) {
        close();
        buffer = new gzstreambuf(filename);
        rdbuf(buffer);
    }
    
    void close() {
        if (buffer) {
            delete buffer;
            buffer = nullptr;
            rdbuf(nullptr);
        }
    }
    
    bool is_open() const { return buffer != nullptr; }
    
private:
    class gzstreambuf : public std::streambuf {
    public:
        explicit gzstreambuf(const std::string& filename) {
            file = gzopen(filename.c_str(), "wb");
            setp(buffer, buffer + bufferSize);
        }
        ~gzstreambuf() {
            sync();
            if (file) gzclose(file);
        }
        
    protected:
        int overflow(int c) override {
            if (c != EOF) {
                *pptr() = c;
                pbump(1);
            }
            return sync();
        }
        
        int sync() override {
            int count = pptr() - pbase();
            if (count > 0) {
                if (gzwrite(file, pbase(), count) != count) {
                    return -1;
                }
                setp(buffer, buffer + bufferSize);
            }
            return 0;
        }
        
    private:
        gzFile file;
        static const int bufferSize = 8192;
        char buffer[bufferSize];
    };
    
    gzstreambuf* buffer;
};

#endif // GZSTREAM_H