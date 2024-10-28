#include "bitmap.h"
#include "cam_detect.h"

// Display type of error and exit
void error_exit(int error_type) {
    if (error_type == INCORRECT_INPUT) {
        printf("Incorrect input\n");
    } else if (error_type == FILE_NOT_FOUND) {
        printf("Could not open calibration file\n");
    }
    exit(error_type);
}

// Clean line to get rid of newline character at the end
void clean_line(char* line) {
    int len = strlen(line);
    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

// Display text file information
void load_calibration_data(char* calibration_file_path, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int* num_calibrations) {
    char line[STR_BUFFER_SIZE];
    FILE* file_pointer = fopen(calibration_file_path, "r");

    if (file_pointer == NULL) {
        error_exit(FILE_NOT_FOUND);
    }

    while (fgets(line, sizeof(line), file_pointer) != NULL) {
        clean_line(line);
        char* token = strtok(line, " ");
        int i = 0;
        while (token != NULL && i < LEN_CALIBRATION_DATA) {
            data[*num_calibrations][i] = (char*)malloc(strlen(token) + 1);
            strcpy(data[*num_calibrations][i], token);
            i++;
            token = strtok(NULL, " ");
        }
        (*num_calibrations)++;
    }
    fclose(file_pointer);
}

// Print out data from given calibration file
void display_calibration_file(char* calibration_file_path) {
    FILE* file_pointer = fopen(calibration_file_path, "r");
    if (file_pointer == NULL) {
        error_exit(FILE_NOT_FOUND);
    }

    char line[STR_BUFFER_SIZE];
    printf("Calibrated Objects:\n");
    while (fgets(line, sizeof(line), file_pointer) != NULL) {
        clean_line(line);
        
        // Tokenize the line into the calibration data
        char* tokens[LEN_CALIBRATION_DATA];
        char* token = strtok(line, " ");
        int i = 0;
        while (token != NULL && i < LEN_CALIBRATION_DATA) {
            tokens[i++] = token;
            token = strtok(NULL, " ");
        }

        // Check that we have enough tokens
        if (i >= 5) {
            // Extract fields for formatted output
            char* name = tokens[0];
            char* hue = tokens[1];
            char* max_diff = tokens[2];
            char* min_saturation = tokens[3];
            char* min_value = tokens[4];

            // Print formatted output
            printf("%s: Hue: %s (Max. Diff: %s), Min. SV: %s %s\n", name, hue, max_diff, min_saturation, min_value);
        } else {
            printf("Too many arguments in calibration file\n");
        }
    }

    fclose(file_pointer);
}

// Creates threshold mask for image with given calibration data
Bmp apply_threshold_to_image(Bmp image_bmp, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int num_calibrations) {
    int height = image_bmp.height;
    int width = image_bmp.width;
    unsigned char*** pixels = image_bmp.pixels;
    Bmp threshold_image = copy_bmp(image_bmp);

    for (int calibration = 0; calibration < num_calibrations; calibration++) {
        int hue_mid = atoi(data[calibration][HUE_MID_INDEX]);
        int max_hue_diff = atoi(data[calibration][MAX_HUE_INDEX]);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                HSV hsv_values = rgb2hsv(pixels[y][x]);
                int hue = hsv_values.hue;
                int saturation = hsv_values.saturation;
                int value = hsv_values.value;
                int hue_diff = hue_difference(hue_mid, hue);
                int visited = pixels[y][x][VISITED_INDEX];

                if ((saturation >= SATURATION_THRESHOLD) && (value >= VALUE_THRESHOLD) && (hue_diff <= max_hue_diff) && (visited == NOT_VISITED)) {
                    set_pixel_white(threshold_image, x, y);
                    pixels[y][x][VISITED_INDEX] = VISITED;
                    threshold_image.pixels[y][x][OBJECT_TYPE_INDEX] = calibration;
                } else if (visited == NOT_VISITED) {
                    set_pixel_black(threshold_image, x, y);
                }
            }
        }
    }
    return threshold_image;
}

// Loads calibration data and creates threshold image
Bmp create_threshold_image(char* calibration_file_path, Bmp image_bmp, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int* num_calibrations) {
    
    load_calibration_data(calibration_file_path, data, num_calibrations);

    Bmp threshold_image = apply_threshold_to_image(image_bmp, data, *num_calibrations);

    return threshold_image;
}

// Change pixels to a certain colour
void set_pixel_black(Bmp image, int x, int y) {
    image.pixels[y][x][RED] = black.red_value;
    image.pixels[y][x][GREEN] = black.green_value;
    image.pixels[y][x][BLUE] = black.blue_value;
}

void set_pixel_white(Bmp image, int x, int y) {
    image.pixels[y][x][RED] = white.red_value;
    image.pixels[y][x][GREEN] = white.green_value;
    image.pixels[y][x][BLUE] = white.blue_value;
}


