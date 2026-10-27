#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv) {

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

        /*check tlb*/
        for (int i = 0; i < 16; i++) {
            if (tlb_page[i] == page_number) {
                frame_number = tlb_frame[i];
                tlb_hits++;
                break;
            }
        }

        /*tlb miss*/
        if (frame_number == -1) {
            frame_number = page_table[page_number];

            if (frame_number == -1) {
                page_faults++;
                frame_number = next_frame;
                next_frame++;

                fseek(backing_store, page_number * 256, SEEK_SET);
                fread(physical_memory[frame_number], 1, 256, backing_store);

                page_table[page_number] = frame_number;
            }

            /*fifo tlb update*/
            tlb_page[tlb_idx] = page_number;
            tlb_frame[tlb_idx] = frame_number;
            tlb_idx = (tlb_idx + 1) % 16;
        }

        /*get physical */
        int physical_address = (frame_number << 8) | offset;
        signed char value = physical_memory[frame_number][offset];

        /*print to output files*/
        fprintf(out1, "%d\n", logical_address);
        fprintf(out2, "%d\n", physical_address);
        fprintf(out3, "%d\n", (int)value);
    }

    /*print stats for statistics*/
    printf("Page Faults: %d\n", page_faults);
    printf("Page Fault Rate: %.3f\n", (double)page_faults / total_addresses);
    printf("TLB Hits: %d\n", tlb_hits);
    printf("TLB Hit Rate: %.3f\n", (double)tlb_hits / total_addresses);


    fclose(addresses);
    fclose(out1);
    fclose(out2);
    fclose(out3);
    return 0;
}