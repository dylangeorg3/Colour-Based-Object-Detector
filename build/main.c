#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "bitmap.h"
#include "cam_detect.h"

int main(int argc, char** argv) {

    // Check if arguments are valid
    if (argc < MIN_ARGC) {
        error_exit(INCORRECT_INPUT);
    }
    
    char operation_mode = argv[MODE][FIRST_INDEX];  // First character of the mode argument

    // Define calibration data array and counter at the beginning of main
    char* data[MAX_CALIBRATIONS][LEN_CALIBRATION_DATA];
    int num_calibrations = 0;

    // Switch based on the chosen mode
    switch (operation_mode) {

        case SHOW_CALIBRATION:
            if (argc != ARGC_SHOW_CALIBRATION) {
                error_exit(INCORRECT_INPUT);
            }
            load_calibration_data(argv[2], data, &num_calibrations);  
            display_calibration_file(argv[2]);                        
            break;

        case DETECT:
            if (argc != ARGC_DETECT) {
                error_exit(INCORRECT_INPUT);
            }
            detection_mode(argv[2], argv[3]);                   
            break;

        case CALIBRATE:
            if (argc != ARGC_CALIBRATION) {
                error_exit(INCORRECT_INPUT);
            }
            calibration_mode(argv[2], argv[3]);                   
            break;

        // Other errors
        default:
            error_exit(INCORRECT_INPUT);
            break;
    }

    // Free allocated memory for calibration data in `data`
    for (int i = 0; i < num_calibrations; i++) {
        for (int j = 0; j < LEN_CALIBRATION_DATA; j++) {
            free(data[i][j]);
        }
    }

    // End successfully
    return 0;
}