// Print data for detection boxes and save to image
void print_image_with_boxes(Bmp image_with_regions, Bmp threshold_image, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int num_calibrations, Region region_array[], int region_number) {
    for (int object_num = 0; object_num < num_calibrations; object_num++) {
        for (int i = 1; i < region_number; i++) {
            int x_coord = region_array[i].min_x;
            int y_coord = region_array[i].min_y;
            int box_width = region_array[i].max_x - region_array[i].min_x + 1;
            int box_height = region_array[i].max_y - region_array[i].min_y + 1;

            int num = 0, sum = 0;
            int q2_x = x_coord + box_width / 4;
            int q3_x = x_coord + 3 * box_width / 4;
            int q2_y = y_coord + box_height / 4;
            int q3_y = y_coord + 3 * box_height / 4;
            for (int x = q2_x; x < q3_x; x++) {
                for (int y = q2_y; y < q3_y; y++) {
                    sum += threshold_image.pixels[y][x][OBJECT_TYPE_INDEX];
                    if (threshold_image.pixels[y][x][OBJECT_TYPE_INDEX] > 0) {
                        num++;
                    }
                }
            }
            int average = (num != 0) ? sum / num : 0;

            if (average == object_num) {
                char* name = data[object_num][NAME_INDEX];
                if (box_width >= 20 && box_height >= 20) {
                    draw_box(image_with_regions, x_coord, y_coord, box_width, box_height);
                    printf("Detected %s: %d %d %d %d\n", name, x_coord, y_coord, box_width, box_height);
                }
            }
        }
    }
    write_bmp(image_with_regions, "output_images/image_with_regions.bmp");
}

// Function to check neighbours of a given pixel
void depth_first_search(Bmp* threshold_image, int x, int y, Region region_array[], int* region_number) {
    threshold_image->pixels[y][x][PIXEL_REGION_INDEX] = *region_number;
    threshold_image->pixels[y][x][VISITED_INDEX] = VISITED;

    int max_width = threshold_image->width;
    int max_height = threshold_image->height;

    if (x < region_array[*region_number].min_x) {
        region_array[*region_number].min_x = x;
    }
    if (x > region_array[*region_number].max_x) {
        region_array[*region_number].max_x = x;
    }
    if (y < region_array[*region_number].min_y) {
        region_array[*region_number].min_y = y;
    }
    if (y > region_array[*region_number].max_y) {
        region_array[*region_number].max_y = y;
    }

    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; i++) {
        int adjacent_x = x + dx[i];
        int adjacent_y = y + dy[i];

        if (adjacent_x > 0 && adjacent_x < max_width && adjacent_y > 0 && adjacent_y < max_height &&
            threshold_image->pixels[adjacent_y][adjacent_x][RED] == white.red_value &&
            threshold_image->pixels[adjacent_y][adjacent_x][VISITED_INDEX] == 0) {
            depth_first_search(threshold_image, adjacent_x, adjacent_y, region_array, region_number);
        }
    }
}

// Finds connected regions by iterating through grid and performing DFS
void find_connected_regions(Bmp threshold_image, Bmp image_bmp, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int num_calibrations) {
    Bmp image_with_regions = copy_bmp(image_bmp);
    int height = threshold_image.height;
    int width = threshold_image.width;
    unsigned char*** pixels = threshold_image.pixels;

    Region region_array[MAX_REGIONS];
    for (int i = 0; i < MAX_REGIONS; i++) {
        region_array[i].min_x = width;
        region_array[i].max_x = 0;
        region_array[i].min_y = height;
        region_array[i].max_y = 0;
    }

    int region_number = 1;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (pixels[y][x][RED] == white.red_value && pixels[y][x][VISITED_INDEX] == 0) {
                depth_first_search(&threshold_image, x, y, region_array, &region_number);
                region_number++;
            }
            threshold_image.pixels[y][x][VISITED_INDEX] = VISITED;
        }
    }
    print_image_with_boxes(image_with_regions, threshold_image, data, num_calibrations, region_array, region_number);
    free_bmp(image_with_regions);
}

// Detects objects in image based on calibration file and outputs relevant masks
void detection_mode(char* calibration_file_path, char* image_file_path) {
    char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA];
    int num_calibrations = 0;

    Bmp image_bmp = read_bmp(image_file_path);
    Bmp threshold_image = create_threshold_image(calibration_file_path, image_bmp, data, &num_calibrations);

    write_bmp(threshold_image, "output_images/threshold_output.bmp");

    find_connected_regions(threshold_image, image_bmp, data, num_calibrations);

    free_bmp(threshold_image);
    free_bmp(image_bmp);
}

// Takes in an image and produces calibration textfile with calibration data
void calibration_mode(char* calibration_file_name, char* image_file_path) {
    Bmp image_bmp = read_bmp(image_file_path);
    int height = image_bmp.height;
    int width = image_bmp.width;
    unsigned char*** pixels = image_bmp.pixels;

    int max_y = height - ((height - WINDOW_SIZE) / 2) - ZERO_INDEX_ADJUSTMENT;
    int min_y = (height - WINDOW_SIZE) / 2 - ZERO_INDEX_ADJUSTMENT;
    int max_x = width - ((width - WINDOW_SIZE) / 2) - ZERO_INDEX_ADJUSTMENT;
    int min_x = (width - WINDOW_SIZE) / 2 - ZERO_INDEX_ADJUSTMENT;

    int min_hue = 360, max_hue = 0;

    for (int y = min_y; y < max_y; y++) {
        for (int x = min_x; x < max_x; x++) {
            HSV hsv_values = rgb2hsv(pixels[y][x]);
            int hue = hsv_values.hue;
            int saturation = hsv_values.saturation;
            int value = hsv_values.value;

            if ((saturation >= SATURATION_THRESHOLD) && (value >= VALUE_THRESHOLD)) {
                if (hue > max_hue) {
                    max_hue = hue;
                } else if (hue < min_hue) {
                    min_hue = hue;
                }
            }
        }
    }
    int hue_diff = hue_difference(max_hue, min_hue);
    int hue_mid = hue_midpoint(max_hue, min_hue);
    int max_hue_diff = hue_diff / 2;

    printf("%s %d %d %d %d\n", calibration_file_name, hue_mid, max_hue_diff, SATURATION_THRESHOLD, VALUE_THRESHOLD);

    free_bmp(image_bmp);
}
