#pragma once

#include "modules/common/common.h"
#include "interfaces/i_processing_pipeline.h"
#include "interfaces/i_statistic_calculator.h"

namespace fq { namespace common {
inline void print_big_logo() { print_logo(); }
inline void software_info(const char* soft_name) { (void)soft_name; print_software_info(); }
}}
