#ifndef LT_ROM_TABLE_H
#define LT_ROM_TABLE_H

#include <limits>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "../support/types.h"
#include "../support/util.hpp"

namespace lt {

class TableAxis {
public:
    virtual double label(int index) const = 0;
    virtual const std::string &name() const = 0;
    virtual ~TableAxis();
};
using TableAxisPtr = std::shared_ptr<TableAxis>;

template <typename DataType> class MemoryAxis : public TableAxis {
public:
    template <typename InputIt>
    MemoryAxis(std::string name, InputIt begin, InputIt end);

    virtual double label(int index) const override;
    virtual const std::string &name() const override;

private:
    std::vector<DataType> data_;
    std::string name_;
};

template <typename DataType>
template <typename InputIt>
MemoryAxis<DataType>::MemoryAxis(std::string name, InputIt begin, InputIt end)
    : data_(begin, end), name_(name) {}

template <typename DataType>
double MemoryAxis<DataType>::label(int index) const {
    if (index < 0 || index >= data_.size()) {
        return 0;
    }

    return data_[index];
}

template <typename DataType>
const std::string &MemoryAxis<DataType>::name() const {
    return name_;
}

template <typename T> class LinearAxis : public TableAxis {
public:
    LinearAxis(std::string name, T start, T increment);

    virtual double label(int index) const override;
    virtual const std::string &name() const override;

private:
    T start_, increment_;
    std::string name_;
};

template <typename T>
LinearAxis<T>::LinearAxis(std::string name, T start, T increment)
    : start_(start), increment_(increment), name_(std::move(name)) {}

template <typename T> double LinearAxis<T>::label(int index) const {
    return (start_) + index * increment_;
}

template <typename T> const std::string &LinearAxis<T>::name() const {
    return name_;
}

template <typename T> struct TableBounds {
    T minimum_{std::numeric_limits<T>::min()};
    T maximum_{std::numeric_limits<T>::max()};

    inline bool withinBounds(T value) const noexcept {
        return value >= minimum_ && value <= maximum_;
    }
};

using StorageVariant = std::variant<std::vector<uint8_t>, std::vector<int8_t>,
                                    std::vector<uint16_t>, std::vector<int16_t>,
                                    std::vector<uint32_t>, std::vector<int32_t>,
                                    std::vector<float>>;

class TableStorage {
public:
    template <typename T> void set(std::size_t index, T value) noexcept {
        std::visit(
            [index, value](auto &&vec) {
                if (index >= vec.size()) {
                    throw std::runtime_error("out of bounds");
                }
                vec[index] = value;
            },
            store_);
    }

    template <typename T> T get(std::size_t index) const noexcept {
        return std::visit(
            [index](auto &&vec) -> T {
                if (index >= vec.size()) {
                    throw std::runtime_error("out of bounds");
                }
                return static_cast<T>(vec[index]);
            },
            store_);
    }

    std::size_t size() const noexcept {
        return std::visit([](auto &&vec) -> std::size_t { return vec.size(); },
                          store_);
    }

    std::vector<uint8_t> serialize(Endianness endianness) const {
        std::vector<uint8_t> serialized;
        std::visit(
            [&serialized, endianness](auto &&vec) {
                using EntryType = typename std::decay_t<decltype(vec)>::value_type;
                serialized.resize(vec.size() * sizeof(EntryType));
                if (endianness == Endianness::Big) {
                    writeBE<EntryType>(vec.begin(), vec.end(),
                                       serialized.begin());
                } else if (endianness == Endianness::Little) {
                    writeLE<EntryType>(vec.begin(), vec.end(),
                                       serialized.begin());
                }
            },
            store_);
        return serialized;
    }

