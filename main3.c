#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv) {

    if (argc < 2) //just stops execution if no address file was given
    {
        printf("Usage: %s addresses.txt\n", argv[0]);
        return 1;
    }

    FILE *addresses = fopen(argv[1], "r");
    FILE *backing_store = fopen("BACKING_STORE.bin", "rb");
    FILE *out1 = fopen("out1.txt", "w");
    FILE *out2 = fopen("out2.txt", "w");
    FILE *out3 = fopen("out3.txt", "w");

    //stop if any file doesnt open
    if (addresses == NULL || backing_store == NULL || out1 == NULL || out2 == NULL || out3 == NULL)
    {
        printf("File wasnt opened\n");
        return 1;
    }

    int page_table[256];
    int tlb_page[16];
    int tlb_frame[16];
    signed char physical_memory[128][256]; //physical memory limited to 128 frames, must replace once full

    int frame_toPage[128]; //which page is stored in each frame, used to target page we need to evict
    int last_used[128]; //when each frame was last accessed

    for (int i = 0; i < 256; i++) {
        page_table[i] = -1;
    }
    for (int i = 0; i < 16; i++) {
        tlb_page[i] = -1;
        tlb_frame[i] = -1;
    }

    
    for (int i = 0; i < 128; i++) //all frames start empty and unspent
    {
        frame_toPage[i] = -1;
        last_used[i] = 0;
    }

    int tlb_idx = 0;             
    int next_frame = 0;          
    int page_faults = 0;
    int tlb_hits = 0;
    int total_addresses = 0;

    int timeForAccess = 0; //goes up by 1 for every address, essentialy works as a timestamp
    
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


                if (next_frame < 128) //use free frame if one is available
                {
                    frame_number = next_frame;
                    next_frame++;

                }
                else //memory is full, replace the oldest used frame
                {
                    int lru_frame = 0;
                    
                    for (int i = 1; i < 128; i++) //finds the frame with the oldest time signature
                    {
                        if (last_used[i] < last_used[lru_frame])
                        {
                            lru_frame = i;
                        }
                    }

                    frame_number = lru_frame;

                    int page_toRemove = frame_toPage[frame_number];

                    page_table[page_toRemove] = -1; //evicted page is no longer in mem, next access will cause page fault

                    for ( int i = 0; i < 16; i++) //remove evicted page from tlb so it cant point to the resued frame
                    {
                        if (tlb_page[i] == page_toRemove)
                        {
                            tlb_page[i] = -1;
                            tlb_frame[i] = -1;
                        }
                    }
                }
                

                fseek(backing_store, page_number * 256, SEEK_SET);
                fread(physical_memory[frame_number], 1, 256, backing_store);

                page_table[page_number] = frame_number;

                frame_toPage[frame_number] = page_number; //records which page now lives in this  current frame
            }

            /*fifo tlb update*/
            tlb_page[tlb_idx] = page_number;
            tlb_frame[tlb_idx] = frame_number;
            tlb_idx = (tlb_idx + 1) % 16;
        }
        

        timeForAccess++; //updates the timestamp on every access, regardless if theres a tlb hit or not
        last_used[frame_number] = timeForAccess;

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
    fclose(backing_store);
    fclose(out1);
    fclose(out2);
    fclose(out3);
    return 0;
}