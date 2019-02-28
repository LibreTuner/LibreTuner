#ifndef SERIALIZEDDATA_H
#define SERIALIZEDDATA_H

namespace serialize {

namespace traits {
template<typename T>
class BufferTraits {
    static constexpr bool resizable = false;


};
}


class InputAdapter {

};


template<typename Buffer>
class InputBufferAdapter {
    InputBufferAdapter(Buffer &buffer) : buffer_(&buffer) {}

    using TIterator = typename Buffer::iterator;

private:
    Buffer *buffer_;
};

};

#endif // SERIALIZEDDATA_H