    // Returns an iterator pointing to the value after the last entry
    template <typename Iter>
    void deserialize(Iter begin, Iter end, Endianness endianness) {
        static_assert(sizeof(*std::declval<Iter>()) == 1,
                      "Iterator must be byte iterator");

        std::visit(
            [&begin, &end, endianness](auto &&vec) {
                using EntryType = typename std::decay_t<decltype(vec)>::value_type;

                if (std::distance(begin, end) / sizeof(EntryType) !=
                    vec.size()) {
                    throw std::runtime_error(
                        "byte buffer is not equal to the table size");
                }

                if (endianness == Endianness::Big) {
                    readBE<EntryType>(begin, end, vec.begin());
                } else if (endianness == Endianness::Little) {
                    readLE<EntryType>(begin, end, vec.begin());
                }
            },
            store_);
    }

    template <typename T> static TableStorage create(std::size_t size) {
        static_assert(std::is_arithmetic_v<T>, "Type must be arithmetic");
        TableStorage store;
        store.store_ = std::vector<T>(size);
        return store;
    }

private:
    StorageVariant store_;
};

template <typename EntryType,
          typename = std::enable_if_t<std::is_arithmetic<EntryType>::value>>
class BasicTable {
public:
    template <typename T>
    void initialize(std::size_t width, std::size_t height) {
        entries_ = TableStorage::create<T>(width * height);
        width_ = width;
    }

    inline EntryType get(std::size_t x, std::size_t y) const noexcept {
        return entries_.get<EntryType>(y * width() + x);
    }

    void set(std::size_t x, std::size_t y, EntryType value) {
        if (!bounds_.withinBounds(value)) {
            throw std::runtime_error("value out of bounds");
        }
		if (get(x, y) != value) {
			setDirty();
			entries_.set<EntryType>(y * width() + x, value);
		}
    }

    inline std::size_t width() const noexcept { return width_; }
    inline std::size_t height() const noexcept {
        return entries_.size() / width_;
    }
    inline std::size_t size() const noexcept { return entries_.size(); }

    template <typename T = EntryType, typename Iter>
    Iter serialize(Iter begin, Iter end) const {
        return entries_.serialize(std::move(begin), std::move(end));
    }

    inline std::vector<uint8_t> serialize(Endianness endianness) const {
        return entries_.serialize(endianness);
    }

    template <typename Iter>
    inline void deserialize(Iter begin, Iter end, Endianness endianness) {
        entries_.deserialize(std::forward<Iter>(begin),
                                std::forward<Iter>(end), endianness);
    }

    inline std::string name() const noexcept { return name_; }
    inline std::string description() const noexcept { return description_; }

    void setName(const std::string &name) noexcept { name_ = name; }
    void setDescription(const std::string &description) noexcept {
        description_ = description;
    }

    inline const TableBounds<EntryType> &bounds() const noexcept {
        return bounds_;
    }
    inline void setBounds(const TableBounds<EntryType> &bounds) noexcept {
        bounds_ = bounds;
    }

    inline EntryType minimum() const noexcept { return bounds_.minimum_; }
    inline EntryType maximum() const noexcept { return bounds_.maximum_; }

    inline bool isOneDimensional() const noexcept { return height() == 1; }
    inline bool isTwoDimensional() const noexcept { return height() > 1; }
    inline bool isSingle() const noexcept {
        return height() == 1 && width() == 1;
    }

    inline void setAxisX(const TableAxisPtr &axis) noexcept { axisX_ = axis; }
    inline const TableAxisPtr &axisX() const noexcept { return axisX_; }

    inline void setAxisY(const TableAxisPtr &axis) noexcept { axisY_ = axis; }
    inline const TableAxisPtr &axisY() const noexcept { return axisY_; }

    inline double scale() const noexcept { return scale_; }
    inline void setScale(double scale) noexcept { scale_ = scale; }

	inline void setDirty() noexcept { dirty_ = true; }
	inline void clearDirty() noexcept { dirty_ = false; }
	inline bool dirty() const noexcept { return dirty_; }

private:
    TableStorage entries_;
    std::size_t width_; // row width
    double scale_{1.0};

    TableBounds<EntryType> bounds_;
    std::string name_;
    std::string description_;

    TableAxisPtr axisX_;
    TableAxisPtr axisY_;
	bool dirty_{false};
};

using Table = BasicTable<double>;
using TablePtr = std::unique_ptr<Table>;

} // namespace lt

#endif
