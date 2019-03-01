#ifndef SERIALIZEDDATA_H
#define SERIALIZEDDATA_H

#include <type_traits>
#include <vector>
#include <stdexcept>
#include <cstring>

namespace serialize {

namespace traits {

template<typename T>
class BufferTraits {
public:
    static constexpr bool resizable = false;
    static constexpr bool constant = std::is_const<T>::value;
    
    
    // Increase size so the buffer will hold at least amount more items
    static void increaseSize(T &t, std::size_t amount) {
        static_assert(std::is_void<T>::value, "Include buffer trait for container or container is not resizable");
    }
};





template<typename T>
class StdContainerTraits {
public:
    using TIterator = typename T::iterator;
    using TConstIterator = typename T::const_iterator;
    using TValue = typename T::value_type;
    
    static void increaseSize(T &t, std::size_t amount) {
        t.reserve(std::max(t.size() + amount, static_cast<std::size_t>(t.size() * 1.5 + 128)));
        t.resize(t.size() + amount);
    }
};





template<class T, class Allocator>
class BufferTraits<std::vector<T, Allocator>> : public StdContainerTraits<std::vector<T, Allocator>> {
    
};

template<class T, class Traits, class Allocator>
class BufferTraits<std::basic_string<T, Traits, Allocator>> : public StdContainerTraits<std::vector<T, Allocator>> {
    
};
} // namespace traits







template<typename T>
class IsSerializable : std::false_type {
};


template<typename Buffer>
class OutputBufferAdapter {
public:
    static_assert(!std::is_const<Buffer>::value, "Buffer must not be const");
    
    using Iterator = typename traits::BufferTraits<Buffer>::TIterator;
    using TValue = typename traits::BufferTraits<typename std::remove_const<Buffer>::type>::TValue;
    
    OutputBufferAdapter(Buffer &buffer) : buffer_(&buffer), pointer_(std::begin(buffer)), end_(std::end(buffer)) {}
    
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type * = nullptr>
    void write(T t) {
        write(reinterpret_cast<const TValue*>(&t), sizeof(T));
    }
    
    void write(const TValue *data, std::size_t size) {
        const auto remaining = std::distance(pointer_, end_);
        if (remaining < size) {
            // Store current position
            const auto pos = std::distance(std::begin(*buffer_), pointer_);
            traits::BufferTraits<Buffer>::increaseSize(*buffer_, size - remaining);
            // Restore iterators
            pointer_ = std::next(std::begin(*buffer_), pos);
            end_ = std::end(*buffer_);
        }
        
        std::copy(data, data + size, pointer_);
        pointer_ += size;
    }
    
private:
    Buffer *buffer_;
    
    Iterator pointer_;
    Iterator end_;
};




template<typename Buffer>
class InputBufferAdapter {
public:
    using Iterator = typename traits::BufferTraits<Buffer>::TConstIterator;
    using TValue = typename traits::BufferTraits<typename std::remove_const<Buffer>::type>::TValue;
    
    InputBufferAdapter(Buffer &buffer) : pointer_(std::begin(buffer)), end_(std::end(buffer)) {}
    
