## Load data
input_file = "raw/brew.csv";
input_data = csvread(input_file);
time = input_data(2:end, 1);
raw_data = input_data(2:end, 2);

## Scale and offset
scale = 1874;
offset = raw_data(1);
after_scaling = (raw_data - offset) / scale;

## Running average filter
filter_size = 10;
after_smoothing = filter(ones(filter_size, 1) / filter_size, 1, after_scaling);

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
input_data = after_smoothing;
output_data = zeros(size(input_data));
for i = 1:numel(input_data)
    output_data(i) = hysteresis(input_data(i), hysteresis_size, previous_output);
    previous_output = output_data(i);
end
after_hysteresis = output_data;

## Write result to file
mkdir("processed");
file = fopen("processed/brew.csv", "w");
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
