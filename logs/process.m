## Load data
input_filename = "brew.csv";
% input_filename = "steadystate.csv";
input_file = fullfile("raw", input_filename);
input_data = csvread(input_file);
time = input_data(2:end, 1);
raw_data = input_data(2:end, 2);

## Scale and offset
scale = 1874;
offset = raw_data(1);
after_scaling = (raw_data - offset) / scale;

## Running average filter
filter_size = 10;
standard_deviation_threshold = 0.1;
in = after_scaling;
out = zeros(size(in));
filtered = filter(ones(filter_size, 1) / filter_size, 1, in);
for i = 1:numel(in)
    stddev = std(filtered(max(1, i - filter_size):i));
    if stddev < standard_deviation_threshold
        out(i) = filtered(i);
    else
        out(i) = in(i);
    end
end
after_smoothing = out;

## Hysteresis
function output = hysteresis(input, threshold, previous_output)
    if input > previous_output + threshold
        output = input;
    elseif input < previous_output - threshold
        output = input;
    else
        output = previous_output;
    end
end

hysteresis_size = 0.1;
previous_output = 0;
in = after_smoothing;
out = zeros(size(in));
for i = 1:numel(in)
    out(i) = hysteresis(in(i), hysteresis_size, previous_output);
    previous_output = out(i);
end
after_hysteresis = out;

## Write result to file
mkdir("processed");
output_file = fullfile("processed", input_filename);
file = fopen(output_file, "w");
fprintf(file, "time,raw_data,1_after_scaling,2_after_smoothing,3_after_hysteresis\n");
for i = 1:numel(time)
    fprintf(
        file,
        "%g,%g,%g,%g,%g\n",
        time(i),
        raw_data(i),
        after_scaling(i),
        after_smoothing(i),
        after_hysteresis(i));
end
fclose(file);

