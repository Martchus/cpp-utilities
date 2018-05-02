#ifndef CPP_UTILITIES_MULTI_ARRAY_H
#define CPP_UTILITIES_MULTI_ARRAY_H

#include <array>
#include <tuple>
#include <vector>

namespace MiscUtilities {

/// \cond
namespace Detail {
template <class Tuple, std::size_t N> struct DimensionsHelper {
    static std::size_t requiredSize(const Tuple &dimensionSizes)
    {
        return DimensionsHelper<Tuple, N - 1>::requiredSize(dimensionSizes) * std::get<N - 1>(dimensionSizes);
    }
    static std::size_t offset(const Tuple &dimensions, const Tuple &indices, std::size_t factor)
    {
        return DimensionsHelper<Tuple, N - 1>::offset(dimensions, indices, factor * std::get<N - 1>(dimensions))
            + (factor * std::get<N - 1>(indices));
    }
};
template <class Tuple> struct DimensionsHelper<Tuple, 1> {
    static std::size_t requiredSize(const Tuple &dimensionSizes)
    {
        return std::get<0>(dimensionSizes);
    }
    static std::size_t offset(const Tuple &, const Tuple &indices, std::size_t factor)
    {
        return factor * std::get<0>(indices);
    }
};
} // namespace Detail
/// \endcond

/// \brief The VectorBasedMultiArray struct allows using an std::vector with custom allocator as underlying container for the MultiArray class.
template <typename Allocator> struct VectorBasedMultiArray {
    template <typename T> using Type = std::vector<T, Allocator>;
    template <typename T> static constexpr Type<T> init(std::size_t requiredSize)
    {
        return Type<T>(requiredSize);
    }
};

/// \brief The VectorBasedMultiArray struct allows using an std::vector as underlying container for the MultiArray class.
template <> struct VectorBasedMultiArray<void> {
    template <typename T> using Type = std::vector<T, std::allocator<T>>;
    template <typename T> static constexpr Type<T> init(std::size_t requiredSize)
    {
        return Type<T>(requiredSize);
    }
};

/// \brief The ArrayBasedMultiArray struct allows using a fixed size array as underlying container for the MultiArray class.
template <std::size_t size> struct ArrayBasedMultiArray {
    template <typename T> using Type = std::array<T, size>;
    template <typename T> static constexpr Type<T> init(std::size_t)
    {
        return Type<T>();
    }
};

/// \brief The NoneOwningMultiArray struct allows using a caller-managed buffer array as underlying container for the MultiArray class.
struct NoneOwningMultiArray {
    template <typename T> using Type = T *;
    template <typename T> static constexpr Type<T> init(std::size_t)
    {
        return nullptr;
    }
};

/// \brief The MultiArray class provides an *N*-dimensional array.
template <typename T, typename UnderlyingContainer, typename... Dimensions> class MultiArray {
public:
    MultiArray(Dimensions... dimensionSizes);
    std::size_t totalSize() const;
    static constexpr std::size_t dimensionCount();
    template <std::size_t index> std::size_t dimensionSize() const;
    T &at(Dimensions... indices);
    const T &at(Dimensions... indices) const;
    T *data();
    const T *data() const;
    typename UnderlyingContainer::template Type<T> &buffer();

private:
    using HelperType = Detail::DimensionsHelper<std::tuple<Dimensions...>, dimensionCount()>;
    const std::tuple<Dimensions...> m_dims;
    const std::size_t m_size;
    typename UnderlyingContainer::template Type<T> m_buff;
};

/// \brief Constructs a new *N*-dimensional array. The sizes for the dimensions are passed as arguments.
/// \remarks The number of dimensions *N* is deduced from the number of \a dimensionSizes.
/// \sa makeMultiArray(), makeFixedSizeMultiArray() and makeNoneOwningMultiArray() for more convenient construction
template <typename T, typename UnderlyingContainer, typename... Dimensions>
MultiArray<T, UnderlyingContainer, Dimensions...>::MultiArray(Dimensions... dimensionSizes)
    : m_dims(std::make_tuple(dimensionSizes...))
    , m_size(HelperType::requiredSize(m_dims))
    , m_buff(UnderlyingContainer::template init<T>(m_size))
{
}

/// \brief Returns the total number of elements.
template <typename T, typename UnderlyingContainer, typename... Dimensions>
std::size_t MultiArray<T, UnderlyingContainer, Dimensions...>::totalSize() const
{
    return m_size;
}

/// \brief Returns the number of dimensions for that type of array.
template <typename T, typename UnderlyingContainer, typename... Dimensions>
constexpr std::size_t MultiArray<T, UnderlyingContainer, Dimensions...>::dimensionCount()
{
    return std::tuple_size<std::tuple<Dimensions...>>::value;
}

/// \brief Returns the number of elements in the specified dimension.
template <typename T, typename UnderlyingContainer, typename... Dimensions>
template <std::size_t index>
std::size_t MultiArray<T, UnderlyingContainer, Dimensions...>::dimensionSize() const
{
    return std::get<index>(m_dims);
}

/// \brief Returns the element at the position specified via \a indices.
/// \remarks The number of \a indices must equal dimensionCount().
template <typename T, typename UnderlyingContainer, typename... Dimensions>
T &MultiArray<T, UnderlyingContainer, Dimensions...>::at(Dimensions... indices)
{
    return m_buff[HelperType::offset(m_dims, std::make_tuple(indices...), 1)];
}

/// \brief Returns the element at the position specified via \a indices.
/// \remarks The number of \a indices must equal dimensionCount().
template <typename T, typename UnderlyingContainer, typename... Dimensions>
const T &MultiArray<T, UnderlyingContainer, Dimensions...>::at(Dimensions... indices) const
{
    return m_buff[HelperType::offset(m_dims, std::make_tuple(indices...), 1)];
}

/// \brief Returns a pointer to the raw data.
/// \remarks Intended for debugging purposes only. The underlying data structure might change in future versions.
template <typename T, typename UnderlyingContainer, typename... Dimensions> T *MultiArray<T, UnderlyingContainer, Dimensions...>::data()
{
    return m_buff.data();
}

/// \brief Returns a pointer to the raw data.
/// \remarks Intended for debugging purposes only. The underlying data structure might change in future versions.
template <typename T, typename UnderlyingContainer, typename... Dimensions> const T *MultiArray<T, UnderlyingContainer, Dimensions...>::data() const
{
    return m_buff.data();
}

/// \brief Allows accessing the underlying buffer directly.
/// \remarks Assign the custom buffer using this method when using NoneOwningMultiArray as UnderlyingContainer.
template <typename T, typename UnderlyingContainer, typename... Dimensions>
typename UnderlyingContainer::template Type<T> &MultiArray<T, UnderlyingContainer, Dimensions...>::buffer()
{
    return m_buff;
}

/// \brief Constructs a new *N*-dimensional array using an std::vector with std::allocator as underlying container.
///        The sizes for the dimensions are passed as arguments.
/// \remarks The number of dimensions *N* is deduced from the number of \a dimensionSizes.
template <typename ValueType, typename... DimensionSizes> inline auto makeMultiArray(DimensionSizes... dimensionSizes)
{
    return MultiArray<ValueType, VectorBasedMultiArray<void>, DimensionSizes...>(dimensionSizes...);
}

/// \brief Constructs a new *N*-dimensional array using a fixed size array as underlying container.
///        The sizes for the dimensions are passed as arguments.
/// \remarks The number of dimensions *N* is deduced from the number of \a dimensionSizes.
template <typename ValueType, std::size_t size, typename... DimensionSizes> inline auto makeFixedSizeMultiArray(DimensionSizes... dimensionSizes)
{
    return MultiArray<ValueType, ArrayBasedMultiArray<size>, DimensionSizes...>(dimensionSizes...);
}

/// \brief Constructs a new *N*-dimensional array using a caller-managed buffer as underlying container.
///        The sizes for the dimensions are passed as arguments.
/// \remarks The number of dimensions *N* is deduced from the number of \a dimensionSizes.
template <typename ValueType, typename... DimensionSizes> inline auto makeNoneOwningMultiArray(DimensionSizes... dimensionSizes)
{
    return MultiArray<ValueType, NoneOwningMultiArray, DimensionSizes...>(dimensionSizes...);
}

} // namespace MiscUtilities

#endif // CPP_UTILITIES_MULTI_ARRAY_H
