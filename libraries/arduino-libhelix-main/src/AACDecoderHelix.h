#pragma once

#include "CommonHelix.h"
#include "libhelix-aac/aacdec.h"

#define AAC_MAX_OUTPUT_SIZE 1024 * 3 
#define AAC_MAX_FRAME_SIZE 2100 

namespace libhelix {

typedef void (*AACInfoCallback)(_AACFrameInfo &info, void* caller);
typedef void (*AACDataCallback)(_AACFrameInfo &info,short *pcm_buffer, size_t len, void* caller);

/**
 * @brief A simple Arduino API for the libhelix AAC decoder. The data us provided with the help of write() calls.
 * The decoded result is available either via a callback method or via an output stream.
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class AACDecoderHelix : public CommonHelix {
    public:
        AACDecoderHelix() = default;

#if defined(ARDUINO) || defined(HELIX_PRINT)
        AACDecoderHelix(Print &output){
            this->out = &output;
        }
#endif
        AACDecoderHelix(AACDataCallback dataCallback){
            this->pcmCallback = dataCallback;
        }

        virtual ~AACDecoderHelix(){
            end();
        }

        void setInfoCallback(AACInfoCallback cb, void* caller=nullptr){
            this->infoCallback = cb;
            p_caller_info = caller;
        }

        void setDataCallback(AACDataCallback cb){
            this->pcmCallback = cb;
        }

        /// Releases the reserved memory
        virtual void end() override {
            LOG_HELIX(Debug, "end");
            if (decoder!=nullptr){
                flush();
                AACFreeDecoder(decoder);
                decoder = nullptr;
            }
            CommonHelix::end();
            memset(&aacFrameInfo,0,sizeof(_AACFrameInfo));
        }

        /// Provides the last available _AACFrameInfo_t
        _AACFrameInfo audioInfo(){
            return aacFrameInfo;
        }

        size_t maxFrameSize() override {
            return max_frame_size == 0 ? AAC_MAX_FRAME_SIZE : max_frame_size;
        }

        size_t maxPCMSize() override {
            return max_pcm_size == 0 ? AAC_MAX_OUTPUT_SIZE : max_pcm_size;
        }

    protected:
        HAACDecoder decoder = nullptr;
        AACDataCallback pcmCallback = nullptr;
        AACInfoCallback infoCallback = nullptr;
        _AACFrameInfo aacFrameInfo;
        void *p_caller_info = nullptr;
        void *p_caller_data = nullptr;

        /// Allocate the decoder
        virtual void allocateDecoder() override {
            if (decoder==nullptr){
                decoder = AACInitDecoder();
            }
        }

        /// finds the sync word in the buffer
        int findSynchWord(int offset=0) override {
            int result = AACFindSyncWord(frame_buffer+offset, buffer_size)+offset;
            return result < 0 ? result : result + offset;
        }

        /// decods the data and removes the decoded frame from the buffer
        void decode(Range r) override {
            LOG_HELIX(Debug, "decode %d", r.end);
            int len = buffer_size - r.start;
            int bytesLeft =  len; 
            uint8_t* ptr = frame_buffer + r.start;

            int result = AACDecode(decoder, &ptr, &bytesLeft, pcm_buffer);
            int decoded = len - bytesLeft;
            assert(decoded == ptr-(frame_buffer + r.start));
            if (result==0){
                LOG_HELIX(Debug, "-> bytesLeft %d -> %d  = %d ", buffer_size, bytesLeft, decoded);
                LOG_HELIX(Debug, "-> End of frame (%d) vs end of decoding (%d)", r.end, decoded)

                // return the decoded result
                _AACFrameInfo info;
                AACGetLastFrameInfo(decoder, &info);
                provideResult(info);

                // remove processed data from buffer 
                if (decoded<=buffer_size) {
                    buffer_size -= decoded;
                    //assert(buffer_size<=maxFrameSize());
                    memmove(frame_buffer, frame_buffer+r.start+decoded, buffer_size);
                    LOG_HELIX(Debug, " -> decoded %d bytes - remaining buffer_size: %d", decoded, buffer_size);
                } else {
                    LOG_HELIX(Warning, " -> decoded %d > buffersize %d", decoded, buffer_size);
                    buffer_size = 0;
                }
            } else {
                // decoding error
                LOG_HELIX(Debug, " -> decode error: %d - removing frame!", result);
                int ignore = decoded;
                if (ignore == 0) ignore = r.end;
                // We advance to the next synch world
                if (ignore<=buffer_size){
                    buffer_size -= ignore;
                    memmove(frame_buffer, frame_buffer+ignore, buffer_size);
                }  else {
                    buffer_size = 0;
                }
            }
        }

        /// Directly calls AACDecode
        size_t decodeRaw(uint8_t* data, size_t len) override {
            int bytesLeft =  len; 
            int result = AACDecode(decoder, &data, &bytesLeft, pcm_buffer);
            if (result==0){
                // return the decoded result
                _AACFrameInfo info;
                AACGetLastFrameInfo(decoder, &info);
                provideResult(info);
            }
            return len - bytesLeft;
        }


        // return the result PCM data
        void provideResult(_AACFrameInfo &info){
            LOG_HELIX(Debug, "provideResult: %d samples",info.outputSamps);
             if (info.outputSamps>0){
            // provide result
                if(pcmCallback!=nullptr){
                    // output via callback
                    pcmCallback(info, pcm_buffer,info.outputSamps, p_caller_data);
                } else {
                    // output to stream
                    if (info.sampRateOut!=aacFrameInfo.sampRateOut && infoCallback!=nullptr){
                        infoCallback(info, p_caller_info);
                    }
#if defined(ARDUINO) || defined(HELIX_PRINT)
                    int sampleSize = info.bitsPerSample / 8;
                    out->write((uint8_t*)pcm_buffer, info.outputSamps*sampleSize);
#endif
                }
                aacFrameInfo = info;
            }
        }            
};
}
