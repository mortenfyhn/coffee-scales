#include <Replay.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace
{

class ReplayHardware : public Hardware
{
  public:
    // Sets the inputs for the next iteration and clears the outputs.
    void next(const LogRow& input)
    {
        input_ = input;
        output_ = LogRow{};
        output_.time_ms = input.time_ms;
    }

    const LogRow& output() const { return output_; }

    uint32_t millis() override { return input_.time_ms; }

    int32_t read_load_cell() override
    {
        output_.has_raw = true;
        output_.raw = input_.raw;
        return input_.raw;
    }

    bool tare_pressed() override
    {
        output_.pressed = input_.pressed;
        return input_.pressed;
    }

    bool long_pressed() override
    {
        output_.long_pressed = input_.long_pressed;
        return input_.long_pressed;
    }

    void show_weight(float grams) override
    {
        output_.has_weight = true;
        output_.weight_x10 = lroundf(grams * 10);
    }

    void show_line() override {}
    void set_brightness_max() override {}
    void set_brightness_dim() override {}
    void start_timer() override {}
    void stop_timer() override {}
    void update_timer() override {}
    void sleep() override {}

  private:
    LogRow input_;
    LogRow output_;
};

// Parses an optional integer field and advances past the following comma.
// Returns false if the field is not a number.
bool parse_field(const char*& pos, bool& present, long& value)
{
    char* end = nullptr;
    value = strtol(pos, &end, 10);
    present = end != pos;
    if (*end != ',' && *end != '\0')
        return false;
    pos = *end == ',' ? end + 1 : end;
    return true;
}

bool parse_row(const char* line, LogRow& row)
{
    constexpr int num_fields = 8;
    bool present[num_fields];
    long value[num_fields];
    const char* pos = line;
    for (int i = 0; i < num_fields; ++i)
    {
        if (!parse_field(pos, present[i], value[i]))
            return false;
    }
    if (*pos != '\0' || !present[0] || !present[3] || !present[4] ||
        !present[5] || present[1] != present[2] ||
        present[7] != (value[3] != 0))
        return false;

    row.time_ms = value[0];
    row.has_raw = present[1];
    row.raw = value[1];
    row.wait_us = value[2];
    row.pressed = value[3];
    row.long_pressed = value[4];
    row.state = value[5];
    row.has_weight = present[6];
    row.weight_x10 = value[6];
    row.press_ms = value[7];
    return true;
}

}  // namespace

bool read_log(FILE* file, std::vector<LogRow>& rows)
{
    char line[128];
    bool found_header = false;
    int line_number = 0;
    while (fgets(line, sizeof(line), file))
    {
        ++line_number;
        line[strcspn(line, "\r\n")] = '\0';
        if (!found_header)
        {
            found_header = strncmp(line, log_header, strlen(log_header)) == 0;
            continue;
        }
        if (strncmp(line, log_header, strlen(log_header)) == 0)
        {
            fprintf(stderr, "Line %d: The board restarted\n", line_number);
            return false;
        }
        LogRow row;
        if (!parse_row(line, row))
        {
            if (feof(file))
                break;  // Capture stopped in the middle of a line
            fprintf(stderr, "Line %d: Invalid row: %s\n", line_number, line);
            return false;
        }
        rows.push_back(row);
    }
    if (!found_header)
    {
        fprintf(stderr, "No header found\n");
        return false;
    }
    return true;
}

std::vector<ReplayRow> replay(const std::vector<LogRow>& log, bool auto_sleep)
{
    std::vector<ReplayRow> result;
    if (log.empty())
        return result;

    auto hardware = ReplayHardware{};
    auto scale = Scale{hardware, auto_sleep};
    hardware.next(log.front());
    scale.start();

    for (const auto& input : log)
    {
        hardware.next(input);
        scale.step();
        auto output = hardware.output();
        output.state = static_cast<uint8_t>(scale.state());
        result.push_back({output, scale.stages()});
    }
    return result;
}

bool outputs_match(const LogRow& replayed, const LogRow& logged)
{
    return replayed.has_raw == logged.has_raw &&
           replayed.state == logged.state &&
           replayed.has_weight == logged.has_weight &&
           (!replayed.has_weight || replayed.weight_x10 == logged.weight_x10);
}
