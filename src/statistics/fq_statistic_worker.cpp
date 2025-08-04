#include "statistics/FqStatisticWorker.h"
#include "statistics/fq_statistic.h"
#include "core_legacy/core.h"

namespace fq::statistic {

FqStatisticWorker::FqStatisticWorker(std::shared_ptr<fq::fastq::FastQInfer> fq_infer)
    : m_fq_infer(std::move(fq_infer)) {
    if (m_fq_infer) {
        m_qual_offset = m_fq_infer->getFqFileAttribution().q_score_type == fq::fastq::QScoreType::Sanger ? 33 : 64;
    }
}

auto FqStatisticWorker::stat(const Batch& batch) -> IStatistic::Result {
    FqStatisticResult result;
    if (batch.reads.empty()) {
        return result;
    }
    result.read_length = batch.reads[0].base.length();
    result.n_pos_qual.resize(result.read_length, std::vector<uint64_t>(fq::fastq::MAX_QUAL, 0));
    result.n_pos_base.resize(result.read_length, std::vector<uint64_t>(MAX_BASE_NUM, 0));

    for (const auto& read : batch.reads) {
        result.n_read++;
        for (size_t i = 0; i < result.read_length; ++i) {
            result.n_pos_qual[i][read.qual[i] - m_qual_offset]++;
            switch (read.base[i]) {
            case 'A':
                result.n_pos_base[i][0]++;
                break;
            case 'C':
                result.n_pos_base[i][1]++;
                break;
            case 'G':
                result.n_pos_base[i][2]++;
                break;
            case 'T':
                result.n_pos_base[i][3]++;
                break;
            case 'N':
            default:
                result.n_pos_base[i][4]++;
                break;
            }
        }
    }

    return result;
}

} // namespace fq::statistic
