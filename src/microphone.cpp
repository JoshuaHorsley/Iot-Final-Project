#include "microphone.h"

Microphone::Microphone(){
    next_index = 0;
    recording_buffer = nullptr;
    rec_mqtt_buffer = nullptr;
}

Microphone::~Microphone(){
    if(recording_buffer != nullptr){
        free(recording_buffer);
    }
}
  

bool Microphone::Init(){
    
    //Yell at them for calling Init() twice.
    if(recording_buffer != nullptr || rec_mqtt_buffer !=nullptr){
        Serial.println("ERR:Microphone:Init: Recording_buffer or mqtt_buffer are no longer nullptrs. Did you call init twice?");
        return false;
    }

    //Allocate (BUFFER_SIZE * 2) bytes of memory for our circular buffer.
    // *2 cuz its 16-bit int, not 8.
    recording_buffer = (typeof(recording_buffer))heap_caps_malloc(
        RECORDING_BUFFER_SIZE * sizeof(int16_t), MALLOC_CAP_8BIT);

    //If allocation failed, return false.
    if(recording_buffer == NULL){
        Serial.println("ERR:Microphone:Init(): allocation failed for recording_buffer");
        return false;
    }


    rec_mqtt_buffer = (typeof(rec_mqtt_buffer))heap_caps_malloc(
        RECORDING_MQTT_BUFFER_SIZE * sizeof(char), MALLOC_CAP_8BIT);
    
    if(rec_mqtt_buffer == NULL){
        Serial.println("ERR:Microphone:Init(): allocation failed for mqtt_buffer");
        return false;
    }

    //Init all memory to 0
    memset(recording_buffer, 0, RECORDING_BUFFER_SIZE * sizeof(int16_t));
    memset(rec_mqtt_buffer, 0, RECORDING_MQTT_BUFFER_SIZE);

    return true;
}


//Record a single block of audio samples
    //This is basically one block of size "SAMPLES_IN_RECORDING_BLOCK"

//I *think* the usage is:
    //1. Record every loop.
    //2. Increment NEXT_INDEX back in main if this function returns true
    //3. Only send when our NEXT_INDEX rolls over.
bool Microphone::RecordOneBlock(){

    //Return early
    if(!M5.Mic.isEnabled()){
        Serial.println("ERR:Microphone:RecordOneBlock(): M5.Mic.isEnabled() returned FALSE");
        return false;
    }

    //Set a pointer to recording_buffer[NEXT_INDEX];
    auto data = &recording_buffer[next_index * RECORDING_SAMPLES_PER_BLOCK];

    //Write accumulated recording data.
    //If false, I think that means something went wrong down in the boiler room? 
    if(!M5.Mic.record(data, RECORDING_SAMPLES_PER_BLOCK, RECORDING_SAMPLE_RATE)){
        Serial.println("ERR:Microphone:RecordOneBlock(): M5.Mic.record returned FALSE");
        return false;
    }

    //Increment circular buffer
    next_index++;
    if (next_index >= RECORDING_BLOCK_COUNT){
        next_index = 0;
    }

    return true;    
}

//Copy to Mqtt buffer.
bool Microphone::WriteToMqttBuffer(char* buffer, size_t buffer_size){
    
    const size_t total_samples = RECORDING_BUFFER_SIZE;
    const size_t bytes_needed = total_samples * sizeof(int16_t);

    if(recording_buffer == nullptr){
        Serial.println("ERR:Microphone:WriteToMqttBuffer: recording_buffer is null. Did you forget to initialize?");
        return false;
    }

    //Is this an error state? Or should we just kind of send what we can..
    if(buffer_size < bytes_needed){
        Serial.print("ERR:Microphone:WriteToMqttBuffer: Tried to write ");
        Serial.print(bytes_needed);
        Serial.print(" bytes to a buffer of size ");
        Serial.println(buffer_size);
        return false;
    }
    
    //Find where our "Next Index" is.
    size_t starting_block = next_index;
    size_t starting_sample = starting_block * RECORDING_SAMPLES_PER_BLOCK;

    //Grab current index --> end of circular buffer
    size_t first_half_sample_count = total_samples - starting_sample;
    //Grab the count of bytes in the first half.
    size_t first_half_byte_count = first_half_sample_count * sizeof(int16_t);

    //////// COPY FROM NEXT INDEX --> IDX[MAX] //////
    memcpy(buffer, 
        &recording_buffer[starting_sample], 
        first_half_byte_count);

    ////// COPY FROM IDX[0] --> NEXT INDEX /////
    if(starting_sample > 0){
        size_t second_half_sample_count = starting_sample;
        size_t second_half_byte_count = second_half_sample_count * sizeof(int16_t);

        memcpy(buffer + first_half_byte_count,
        &recording_buffer[0],
        second_half_byte_count);
    }

    return true;
}

bool Microphone::PublishRecording(){

    if(recording_buffer == nullptr || rec_mqtt_buffer == nullptr){
        Serial.println("ERR:Microphone:PublishRecording: Failed to initialze buffers. Aborting publish.");
        return false;
    }

    if(!WriteToMqttBuffer(this->rec_mqtt_buffer, RECORDING_MQTT_BUFFER_SIZE)){
        Serial.println("ERR:Microphone:PublishRecording: Write to MQTT buffer failed. Aborting publish.");
        return false;
    }

    String metaTopic = mqtt_base + "mic/meta";
    String dataTopic = mqtt_base + "mic/bytes";


    const int metaData = RECORDING_BUFFER_SIZE * sizeof(int16_t);
    char temp_buffer[40];
    memset(temp_buffer, 0, 40);
    sprintf(temp_buffer, "%d", metaData);

    if(!client.publish(metaTopic.c_str(), temp_buffer)){
        Serial.println("client.publish( metadata )  returned FALSE.");
    }

    Serial.print("MQTT BUFFER SIZE: ");
    Serial.println(client.getBufferSize());

    if(!client.publish(dataTopic.c_str(), (uint8_t*)rec_mqtt_buffer, metaData)){
        Serial.println("client.publish( rec_mqtt_buffer )  returned FALSE.");
    }

    return true;
}

