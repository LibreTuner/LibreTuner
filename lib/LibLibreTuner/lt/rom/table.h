#ifndef LIBRETUNER_TABLE_H
#define LIBRETUNER_TABLE_H

#include <cassert>
#include <memory>
#include <vector>

#include "../support/types.h"
#include "../support/util.hpp"

namespace lt
{

// Provides a way to interface with the underlying data of many tables/axes
// by using a common datatype.
template<typename PresentedType>
struct Entries
{
    virtual PresentedType get(int index) const noexcept = 0;
    virtual void set(int index, PresentedType value) noexcept = 0;

    // Serializes the entries into a byte array
    virtual std::vector<uint8_t> intoBytes(Endianness endianness) const
    noexcept = 0;
    virtual ~Entries() = default;
};

// Type-defined implementation of Entries
template <typename PresentedType, typename T>
struct EntriesImpl : public Entries<PresentedType>
{
private:
    template <Endianness endianness>
    inline std::vector<uint8_t> intoBytes_() const noexcept
    {
        // Endianness-aware byte serialization
        std::vector<uint8_t> out;
        out.reserve(entries_.size() * sizeof(T));
        for (T entry : entries_)
        {
            entry = endian::convert<T, endian::current, endianness>(entry);
            // Get byte representation and append to buffer
            uint8_t * repr = reinterpret_cast<uint8_t *>(&entry);
            out.insert(out.end(), repr, &repr[sizeof(T)]);
        }
        return out;
    }

public:
    EntriesImpl(std::vector<T> && entries) : entries_(std::move(entries)) {}

