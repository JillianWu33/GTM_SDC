#include "GTM_Decoder_Function.h"
#include "GTM_Decoder_Parse_TMTC_Data.h"



int parse_tmtc_data(int input_file_pointer) {
    unsigned char *tmtc_data_buffer;
    size_t actual_input_binary_buffer_size;
    int output_file_pointer;
    int tmtc_data_buffer_counter;
    size_t location;

    // dynamic memory allocation for each 128 bytes TMTC data
    tmtc_data_buffer = (unsigned char *)malloc(TMTC_DATA_SIZE); // (unsigned char *) is not necessary!
    if (!tmtc_data_buffer) { // !tmtc_data_buffer == (tmtc_data_buffer == 0) because malloc may return null (=0 in C) when exhausting address space
        log_error("fail to create tmtc data buffer");
    }

    // move file pointer inside input_binary_file base on input_file_pointer
    fseek(input_binary_file, input_file_pointer, SEEK_SET);

    // record how many bytes in input_binary_buffer
    actual_input_binary_buffer_size = fread(input_binary_buffer, 1, max_input_binary_buffer_size, input_binary_file);

    // update file pointer by input_file_pointer and actual_input_binary_buffer_size
    output_file_pointer = input_file_pointer + (int)actual_input_binary_buffer_size;

    while (1) {
        log_message("load new chunk");

        // initializing the position of tmtc_data_buffer
        tmtc_data_buffer_counter = 0;

        // parsing data in input_binary_buffer
        for (location=0; location < actual_input_binary_buffer_size; location++) {
            memcpy(tmtc_data_buffer+tmtc_data_buffer_counter, input_binary_buffer+location, 1);
            tmtc_data_buffer_counter++;

            // checking TMTC header
            if (tmtc_data_buffer_counter == 2) {
                // if anything is wrong, shifting 1 byte to re-surching TMTC header
                if (!is_tmtc_header(tmtc_data_buffer)) { 
                    tmtc_data_buffer[0] = tmtc_data_buffer[1];
                    tmtc_data_buffer_counter = 1;
                }
            }

            // TMTC data (all 128 bytes data are loaded into tmtc_data_buffer)
            if (tmtc_data_buffer_counter == TMTC_DATA_SIZE) {
                // checking TMTC tail
                // if anything is wrong, giving up this 128 bytes
                if (!is_tmtc_tail(tmtc_data_buffer + TMTC_DATA_SIZE - 2)) {
                    log_message("tmtc tail missing!!");
                }
                // if all data is healthy, parsing data base on ICD
                else { 
                    parse_tmtc_packet(tmtc_data_buffer);
                }
                tmtc_data_buffer_counter = 0;
            }
        }
        break;
    }

    // releaseing dynamic memory allocation for each 128 bytes TMTC data
    free(tmtc_data_buffer);

    // returning new file pointer to main funciton
    return output_file_pointer;
}