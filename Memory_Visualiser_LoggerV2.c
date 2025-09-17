#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // Sleep()
#include <conio.h>    // _kbhit(), _getch()

typedef struct Block {
    void* addr;
    size_t size;
    int free;
    struct Block* next;
} Block;

Block* head = NULL;
const char* log_filename = "memory_log.json";

// Memory wrappers
void* my_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;

    Block* b = malloc(sizeof(Block));
    b->addr = ptr;
    b->size = size;
    b->free = 0;
    b->next = head;
    head = b;

    return ptr;
}

void my_free(void* ptr) {
    Block* temp = head;
    while (temp) {
        if (temp->addr == ptr && temp->free == 0) {
            temp->free = 1;
            break;
        }
        temp = temp->next;
    }
    free(ptr);
}

// Write memory state to JSON file
void log_memory() {
    FILE* f = fopen(log_filename, "w");
    if (!f) return;

    fprintf(f, "[\n");
    Block* temp = head;
    while (temp) {
        fprintf(f, "  {\"addr\": \"%p\", \"size\": %zu, \"free\": %d}%s\n",
                temp->addr, temp->size, temp->free, temp->next ? "," : "");
        temp = temp->next;
    }
    fprintf(f, "]\n");
    fclose(f);
}

// Free all allocated memory on exit
void cleanup() {
    Block* temp = head;
    while (temp) {
        Block* next = temp->next;
        if (!temp->free) free(temp->addr);
        free(temp);
        temp = next;
    }
}

int main() {
    printf("Live Memory Visualizer Running.\n");
    printf("Press 'a' to allocate 32B, 'f' to free last block, 'q' to quit.\n");

    while (1) {
        // Update JSON every 0.5s
        log_memory();
        Sleep(500);

        // Check for key press
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'a') {
                void* p = my_malloc(32);
                printf("Allocated 32B at %p\n", p);
            } else if (ch == 'f') {
                // Free last unfreed block
                Block* temp = head;
                while (temp) {
                    if (!temp->free) {
                        my_free(temp->addr);
                        printf("Freed block at %p\n", temp->addr);
                        break;
                    }
                    temp = temp->next;
                }
            } else if (ch == 'q') {
                printf("Exiting...\n");
                cleanup();
                break;
            }
        }
    }

    return 0;
}
