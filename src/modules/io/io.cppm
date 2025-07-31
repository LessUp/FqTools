export module fq.io;

import std;
import fq.core;
import fq.error;

export namespace fq::io {
    // 缓冲区接口
    class Buffer : public fq::core::MemoryTrackable {
    public:
        virtual ~Buffer() = default;
        
        [[nodiscard]] virtual auto data() -> std::byte* = 0;
        [[nodiscard]] virtual auto data() const -> const std::byte* = 0;
        [[nodiscard]] virtual auto size() const noexcept -> std::size_t = 0;
        [[nodiscard]] virtual auto capacity() const noexcept -> std::size_t = 0;
        
        virtual void resize(std::size_t new_size) = 0;
        virtual void clear() noexcept = 0;
        
        // 字符串视图支持
        [[nodiscard]] auto as_string_view() const -> std::string_view {
            return std::string_view(reinterpret_cast<const char*>(data()), size());
        }
        
        // 字节跨度支持
        [[nodiscard]] auto as_span() -> std::span<std::byte> {
            return std::span<std::byte>(data(), size());
        }
        
        [[nodiscard]] auto as_span() const -> std::span<const std::byte> {
            return std::span<const std::byte>(data(), size());
        }
    };

    // 共享缓冲区实现（零拷贝支持）
    class SharedBuffer : public Buffer {
    private:
        struct Impl {
            std::unique_ptr<std::byte[]> data;
            std::size_t capacity;
            std::size_t size;
            std::atomic<std::size_t> ref_count{1};
            
            Impl(std::size_t cap) : capacity(cap), size(0) {
                if (cap > 0) {
                    data = std::make_unique<std::byte[]>(cap);
                }
            }
        };
        
    public:
        explicit SharedBuffer(std::size_t capacity) 
            : m_impl(std::make_shared<Impl>(capacity)) 
        {}
        
        SharedBuffer(const SharedBuffer&) = default;
        SharedBuffer(SharedBuffer&&) = default;
        SharedBuffer& operator=(const SharedBuffer&) = default;
        SharedBuffer& operator=(SharedBuffer&&) = default;
        
        [[nodiscard]] auto data() -> std::byte* override {
            return m_impl->data.get();
        }
        
        [[nodiscard]] auto data() const -> const std::byte* override {
            return m_impl->data.get();
        }
        
        [[nodiscard]] auto size() const noexcept -> std::size_t override {
            return m_impl->size;
        }
        
        [[nodiscard]] auto capacity() const noexcept -> std::size_t override {
            return m_impl->capacity;
        }
        
        void resize(std::size_t new_size) override {
            if (new_size > m_impl->capacity) {
                FQ_THROW_RESOURCE_ERROR("buffer", "resize beyond capacity");
            }
            m_impl->size = new_size;
        }
        
        void clear() noexcept override {
            m_impl->size = 0;
        }
        
        [[nodiscard]] auto view(std::size_t offset, std::size_t length) const -> std::span<const std::byte> {
            if (offset + length > m_impl->size) {
                FQ_THROW_VALIDATION_ERROR("buffer_view", "offset+length exceeds buffer size");
            }
            return std::span<const std::byte>(m_impl->data.get() + offset, length);
        }
        
        [[nodiscard]] auto ref_count() const noexcept -> std::size_t {
            return m_impl.use_count();
        }
        
        [[nodiscard]] auto memory_usage() const noexcept -> std::size_t override {
            return m_impl->capacity;
        }
        
        // 写入数据
        void write_data(const void* source, std::size_t count, std::size_t offset = 0) {
            if (offset + count > m_impl->capacity) {
                FQ_THROW_RESOURCE_ERROR("buffer", "write beyond capacity");
            }
            
            std::memcpy(m_impl->data.get() + offset, source, count);
            m_impl->size = std::max(m_impl->size, offset + count);
        }
        
        void write_string(std::string_view str, std::size_t offset = 0) {
            write_data(str.data(), str.size(), offset);
        }
        
    private:
        std::shared_ptr<Impl> m_impl;
    };

    // 异步I/O接口
    template<typename T>
    class AsyncReader {
    public:
        using ReadResult = std::expected<T, fq::error::FastQException>;
        using ReadFuture = std::future<ReadResult>;
        
        virtual ~AsyncReader() = default;
        
        [[nodiscard]] virtual auto read_async() -> ReadFuture = 0;
        [[nodiscard]] virtual auto is_eof() const noexcept -> bool = 0;
        virtual void close() = 0;
    };

    template<typename T>
    class AsyncWriter {
    public:
        using WriteResult = std::expected<void, fq::error::FastQException>;
        using WriteFuture = std::future<WriteResult>;
        
        virtual ~AsyncWriter() = default;
        
        [[nodiscard]] virtual auto write_async(const T& data) -> WriteFuture = 0;
        virtual void flush() = 0;
        virtual void close() = 0;
    };
    
    // 文件工具类
    class FileUtils {
    public:
        // 检测文件类型
        enum class FileType {
            Unknown,
            Plain,
            Gzip,
            Bzip2,
            Xz
        };
        
