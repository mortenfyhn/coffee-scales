// Host program: replays a log from the logging build through the scale logic.
// Prints the intermediate signal processing stages as CSV to stdout, and
// reports iterations where the replay differs from the device to stderr.

#include <Replay.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    const bool auto_sleep = argc == 3 && strcmp(argv[1], "--auto-sleep") == 0;
    if (argc != 2 && !auto_sleep)
    {
        fprintf(stderr, "Usage: %s [--auto-sleep] LOG\n", argv[0]);
        return 2;
    }

    FILE* file = fopen(argv[argc - 1], "r");
    if (!file)
    {
        perror(argv[argc - 1]);
        return 2;
    }
    std::vector<LogRow> log;
    const bool ok = read_log(file, log);
    fclose(file);
    if (!ok)
        return 2;

    const auto replayed = replay(log, auto_sleep);

    printf("time_ms,raw,scaled,filtered,steady,bias,compensated,state,"
           "displayed,logged_state,logged_displayed\n");
    int mismatches = 0;
    for (size_t i = 0; i < log.size(); ++i)
    {
        const auto& out = replayed[i].output;
        const auto& s = replayed[i].stages;
        printf("%u,", out.time_ms);
        if (out.has_raw)
            printf("%d,%.4f,%.4f,%d,%.4f,%.4f,", out.raw, s.scaled, s.filtered,
                   s.steady, s.bias, s.compensated);
        else
            printf(",,,,,,");
        printf("%u,", out.state);
        if (out.has_weight)
            printf("%.1f", out.weight_x10 / 10.0);
        printf(",%u,", log[i].state);
        if (log[i].has_weight)
            printf("%.1f", log[i].weight_x10 / 10.0);
        printf("\n");

        if (!outputs_match(out, log[i]))
        {
            if (mismatches < 10)
                fprintf(stderr,
                        "Mismatch at %u ms: state %u (device %u), "
                        "weight_x10 %d (device %d)\n",
                        out.time_ms, out.state, log[i].state, out.weight_x10,
                        log[i].weight_x10);
            ++mismatches;
        }
    }

    fprintf(stderr, "%zu rows, %d mismatches\n", log.size(), mismatches);
    return mismatches == 0 ? 0 : 1;
}
