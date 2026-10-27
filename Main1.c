#include <stdio.h>
#include <stdlib.h>

main (int argc, char** argv) {

    FILE *addresses = fopen(argv[1], "r");
    FILE *backing_store = fopen("BACKING_STORE.bin", "rb");
    FILE *out1 = fopen("out1.txt", "w");
    FILE *out2 = fopen("out2.txt", "w");
    FILE *out3 = fopen("out3.txt", "w");

    int page_table[256];
    int tlb_page[16];
    int tlb_frame[16];
    signed char physical_memory[256][256];

    for (int i = 0; i < 256; i++) {
        page_table[i] = -1;
    }
    for (int i = 0; i < 16; i++) {
        tlb_page[i] = -1;
        tlb_frame[i] = -1;
    }

    int tlb_idx = 0;             
    int next_frame = 0;          
    int page_faults = 0;
    int tlb_hits = 0;
    int total_addresses = 0;
    
    int logical_address;

    while(fscanf(addresses, "%d", &logical_address) == 1) {
        ++total_addresses;

        int page_number = (logical_address >> 8) & 0xFF;

        int offset = logical_address & 0xFF;

        int frame_number = -1;

        //check tlb
        for (int i = 0; i < 16; i++) {
            if (tlb_page[i] == page_number) {
                frame_number = tlb_frame[i];
                tlb_hits++;
                break;
            }
        }

        //TO DO: tlb miss

        //get physical stuff
        int physical_address = (frame_number << 8) | offset;
        signed char value = physical_memory[frame_number][offset];

        //print to output files
        fprintf(out1, "%d\n", logical_address);
        fprintf(out2, "%d\n", physical_address);
        fprintf(out3, "%d\n", (int)value);
    }

    //print stats


    fclose(addresses);
    fclose(out1);
    fclose(out2);
    fclose(out3);
    return 0;
}