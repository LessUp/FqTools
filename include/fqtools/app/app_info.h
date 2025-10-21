#pragma once

#include "core_legacy/core.h"
#include "fqtools/pipeline/processing_pipeline_interface.h"
#include "fqtools/statistics/statistic_calculator_interface.h"

namespace fq { namespace common {
inline void software_info(const char* soft_name) { (void)soft_name; ::print_software_info(); }
}}