        static auto detect_file_type(const std::filesystem::path& path) -> FileType {
            if (!std::filesystem::exists(path)) {
                return FileType::Unknown;
            }
            
            std::ifstream file(path, std::ios::binary);
            if (!file) {
                return FileType::Unknown;
            }
            
            std::array<unsigned char, 4> magic{};
            file.read(reinterpret_cast<char*>(magic.data()), magic.size());
            
            // Gzip: 1f 8b
            if (magic[0] == 0x1f && magic[1] == 0x8b) {
                return FileType::Gzip;
            }
            
            // Bzip2: 42 5a
            if (magic[0] == 0x42 && magic[1] == 0x5a) {
                return FileType::Bzip2;
            }
            
            // XZ: fd 37 7a 58
            if (magic[0] == 0xfd && magic[1] == 0x37 && magic[2] == 0x7a && magic[3] == 0x58) {
                return FileType::Xz;
            }
            
            return FileType::Plain;
        }
        
        // 获取文件大小
        static auto get_file_size(const std::filesystem::path& path) -> std::size_t {
            std::error_code ec;
            auto size = std::filesystem::file_size(path, ec);
            if (ec) {
                FQ_THROW_IO_ERROR(path.string(), ec.value());
            }
            return size;
        }
        
        // 创建临时文件
        static auto create_temp_file(const std::string& prefix = "fastqtools") -> std::filesystem::path {
            auto temp_dir = std::filesystem::temp_directory_path();
            auto temp_file = temp_dir / (prefix + "_" + std::to_string(std::random_device{}()));
            
            // 确保文件不存在
            while (std::filesystem::exists(temp_file)) {
                temp_file = temp_dir / (prefix + "_" + std::to_string(std::random_device{}()));
            }
            
            return temp_file;
        }
        
        // 安全删除文件
        static auto safe_remove(const std::filesystem::path& path) -> bool {
            std::error_code ec;
            return std::filesystem::remove(path, ec);
        }
        
        // 确保目录存在
        static void ensure_directory_exists(const std::filesystem::path& dir) {
            if (!std::filesystem::exists(dir)) {
                std::error_code ec;
                if (!std::filesystem::create_directories(dir, ec)) {
                    FQ_THROW_IO_ERROR(dir.string(), ec.value());
                }
            }
        }
    };
    
    // 缓冲流实现
    template<typename StreamType>
    class BufferedStream {
    public:
        explicit BufferedStream(std::unique_ptr<StreamType> stream, std::size_t buffer_size = 64 * 1024)
            : m_stream(std::move(stream))
            , m_buffer(std::make_unique<char[]>(buffer_size))
            , m_buffer_size(buffer_size)
            , m_buffer_pos(0)
            , m_buffer_end(0)
        {
            if constexpr (std::is_base_of_v<std::istream, StreamType>) {
                m_stream->rdbuf()->pubsetbuf(m_buffer.get(), static_cast<std::streamsize>(buffer_size));
            } else if constexpr (std::is_base_of_v<std::ostream, StreamType>) {
                m_stream->rdbuf()->pubsetbuf(m_buffer.get(), static_cast<std::streamsize>(buffer_size));
            }
        }
        
        [[nodiscard]] auto get_stream() -> StreamType& {
            return *m_stream;
        }
        
        [[nodiscard]] auto get_stream() const -> const StreamType& {
            return *m_stream;
        }
        
        void flush() {
            if constexpr (std::is_base_of_v<std::ostream, StreamType>) {
                m_stream->flush();
            }
        }
        
        [[nodiscard]] auto is_open() const -> bool {
            if constexpr (requires { m_stream->is_open(); }) {
                return m_stream->is_open();
            }
            return static_cast<bool>(*m_stream);
        }
        
        void close() {
            if constexpr (requires { m_stream->close(); }) {
                m_stream->close();
            }
        }
        
        [[nodiscard]] auto bytes_processed() const noexcept -> std::size_t {
            return m_bytes_processed;
        }
        
    private:
        std::unique_ptr<StreamType> m_stream;
        std::unique_ptr<char[]> m_buffer;
        std::size_t m_buffer_size;
        std::size_t m_buffer_pos;
        std::size_t m_buffer_end;
        std::size_t m_bytes_processed = 0;
    };
    
    // 便利类型别名
    using BufferedIfstream = BufferedStream<std::ifstream>;
    using BufferedOfstream = BufferedStream<std::ofstream>;
    
    // I/O性能监控
    class IOMetrics {
    public:
        struct Stats {
            std::size_t bytes_read = 0;
            std::size_t bytes_written = 0;
            std::size_t read_operations = 0;
            std::size_t write_operations = 0;
            std::chrono::nanoseconds read_time{0};
            std::chrono::nanoseconds write_time{0};
            
            [[nodiscard]] auto read_throughput_mbps() const -> double {
                if (read_time.count() == 0) return 0.0;
                return static_cast<double>(bytes_read) / (1024.0 * 1024.0) / 
                       (static_cast<double>(read_time.count()) / 1e9);
            }
            
            [[nodiscard]] auto write_throughput_mbps() const -> double {
                if (write_time.count() == 0) return 0.0;
                return static_cast<double>(bytes_written) / (1024.0 * 1024.0) / 
                       (static_cast<double>(write_time.count()) / 1e9);
            }
        };
        
        void record_read(std::size_t bytes, std::chrono::nanoseconds duration) {
            m_stats.bytes_read += bytes;
            m_stats.read_operations++;
            m_stats.read_time += duration;
        }
        
        void record_write(std::size_t bytes, std::chrono::nanoseconds duration) {
            m_stats.bytes_written += bytes;
            m_stats.write_operations++;
            m_stats.write_time += duration;
        }
        
        [[nodiscard]] auto get_stats() const -> const Stats& {
            return m_stats;
        }
        
        void reset() {
            m_stats = Stats{};
        }
        
    private:
        Stats m_stats;
    };
}