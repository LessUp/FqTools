/**
 * @file gzstream.h
 * @brief gzip 压缩流处理类
 * @details 该文件提供了基于 zlib 的 gzip 压缩文件流处理类，
 *          支持 gzip 格式文件的读取和写入操作，继承自标准库的 iostream
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#ifndef GZSTREAM_H
#define GZSTREAM_H

#include <iostream>
#include <fstream>
#include <string>
#include <zlib.h>

/**
 * @brief gzip 输入流类
 * @details 该类继承自 std::istream，提供对 gzip 压缩文件的读取功能
 *          支持标准的输入流操作，自动解压读取的数据
 * 
 * @note 使用前需要确保 zlib 库已正确安装
 * @warning 不支持多线程并发读取同一文件
 */
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

/**
 * @brief gzip 输出流类
 * @details 该类继承自 std::ostream，提供对 gzip 压缩文件的写入功能
 *          支持标准的输出流操作，自动压缩写入的数据
 * 
 * @note 使用前需要确保 zlib 库已正确安装
 * @warning 不支持多线程并发写入同一文件
 */
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