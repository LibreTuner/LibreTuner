#ifndef LT_ROM_TABLE_H
#define LT_ROM_TABLE_H

#include <limits>
#include <memory>
#include <string>
#include <variant>
#include <vector>

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
        Logger::warning("Axis index '" + std::to_string(index) +
                        "' exceeds axis size (" + std::to_string(data_.size()) +
                        ")");
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

    template <typename T, typename Iter>
    Iter serialize(Iter begin, Iter end, double scale = 1.0) const {
        if (std::distance(begin, end) < size()) {
            throw std::runtime_error("buffer too small");
        }
        std::visit([&begin](auto &&vec) {
            for (auto entry : vec) {
                *begin++ = static_cast<T>(entry * scale);
            }
        });
        return begin;
    }

    // Returns an iterator pointing to the value after the last entry
    template <typename T = EntryType, typename Iter>
    Iter deserialize(Iter begin, Iter end, double scale = 1.0) {
        if (std::distance(begin, end) < size()) {
            throw std::runtime_error("buffer too small");
        }
        std::visit([&begin](auto &&vec) {
            for (auto &entry : vec) {
                entry = static_cast<T>(*begin++ * scale);
            }
        });
        return begin;
    }

    template <typename T> static TableStorage create(std::size_t size) {
        static_assert(std::is_arithmetic_v(T), "Type must be arithmetic");
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

    void set(std::size_t x, std::size_t y, EntryType value) noexcept {
        if (!bounds_.withinBounds(value)) {
            throw std::runtime_error("value out of bounds");
        }
        entries_.set<EntryType>(y * width() + x, value);
    }

    inline std::size_t width() const noexcept { return width_; }
    inline std::size_t height() const noexcept {
        return entries_.size() / width_;
    }
    inline std::size_t size() const noexcept { return entries_.size(); }

    template <typename T = EntryType, typename Iter>
    Iter serialize(Iter begin, Iter end, double scale = 1.0) const {
        return entries_.serialize<T>(std::move(begin), std::move(end), scale);
    }

    template <typename T = EntryType>
    std::vector<T> serialize(double scale = 1.0) const {
        std::vector<T> res;
        res.resize(entries_.size());
        serialize<T>(res.begin(), res.end(), scale);
        return res;
    }

    // Returns an iterator pointing to the value after the last entry
    template <typename T = EntryType, typename Iter>
    Iter deserialize(Iter begin, Iter end, double scale = 1.0) {
        return entries_.deserialize<T>(begin, end, scale);
    }

    inline std::string name() const noexcept { return name_; }
    inline std::string description() const noexcept { return description_; }

    void setName(const std::string &name) noexcept { name_ = name; }
    void setDescription(const std::string &description) noexcept {
        description_ = description;
    }

    inline TableBounds<EntryType> &bounds() const noexcept { return bounds_; }
    inline void setBounds(const TableBounds<EntryType> &bounds) noexcept {
        bounds_ = bounds;
    }

    inline bool isOneDimensional() const noexcept { return height() == 1; }
    inline bool isTwoDimensional() const noexcept { return height() > 1; }
    inline bool isSingle() const noexcept {
        return height() == 0 && width() == 0;
    }

    inline void setAxisX(const TableAxisPtr &axis) noexcept { axisX_ = axis; }
    inline const TableAxisPtr &axisX() const noexcept { return axisX_; }

    inline void setAxisY(const TableAxisPtr &axis) noexcept { axisY_ = axis; }
    inline const TableAxisPtr &axisY() const noexcept { return axisY_; }

private:
    TableStorage entries_;
    std::size_t width_; // row width

    TableBounds<EntryType> bounds_;
    std::string name_;
    std::string description_;

    TableAxisPtr axisX_;
    TableAxisPtr axisY_;
};

using Table = BasicTable<double>;
using TablePtr = std::unique_ptr<Table>;

} // namespace lt

#endif