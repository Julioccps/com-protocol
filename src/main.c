#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_BYTE 0x55
#define MAX_PAYLOAD_SIZE 256

typedef struct {
    uint8_t start_byte;
    uint8_t message_id;
    uint8_t length;
    uint8_t payload[MAX_PAYLOAD_SIZE];
    uint8_t checksum;
} protocol_frame_t;

typedef enum{   
    STATE_WAIT_START,
    STATE_READ_HEADER,
    STATE_READ_PAYLOAD,
    STATE_VERIFY_CHECKSUM
} parser_state_t;

typedef struct {
    parser_state_t state;
    protocol_frame_t current_frame;
    uint8_t bytes_received;
    uint8_t payload_index;
}protocol_parser_t;

uint8_t calculate_checksum(const uint8_t* data, size_t length){
    uint8_t sum = 0;
    for(size_t i = 0; i < length; i++) {
        sum ^= data[i];
    }
    return sum;
}

protocol_frame_t mount_frame(const uint8_t* message, uint8_t id, uint8_t msg_length){
    protocol_frame_t frame;
    if (msg_length > MAX_PAYLOAD_SIZE){
        msg_length = MAX_PAYLOAD_SIZE;
    }

    frame.start_byte = START_BYTE;
    frame.message_id = id;
    frame.length = msg_length;

    memcpy(frame.payload, message, msg_length);
    
    uint8_t checksum_data[2 + MAX_PAYLOAD_SIZE];
    checksum_data[0] = frame.message_id;
    checksum_data[1] = frame.length;
    
    memcpy(&checksum_data[2], frame.payload, msg_length);
    
    frame.checksum = calculate_checksum(checksum_data, 2 + msg_length);

    return frame;
}

void transmit_to_file(const protocol_frame_t* frame, const char* filename){
    FILE* file = fopen(filename, "wb");
    if (!file){
        perror("Error while opening file");
        return;
    }

    fwrite(&frame->start_byte, 1, 1, file);
    fwrite(&frame->message_id, 1, 1, file);
    fwrite(&frame->length, 1, 1, file);
    fwrite(&frame->payload, 1, frame->length, file);
    fwrite(&frame->checksum, 1, 1, file);

    fclose(file);
    printf("Frame %u transmitted to %s\n", frame->message_id, filename);
}

int receive_from_file(protocol_frame_t* frame, const char* filename){
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return -1;
    }
    
    fread(&frame->start_byte, 1, 1, file);
    fread(&frame->message_id, 1, 1, file);
    fread(&frame->length, 1, 1, file);
    fread(frame->payload, 1, frame->length, file);
    fread(&frame->checksum, 1, 1, file);

    fclose(file);

    printf("Frame %u recieved from %s\n", frame->message_id, filename);
    return 0;
}


int main(){
    return 0;
}

int verify_checksum(const protocol_frame_t* frame){
    uint8_t checksum_data[2 + MAX_PAYLOAD_SIZE];
    checksum_data[0] = frame->message_id;
    checksum_data[1] = frame->length;
    memcpy(&checksum_data[2], frame->payload, frame->length);
    uint8_t calculated_checksum = calculate_checksum(checksum_data, 2 + frame->length);
    return (calculated_checksum == frame->checksum);
}

void simulate_corruption(protocol_frame_t* frame, int corruption_chance){
    if (corruption_chance > 0 && (rand() % 100 < corruption_chance)){
        int byte_to_corrupt = 2 + (rand() % (frame->length + 1)); 
        uint8_t* frame_data = (uint8_t*)frame;
        frame_data[byte_to_corrupt] = 0x0F;
        printf("Simulated corruption at byte ");
    }
}

void print_frame(const protocol_frame_t* frame){
    printf("Frame ID: %u, Length: %u, Checksum: 0x%02X\n", frame->message_id, frame->length, frame->checksum);
 
    printf("Payload: ");
    for (int i = 0;i < frame->length; i++){
        printf("%c", frame->payload[i]);
    }
    printf("\n");
}

void init_parser(protocol_parser_t* parser){
    parser->state = STATE_WAIT_START;
    parser->bytes_received = 0;
    parser->payload_index = 0;
    memset(&parser->current_frame, 0, sizeof(protocol_frame_t));
}

int parse_byte(protocol_parser_t* parser, uint8_t byte){
    switch (parser->state) {
        case STATE_WAIT_START:
            if (byte == START_BYTE){
                parser->current_frame.start_byte = byte;
                parser->state = STATE_READ_HEADER;
                parser->bytes_received = 1;
            }
            break;

        case STATE_READ_HEADER:
            if (parser->bytes_received== 1){
                parser->current_frame.message_id = byte;
                parser->bytes_received++;
            }
            else if (parser->bytes_received == 2){
                parser->current_frame.length = byte;
                parser->payload_index = 0;
                parser->state = STATE_READ_PAYLOAD;
                parser->bytes_received++;
            }
            break;
        case STATE_READ_PAYLOAD:
            parser->current_frame.payload[parser->payload_index++];
            if (parser->payload_index >= parser->current_frame.length){
                parser->state = STATE_VERIFY_CHECKSUM;
            }
        case STATE_VERIFY_CHECKSUM:
            parser->current_frame.checksum = byte;
            if (verify_checksum(&parser->current_frame)){
                parser->state = STATE_WAIT_START;
                return 1; // Frame complete and valid
            }
            else{
                parser->state = STATE_WAIT_START;
                return -1; // Frame corrupted
            }
            break;
    }
    return 0; // Frame incomplete

}
