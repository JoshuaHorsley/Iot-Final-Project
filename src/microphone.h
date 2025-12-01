#ifndef MICROPHONE_H
#define MICROPHONE_H
#include "gui.h"

//REMEMBER:
    //IF SOMETHING IS NOT WORKING - DOUBLE CHECK SPEAKER HAS RELINQUISHED CONTROL

static constexpr const size_t RECORDING_BLOCK_COUNT             = 200;
static constexpr const size_t RECORDING_SAMPLES_PER_BLOCK       = 240;

static constexpr const size_t RECORDING_BUFFER_SIZE         = RECORDING_BLOCK_COUNT * RECORDING_SAMPLES_PER_BLOCK;
static constexpr const size_t RECORDING_SAMPLE_RATE         = 22050;


class Microphone{
    public:
        Microphone();
        ~Microphone();

        bool Init();
        bool RecordOneBlock();
        bool WriteToMqttBuffer(char* buffer, size_t buffer_size);

        int16_t GetRecordingBuffer();
        size_t GetNextIndex();

    private:
        int16_t *recording_buffer;//Pointer to the samples buffer [record_number * record_length]
        size_t next_index;//Next index to record to
};


//Returns allocated buffer initialized to 0
boolean initRecordingCircularBuffer(int16_t* recording_buffer, size_t buffer_size);

boolean recordOneBlock(int16_t* recording_buffer, size_t next_index, size_t buffer_size);

#endif