    void read(TValue *data, std::size_t size) {
        if (pointer_ + size <= end_) {
            std::copy(pointer_, pointer_ + size, data);
            pointer_ += size;
            return;
        }
        throw std::runtime_error("end of buffer");
    }
    
    
private:
    Iterator pointer_;
    Iterator end_;
};



// Default deserializers
// String
template<typename D, class T, class Traits, class Allocator>
void deserialize(D &d, std::basic_string<T, Traits, Allocator> &string) {
    typename std::basic_string<T, Traits, Allocator>::size_type size;
    d.deserialize(size);
    T data[size];
    d.deserialize(data, size);
    string.assign(data, size);
}

// Vector
template<typename D, typename T, class Allocator>
void deserialize(D &d, std::vector<T, Allocator> &vector) {
    std::size_t size;
    d.deserialize(size);
    vector.resize(size);
    d.deserialize(vector.data(), size);
}

// C array literal (size is NOT stored)
template<typename D, typename T, std::size_t SIZE>
void deserialize(D &d, T (&t)[SIZE]) {
    d.deserialize(t, SIZE);
}

// Default serializers
template<typename S>
void serialize(S &s, const std::string &string) {
    s.serialize(string.size());
    s.serialize(string.c_str(), string.size());
}


template<typename S, typename T>
void serialize(S &s, const T *data, std::size_t size) {
    s.serialize(size);
    s.serialize(data, size);
}

// C array literal (size is NOT stored)
template<typename D, typename T, std::size_t SIZE>
void serialize(D &d, const T (&t)[SIZE]) {
    d.serialize(t, SIZE);
}



template<typename S>
void serialize(S &s, const char *string) {
    std::size_t len = std::strlen(string);
    s.template serialize(len);
    s.serialize(string, len);
}


template<class...> using void_t = void;


template<typename, typename, typename = void>
struct HasSerializeFunction : std::false_type {
};

template<typename S, typename T>
struct HasSerializeFunction<S, T, void_t<decltype(serialize(std::declval<S&>(), std::declval<T&>()))>> : std::true_type {
};


template<typename, typename, typename = void>
struct HasDeserializeFunction : std::false_type {
};

template<typename S, typename T>
struct HasDeserializeFunction<S, T, void_t<decltype(deserialize(std::declval<S&>(), std::declval<T&>()))>> : std::true_type {
};


template<typename S, typename T>
struct SerializeFunction {
    static void invoke(S &s, const T &t) {
        static_assert(HasSerializeFunction<S, T>::value, "Please define 'serialize' for type");
        
        serialize(s, t);
    }
};


template<typename S, typename T>
struct DeserializeFunction {
    static void invoke(S &s, T &t) {
        static_assert(HasDeserializeFunction<S, T>::value, "Please define 'deserialize' for type");
        
        deserialize(s, t);
    }
};



template<typename InputAdapter>
class Deserializer {
public:
    Deserializer(InputAdapter &&input) : input_{std::forward<InputAdapter>(input)} {}
    
    template<typename T>
    void load(T &t) {
        return deserialize(t);
    }
    
    template<typename T>
    void deserialize(T &t) {
        return read(t);
    }
    
    template<typename T>
    void deserialize(T *t, std::size_t size) {
        return read(t, size);
    }
    
private:
    InputAdapter input_;
    
    // Arithmetic types
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type * = nullptr>
    void read(T &t) {
        input_.read(reinterpret_cast<typename InputAdapter::TValue*>(&t), sizeof(T));
    }
    
    // Arithmetic arrays
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type * = nullptr>
    void read(T *t, std::size_t size) {
        input_.read(reinterpret_cast<typename InputAdapter::TValue*>(t), sizeof(T) * size);
    }
    
    // Other types
    template<typename T, typename std::enable_if<!std::is_arithmetic<T>::value>::type * = nullptr>
    void read(T &t) {
        DeserializeFunction<Deserializer, T>::invoke(*this, t);
    }
};



template<typename OutputAdapter>
class Serializer {
public:
    Serializer(OutputAdapter &&output) : output_{std::forward<OutputAdapter>(output)} {}
    
    template<typename T>
    void save(const T &t) {
        serialize(t);
    }
    
    template<typename T>
    void serialize(const T &t) {
        write(t);
    }
    
    template<typename T>
    void serialize(T *t, std::size_t size) {
        write(t, size);
    }
    
private:
    OutputAdapter output_;
    
    // Arithmetic types
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type * = nullptr>
    void write(const T &t) {
        output_.write(reinterpret_cast<const typename OutputAdapter::TValue*>(&t), sizeof(T));
    }
    
    // Arithmetic arrays
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type * = nullptr>
    void write(T *t, std::size_t size) {
        output_.write(reinterpret_cast<const typename OutputAdapter::TValue*>(t), sizeof(T) * size);
    }
    
    // Other
    template<typename T, typename std::enable_if<!std::is_arithmetic<T>::value>::type * = nullptr>
    void write(const T &t) {
        SerializeFunction<Serializer, T>::invoke(*this, t);
    }
};


} // namespace serialize

#endif // SERIALIZEDDATA_H
