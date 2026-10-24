#pragma once

#include <Scale.h>
#include <log_row.h>
#include <stdio.h>
#include <vector>

// Reads a log from the logging build. Skips lines before the header and an
// incomplete last line. Returns false and prints to stderr on other errors.
bool read_log(FILE* file, std::vector<LogRow>& rows);

struct ReplayRow
{
    LogRow output;  // Same fields as the log, computed by the replay
    Stages stages;  // Valid if output.has_raw
};

// Runs the scale logic with the inputs from a log. Returns one row per log row.
std::vector<ReplayRow> replay(const std::vector<LogRow>& log, bool auto_sleep);

// True if the replay and the log agree on the outputs of an iteration.
bool outputs_match(const LogRow& replayed, const LogRow& logged);
