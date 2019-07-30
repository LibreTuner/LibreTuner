#ifndef LIBRETUNER_TABLE_H
#define LIBRETUNER_TABLE_H

#include <cassert>
#include <memory>
#include <vector>

#include "../buffer/view.h"
#include "../support/types.h"
#include "../support/util.hpp"

namespace lt
{

// Provides a way to interface with the underlying data of many tables/axes
// by using a common datatype.
template <typename PresentedType> class Entries
{
public:
    virtual PresentedType get(int index) const = 0;
    virtual void set(int index, PresentedType value) = 0;
    virtual int size() const noexcept =0;

    virtual ~Entries() = default;
};

template <typename T> using EntriesPtr = std::unique_ptr<Entries<T>>;

// Type-defined implementation of Entries
template <typename PresentedType, typename T, Endianness endianness>
struct EntriesImpl : public Entries<PresentedType>
{
public:
    EntriesImpl(View view) : view_(std::move(view)) {}

    PresentedType get(int index) const override
    {
        return static_cast<PresentedType>(
            view_.get<T, endianness>(index * sizeof(T)));
    }
    void set(int index, PresentedType value)
    {
        view_.set<T, endianness>(static_cast<T>(value), index * sizeof(T));
    }
    int size() const noexcept {
        return view_.size() / sizeof(T);
    }

private:
    View view_;
};

template <typename PresentedType, Endianness endianness>
EntriesPtr<PresentedType> create_entries(DataType type, View view)
{
    switch (type)
    {
    case DataType::Uint8:
        return std::make_unique<
            EntriesImpl<PresentedType, uint8_t, endianness>>(view);
    case DataType::Uint16:
        return std::make_unique<
            EntriesImpl<PresentedType, uint16_t, endianness>>(view);
    case DataType::Uint32:
        return std::make_unique<
            EntriesImpl<PresentedType, uint32_t, endianness>>(view);
    case DataType::Int8:
        return std::make_unique<EntriesImpl<PresentedType, int8_t, endianness>>(
            view);
    case DataType::Int16:
        return std::make_unique<
            EntriesImpl<PresentedType, int16_t, endianness>>(view);
    case DataType::Int32:
        return std::make_unique<
            EntriesImpl<PresentedType, int32_t, endianness>>(view);
    case DataType::Float:
        return std::make_unique<EntriesImpl<PresentedType, float, endianness>>(
            view);
    }
}

template <typename PresentedType> class AxisEntries
{
public:
    virtual ~AxisEntries() = default;
    virtual PresentedType get(int index) const =0;
};
template<typename PresentedType>
using AxisEntriesPtr = std::unique_ptr<AxisEntries<PresentedType>>;

template <typename PresentedType> class AxisMemoryEntries : public AxisEntries<PresentedType>
{
public:
    AxisMemoryEntries(EntriesPtr<PresentedType> && entries)
        : entries_(std::move(entries))
    {
    }

    PresentedType get(int index) const override { return entries_->get(index); }

private:
    EntriesPtr<PresentedType> entries_;
};

template <typename PresentedType> class AxisLinearEntries : public AxisEntries<PresentedType>
{
public:
    AxisLinearEntries(PresentedType first, PresentedType step)
        : first_(first), step_(step)
    {
    }

    PresentedType get(int index) const override
    {
        return first_ + index * step_;
    }

private:
    PresentedType first_, step_;
};

/* A memory axis uses a defined set of values as indicies. */
template <typename PresentedType> class BasicAxis
{
private:
    explicit BasicAxis(AxisEntriesPtr<PresentedType> && entries,
                       std::string && name, int size)
        : entries_(std::move(entries)), size_(size), name_(std::move(name))
    {
    }

public:
    class Builder
    {
    public:
        Builder & setEntries(EntriesPtr<PresentedType> && entries)
        {
            size_ = entries->size();
            entries_ = std::make_unique<AxisMemoryEntries<PresentedType>>(std::move(entries));
            return *this;
        }

        Builder & setLinear(PresentedType first, PresentedType step)
        {
            entries_ = std::make_unique<AxisLinearEntries<PresentedType>>(first, step);
            size_ = std::numeric_limits<int>::max();
            return *this;
        }

        Builder & setName(std::string name)
        {
            name_ = std::move(name);
            return *this;
        }

        BasicAxis<PresentedType> build()
        {
            if (!entries_)
                throw std::runtime_error(
                    "attempt to build axis without setting entries");
            return BasicAxis<PresentedType>(std::move(entries_),
                                            std::move(name_), size_);
        }

    private:
        AxisEntriesPtr<PresentedType> entries_;
        int size_;
        std::string name_;
    };

    PresentedType index(int index) const noexcept
    {
        if (index < 0 || index >= size_)
            return PresentedType{};
        return entries_->get(index);
    }

    int size() const noexcept { return size_; }

    const std::string & name() const noexcept { return name_; }

private:
    AxisEntriesPtr<PresentedType> entries_;
    int size_;
    std::string name_;
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
     * Calculated by multiplying the row and width and adding the
     * column. Checks if the point is in bounds and throws an exception
     * if the check fails. */
    inline int index(int row, int column) const
    {
        assert(row >= 0 && column >= 0);
        if (row >= height_ || column >= width_)
            throw std::runtime_error("point (" + std::to_string(row) + ", " +
                                     std::to_string(column) +
                                     ") out of bounds.");
        return row * width_ + column;
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
    inline bool isOneDimensional() const noexcept { return height_ == 1; }

private:
    std::string name_;
    Bounds<PresentedType> bounds_;
    EntriesPtr<PresentedType> entries_;
    int width_, height_;
    AxisTypePtr xAxis_, yAxis_;
    double scale_;
    bool dirty_{false};

    BasicTable(std::string name, Bounds<PresentedType> bounds,
               std::unique_ptr<Entries<PresentedType>> && entries, int width,
               int height, AxisTypePtr && xAxis, AxisTypePtr && yAxis,
               double scale)
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

        Builder & setEntries(EntriesPtr<PresentedType> && entries)
        {
            entries_ = std::move(entries);
            return *this;
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
            // TODO: Verify entry size
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
        EntriesPtr<PresentedType> entries_;
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
