#ifndef _CAM_DETECT_H
#define _CAM_DETECT_H

#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mode selection defines
#define SHOW_CALIBRATION 's'    ///< Mode for displaying the calibration file contents
#define DETECT 'd'              ///< Mode for detecting objects in the image using the calibration file
#define CALIBRATE 'c'           ///< Mode for calibrating an object based on the image and label

// Argument index defines
#define MODE 1                  ///< Index of the mode argument in argv
#define FIRST_INDEX 0           ///< First index of array

// Error type defines
#define INCORRECT_INPUT 1       ///< Error type for incorrect input
#define FILE_NOT_FOUND 2        ///< Error type for missing file

// Argument count requirements for different modes
#define MIN_ARGC 2              ///< Minimum number of arguments required to run the program
#define ARGC_SHOW_CALIBRATION 3 ///< Required number of arguments for "show calibration" mode
#define ARGC_DETECT 4           ///< Required number of arguments for "detect" mode
#define ARGC_CALIBRATION 4      ///< Required number of arguments for "calibration" mode

// Buffer and data size defines
#define STR_BUFFER_SIZE 1024    ///< Buffer size for strings
#define INT_BUFFER_SIZE 10      ///< Buffer size for integers
#define LEN_CALIBRATION_DATA 20 ///< Number of data points for each calibration file entry

// Calibration and pixel index defines
#define WINDOW_SIZE 50          ///< Size of the calibration window (width and height)
#define ZERO_INDEX_ADJUSTMENT 1 ///< Adjustment for zero-indexed coordinates

#define SATURATION_THRESHOLD 50 ///< Minimum saturation threshold for calibration
#define VALUE_THRESHOLD 30      ///< Minimum value threshold for calibration

#define PIXEL_REGION_INDEX 3    ///< 4th index in each pixel to store region value
#define VISITED_INDEX 4         ///< 5th index in each pixel to mark if visited for BFS
#define OBJECT_TYPE_INDEX 5     ///< 6th index in each pixel to store object type
#define MAX_CALIBRATIONS 10     ///< Maximum number of calibration entries allowed
#define NAME_INDEX 0            ///< Index for name in calibration data
#define HUE_MID_INDEX 1         ///< Index for middle hue value in calibration data
#define MAX_HUE_INDEX 2         ///< Index for maximum hue in calibration data

#define MAX_REGIONS 50          ///< Maximum number of regions

#define NOT_VISITED 0           ///< Pixel status: not visited in BFS
#define VISITED 1               ///< Pixel status: visited in BFS
#define MAX_NAME_LENGTH 50      ///< Maximum length for object names

// Region structure to define the bounding box of detected objects
typedef struct {
    int min_x; ///< Minimum x-coordinate of the region
    int max_x; ///< Maximum x-coordinate of the region
    int min_y; ///< Minimum y-coordinate of the region
    int max_y; ///< Maximum y-coordinate of the region
} Region;

//--------------------------------------------------------------------------------------
// Function declarations
//--------------------------------------------------------------------------------------

/**
 @brief Exits the program with an error message based on error_type
 @param error_type Integer representing the error type
 */
void error_exit(int error_type);

/**
 @brief Removes the newline character at the end of a string
 @param line The string to be cleaned
 */
void clean_line(char* line);

/**
 @brief Loads calibration data from a file into a data array
 @param calibration_file_path Path to the calibration file
 @param data Array to store calibration data
 @param num_calibrations Pointer to the number of calibrations loaded
 */
void load_calibration_data(char* calibration_file_path, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int* num_calibrations);

/**
 @brief Displays the contents of the calibration file for verification
 @param calibration_file_path Path to the calibration file
 */
void display_calibration_file(char* calibration_file_path);

/**
 @brief Sets a specific pixel to black
 @param image The image to modify
 @param x The x-coordinate of the pixel
 @param y The y-coordinate of the pixel
 */
void set_pixel_black(Bmp image, int x, int y);

/**
 @brief Sets a specific pixel to white
 @param image The image to modify
 @param x The x-coordinate of the pixel
 @param y The y-coordinate of the pixel
 */
void set_pixel_white(Bmp image, int x, int y);

/**
 @brief Applies thresholding to the image using calibration data
 @param image_bmp The original image bitmap
 @param data Array storing calibration data
 @param num_calibrations The number of calibrations loaded
 @return A thresholded Bmp image
 */
Bmp apply_threshold_to_image(Bmp image_bmp, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int num_calibrations);

/**
 @brief Loads calibration data and creates a thresholded version of an image
 @param calibration_file_path Path to the calibration file
 @param image_bmp The original image bitmap
 @param data Array storing calibration data
 @param num_calibrations Pointer to the number of calibrations loaded
 @return A thresholded Bmp image
 */
Bmp create_threshold_image(char* calibration_file_path, Bmp image_bmp, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int* num_calibrations);

/**
 @brief Prints and saves an image with boxes around detected regions
 @param image_with_regions The image on which boxes will be drawn
 @param threshold_image The threshold image used for detection
 @param data Array storing calibration data
 @param num_calibrations Number of calibrations loaded
 @param region_array Array of detected regions
 @param region_number Number of detected regions
 */
void print_image_with_boxes(Bmp image_with_regions, Bmp threshold_image, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int num_calibrations, Region region_array[], int region_number);

/**
 @brief Performs a recursive depth-first search on a pixel to find connected regions
 @param threshold_image The threshold image
 @param x The x-coordinate of the starting pixel
 @param y The y-coordinate of the starting pixel
 @param region_array Array storing region data
 @param region_number Pointer to the current region number
 */
void depth_first_search(Bmp* threshold_image, int x, int y, Region region_array[], int* region_number);

/**
 @brief Finds connected regions in a thresholded image using depth first search.
 @param threshold_image The thresholded image
 @param image_bmp The original image bitmap
 @param data Array storing calibration data
 @param num_calibrations Number of calibrations loaded
 */
void find_connected_regions(Bmp threshold_image, Bmp image_bmp, char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA], int num_calibrations);

/**
 @brief Detects objects in an image using calibration data and outputs detected objects
 @param calibration_file_path Path to the calibration file
 @param image_file_path Path to the image file to detect objects
 */
void detection_mode(char* calibration_file_path, char* image_file_path);

/**
 @brief Calibrates an object based on an image and writes calibration data to a file
 @param calibration_file_name Path to the calibration file
 @param image_file_path Path to the image file to use for calibration
 */
void calibration_mode(char* calibration_file_name, char* image_file_path);

#endif
