#include "MixedBuffer.h"

MixedBuffer::MixedBuffer(BufferType type, uint8_t *data, int size, bool own_data)
        : mSize(size),
          mData(data),
          bOwnData(own_data),
          mBufferType(type) {
    initType(type);
}

MixedBuffer::MixedBuffer(BufferType type, int capacity)
        : mSize(0),
          mData(new uint8_t[capacity]),
          bOwnData(true),
          mBufferType(type) {
    initType(type);
}

MixedBuffer::~MixedBuffer() {
    if (bOwnData) {
        delete[] mData;
    }
}

void MixedBuffer::initType(BufferType type) {
    switch (type) {
        case BufferType::BUFFER_VIDEO_FRAME:
            mVideoFrameMetadata = std::make_unique<VideoFrameMetadata>();
            break;
        case BufferType::BUFFER_AUDIO_FRAME:
            mAudioFrameMetadata = std::make_unique<AudioFrameMetadata>();
            break;
        case BufferType::BUFFER_VIDEO_FORMAT:
            mVideoFormatMetadata = std::make_unique<VideoFormatMetadata>();
            break;
        case BufferType::BUFFER_VIDEO_PACKET:
            mVideoPacketMetadata = std::make_unique<VideoPacketMetadata>();
            break;
        case BufferType::BUFFER_AUDIO_PACKET:
            mAudioPacketMetadata = std::make_unique<AudioPacketMetadata>();
            break;
        default:
            break;
    }
}

int MixedBuffer::getSize() const {
    return mSize;
}

uint8_t *MixedBuffer::getData() const {
    return mData;
}

BufferType MixedBuffer::getType() const {
    return mBufferType;
}

uint8_t *MixedBuffer::obtainData() {
    if (bOwnData && mSize > 0) {
        bOwnData = false;
        return mData;
    }
    return nullptr;
}

VideoFrameMetadata *MixedBuffer::getVideoFrameMetadata() const {
    return mVideoFrameMetadata.get();
}

AudioFrameMetadata *MixedBuffer::getAudioFrameMetadata() const {
    return mAudioFrameMetadata.get();
}

VideoPacketMetadata *MixedBuffer::getVideoPacketMetadata() const {
    return mVideoPacketMetadata.get();
}

AudioPacketMetadata *MixedBuffer::getAudioPacketMetadata() const {
    return mAudioPacketMetadata.get();
}

VideoFormatMetadata *MixedBuffer::getVideoFormatMetadata() const {
    return mVideoFormatMetadata.get();
}

void MixedBuffer::updateBuffer(uint8_t *data, int size, bool ownData) {
    if (bOwnData) {
        delete[] mData;
    }

    mData    = data;
    mSize    = size;
    bOwnData = ownData;
}
