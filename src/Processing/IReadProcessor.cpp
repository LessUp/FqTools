#include "IReadProcessor.h"
#include "ProcessingPipeline.h"
#include <sstream>
#include <iomanip>

namespace fq::processing {

auto ProcessingStatistics::toString() const -> std::string {
    std::ostringstream oss;
    
    oss << "处理统计信息:\n";
    oss << "  总读取数: " << total_reads << "\n";
    oss << "  通过读取数: " << passed_reads << " (" << std::fixed << std::setprecision(2) << getPassRate() * 100.0 << "%)\n";
    oss << "  过滤读取数: " << filtered_reads << " (" << std::fixed << std::setprecision(2) << getFilterRate() * 100.0 << "%)\n";
    oss << "  修改读取数: " << modified_reads << "\n";
    oss << "  错误读取数: " << error_reads << "\n";
    oss << "  处理时间: " << std::fixed << std::setprecision(2) << processing_time_ms << " ms\n";
    oss << "  处理吞吐量: " << std::fixed << std::setprecision(2) << throughput_mbps << " MB/s";
    
    return oss.str();
}

} // namespace fq::processing
