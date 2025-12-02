#ifndef MICROPHONE_H
#define MICROPHONE_H
#include "mqtt_handler.h"

//REMEMBER:
    //IF SOMETHING IS NOT WORKING - DOUBLE CHECK SPEAKER HAS RELINQUISHED CONTROL


static constexpr const size_t RECORDING_SAMPLE_RATE = 11025;

static constexpr const size_t RECORDING_BLOCK_COUNT             = 80;
static constexpr const size_t RECORDING_SAMPLES_PER_BLOCK       = 100;

static constexpr const size_t RECORDING_BUFFER_SIZE         = RECORDING_BLOCK_COUNT * RECORDING_SAMPLES_PER_BLOCK;
static constexpr const size_t RECORDING_MQTT_BUFFER_SIZE    = (RECORDING_BUFFER_SIZE * 2) + 100;



class Microphone{
    public:
        Microphone();
        ~Microphone();

        bool Init();
        bool RecordOneBlock();
        bool WriteToMqttBuffer(char* buffer, size_t buffer_size);

        bool PublishRecording();

    private:
        char* rec_mqtt_buffer;
        int16_t *recording_buffer;//Pointer to the samples buffer [record_number * record_length]
        size_t next_index;//Next index to record to
};


#endif