    PresentedType get(int index) const noexcept override { return static_cast<PresentedType>(entries_[index]); }
    void set(int index, PresentedType value) noexcept
    {
        entries_[index] = static_cast<T>(value);
    }
    std::vector<uint8_t> intoBytes(Endianness endianness) const
    noexcept override
    {
        switch (endianness)
        {
        case Endianness::Big:
            return intoBytes_<Endianness::Big>();
        case Endianness::Little:
            return intoBytes_<Endianness::Little>();
        default:
            return std::vector<uint8_t>();
        }
    }
    std::vector<T> entries_;
};

/* A memory axis uses a defined set of values as indicies. */
template <typename PresentedType> class BasicAxis
{
private:
    explicit BasicAxis(std::unique_ptr<Entries<PresentedType>> entries, int size) : entries_(std::move(entries)), size_(size)
    {
    }

public:
    class Builder
    {
    public:
        template<typename T>
        Builder & setEntries(std::vector<T> && entries)
        {
            size_ = entries.size();
            entries_ = std::make_unique<EntriesImpl<PresentedType, T>>(std::move(entries));
            return *this;
        }

        /* Creates a set of linear entries. A linear axis
         * uses the form y = mx + b where m is the step, b
         * is the start, and x is the index. */
        template<typename T>
        Builder & setLinear(T start, T step, int size)
        {
            std::vector<T> entries;
            for (int i = 0; i < size; ++i, start += step)
                entries.emplace_back(start);
            size_ = size;
            return setEntries(std::move(entries));
        }

        BasicAxis<PresentedType> build()
        {
            if (!entries_)
                throw std::runtime_error("attempt to build axis without setting entries");
            return BasicAxis<PresentedType>(std::move(entries_), size_);
        }
    private:
        std::unique_ptr<Entries<PresentedType>> entries_;
        int size_;
    };

    PresentedType index(int index) const noexcept
    {
        if (index < 0 || index >= size_)
            return PresentedType{};
        return entries_->get(index);
    }

    int size() const noexcept { return size_; }

private:
    std::unique_ptr<Entries<PresentedType>> entries_;
    int size_;
};

// Defines the minimum and maximum bounds of table entries
template <typename T> struct Bounds
{
    T minimum, maximum;

    bool within(T t) const noexcept { return t >= minimum && t <= maximum; }
};

template <typename PresentedType> class BasicTable
{
public:
    using AxisType = BasicAxis<PresentedType>;
    using AxisTypePtr = std::shared_ptr<AxisType>;

    /* Creates a one-dimensional index from a two-dimensional point.
     * Calculated by multiplying the row and height and adding the
     * column. Checks if the point is in bounds and throws an exception
     * if the check fails. */
    inline int index(int row, int column) const
    {
        assert(row > 0 && column > 0);
        if (row >= width_ || column >= height_)
            throw std::runtime_error("point (" + std::to_string(row) + ", " +
                                     std::to_string(column) +
                                     ") out of bounds.");
        return row * height_ + column;
    }

    /* Returns the entry at position (`row`, `column`). Throws an
     * exception if the point is out-of-bounds. */
    PresentedType get(int row, int column) const
    {
        return static_cast<PresentedType>(entries_->get(index(row, column)) *
                                          scale_);
    }

    /* Sets the entry at position (`row`, `column`) to `value`. Throws
     * an exception if the point is out-of-bounds. */
    void set(int row, int column, PresentedType value) noexcept
    {
        entries_->set(index(row, column),
                      static_cast<PresentedType>(value / scale_));
        dirty_ = true;
    }

    // Getters
    inline const std::string & name() const noexcept { return name_; }
    inline int width() const noexcept { return width_; }
    inline int height() const noexcept { return height_; }
    inline AxisTypePtr xAxis() const noexcept { return xAxis_; }
    inline AxisTypePtr yAxis() const noexcept { return yAxis_; }
    inline PresentedType minimum() const noexcept { return bounds_.minimum; }
    inline PresentedType maximum() const noexcept { return bounds_.maximum; }

    /* Returns true if the dirty bit is set. Thit bit is set
     * every time set() is called. */
    inline bool dirty() const noexcept { return dirty_; }

    // Clears the dirty bit
    inline void clearDirty() noexcept { dirty_ = false; }

    // Returns true if the value is within the entry bounds
    inline bool inBounds(PresentedType value) const noexcept
    {
        return bounds_.within(value);
    }

    /* Returns true if the table holds a single value
     * (width = height = 1) */
    inline bool isSingle() const noexcept
    {
        return width_ == 1 && height_ == 1;
    }

    /* Returns true if the table contains a single row
     * (height = 1) */
    inline bool isOneDimensional() const noexcept{
        return height_ == 1;
    }

    // Serializes entries into bytes
    std::vector<uint8_t> intoBytes(Endianness endianness) const noexcept
    {
        return entries_->intoBytes(endianness);
    }

private:
    std::string name_;
    Bounds<PresentedType> bounds_;
    std::unique_ptr<Entries<PresentedType>> entries_;
    int width_, height_;
    AxisTypePtr xAxis_, yAxis_;
    double scale_;
    bool dirty_{false};

    BasicTable(std::string name, Bounds<PresentedType> bounds,
               std::unique_ptr<Entries<PresentedType>> && entries, int width, int height,
               AxisTypePtr && xAxis, AxisTypePtr && yAxis, double scale)
        : name_(std::move(name)), bounds_(std::move(bounds)),
          entries_(std::move(entries)), width_(width), height_(height),
          xAxis_(std::move(xAxis)), yAxis_(std::move(yAxis)), scale_(scale)
    {
        assert(entries_);
    }

public:
    // Builder pattern for cleaner code
    class Builder
    {
    public:
        inline Builder & setSize(int width, int height) noexcept
        {
            width_ = width;
            height_ = height;
            return *this;
        }

        inline Builder & setName(std::string name) noexcept
        {
            name_ = std::move(name);
            return *this;
        }

        inline Builder & setBounds(PresentedType minimum,
                                   PresentedType maximum) noexcept
        {
            bounds_.minimum = minimum;
            bounds_.maximum = maximum;
            return *this;
        }

        inline Builder & setScale(double scale) noexcept
        {
            scale_ = scale;
            return *this;
        }

        template <typename T>
        Builder & setEntries(std::vector<T> && entries) noexcept
        {
            entries_ = std::make_unique<EntriesImpl<PresentedType, T>>(std::move(entries));
            return *this;
        }

        // Deserializes entries from bytes
        template <typename T, typename It>
        inline Builder & operator()(It begin, It end, Endianness endianness)
        {
            return setEntries(fromBytes<T>(std::forward<It>(begin), std::forward<It>(end),
                      endianness));
        }

        inline Builder & setXAxis(AxisTypePtr xAxis) noexcept
        {
            xAxis_ = std::move(xAxis);
            return *this;
        }

        inline Builder & setYAxis(AxisTypePtr yAxis) noexcept
        {
            yAxis_ = std::move(yAxis);
            return *this;
        }

        BasicTable<PresentedType> build() noexcept
        {
            return BasicTable<PresentedType>(
                std::move(name_), std::move(bounds_), std::move(entries_),
                width_, height_, std::move(xAxis_), std::move(yAxis_), scale_);
        }

    private:
        int width_, height_;
        double scale_{1.0};
        AxisTypePtr xAxis_;
        AxisTypePtr yAxis_;
        std::string name_;
        Bounds<PresentedType> bounds_;
        std::unique_ptr<Entries<PresentedType>> entries_;
    };
    friend class Builder;
};

// Alias for a double-presented table
using Table = BasicTable<double>;
using TablePtr = std::shared_ptr<Table>;

using Axis = BasicAxis<double>;
using AxisPtr = std::shared_ptr<Axis>;

} // namespace lt

#endif // LIBRETUNER_TABLE_H
