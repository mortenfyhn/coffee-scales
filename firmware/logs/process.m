#!/usr/bin/env octave-cli

## Config (should match config.h)
division_factor = 1540.32;
filter_size = 10;
hysteresis_size = 0.1;
standard_deviation_threshold = 0.1;  # This one is in SmoothingFilter.h

## Load data
if nargin < 1
    error("Missing argument INPUT_FILE")
end
input_file = argv(){1};
input_data = csvread(input_file);
time = input_data(2:end, 1);
raw_data = input_data(2:end, 2);

## Scale and offset
offset = raw_data(1);
after_scaling = (raw_data - offset) / division_factor;

## Running average filter
in = after_scaling;
out = zeros(size(in));
is_steady = false(size(in));  # Save for use in bias compensation
filtered = filter(ones(filter_size, 1) / filter_size, 1, in);
for i = 1:numel(in)
    stddev = std(filtered(max(1, i - filter_size):i));
    if stddev < standard_deviation_threshold
        out(i) = filtered(i);
        is_steady(i) = true;
    else
        out(i) = in(i);
        is_steady(i) = false;
    end
end
after_smoothing = out;

## Bias compensation
df = [0; diff(filtered(:))];
bias = cumsum(df .* (is_steady(:) ~= 0));
after_bias_comp = filtered(:) - bias;

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

previous_output = 0;
in = after_bias_comp;
out = zeros(size(in));
for i = 1:numel(in)
    out(i) = hysteresis(in(i), hysteresis_size, previous_output);
    previous_output = out(i);
end
after_hysteresis = out;

## Output result
printf("time,raw_data,1_after_scaling,2_after_smoothing,3_after_bias,4_after_hysteresis,is_steady\n");
output_matrix = [ time(:), raw_data(:), after_scaling(:), after_smoothing(:), after_bias_comp(:), after_hysteresis(:), is_steady(:) ];
fprintf([repmat("%g,",1,columns(output_matrix)-1) "%g\n"], output_matrix.');
