#ifndef LIBRETUNER_MEMORYBUFFER_H
#define LIBRETUNER_MEMORYBUFFER_H

#include <vector>

namespace lt
{
class View;

class MemoryBuffer
{
public:
    using iterator = std::vector<uint8_t>::iterator;
    using const_iterator = std::vector<uint8_t>::const_iterator;

    MemoryBuffer(const MemoryBuffer&) = delete;
    MemoryBuffer(MemoryBuffer&&) = default;
    MemoryBuffer & operator=(const MemoryBuffer&) = delete;

    MemoryBuffer() = default;
    explicit MemoryBuffer(std::vector<uint8_t> && data) : data_(std::move(data))
    {
    }

    template <typename It> MemoryBuffer(It begin, It end)
    {
        static_assert(sizeof(std::decay_t<decltype(*std::declval<It>())>) == 1,
                      "Iterator type must be byte");
        data_.assign(begin, end);
    }

    inline iterator begin() { return data_.begin(); }
    inline iterator end() { return data_.end(); }
    inline const_iterator cbegin() const { return data_.cbegin(); }
    inline const_iterator cend() const { return data_.cend(); }

    inline uint8_t * operator*() noexcept { return data_.data(); }
    inline uint8_t & operator[](int index) { return data_[index]; }
    inline const uint8_t & operator[](int index) const { return data_[index]; }

    inline int size() const noexcept { return static_cast<int>(data_.size()); }
    inline uint8_t * data() noexcept { return data_.data(); }
    inline const uint8_t * data() const noexcept { return data_.data(); }

    View view();
    View view(int offset, int size);

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(data_);
    }

private:
    std::vector<uint8_t> data_;
};
} // namespace lt

#endif // LIBRETUNER_MEMORYBUFFER_H
