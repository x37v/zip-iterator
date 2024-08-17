#include <functional>

namespace c9 {

  template <typename T>
    using select_iterator_for = std::conditional_t<
    std::is_const_v<std::remove_reference_t<T>>,
    typename std::remove_reference_t<T>::const_iterator,
    typename std::remove_reference_t<T>::iterator>;

template <typename T>
using select_access_type_for = std::conditional_t<
    std::is_const_v<std::remove_reference_t<T>>,
    typename std::remove_reference_t<T>::const_reference,
    typename std::remove_reference_t<T>::reference>;

template <typename ... Args, std::size_t ... Index>
auto any_match_impl(std::tuple<Args...> const & lhs,
    std::tuple<Args...> const & rhs,
    std::index_sequence<Index...>) -> bool
{
    auto result = false;
    result = (... || (std::get<Index>(lhs) == std::get<Index>(rhs)));
    return result;
}


template <typename ... Args>
auto any_match(std::tuple<Args...> const & lhs, std::tuple<Args...> const & rhs)
    -> bool
{
    return any_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
}

template <typename ... T>
class zip_iterator
{
public:
    using value_type = std::tuple<select_access_type_for<T>...>;
    using Iters = std::tuple<select_iterator_for<T>...>;

    zip_iterator() = delete;

    zip_iterator(Iters && iters)
        : m_iters {std::forward<Iters>(iters)}
    {
    }

    auto operator++() -> zip_iterator& 
    {
        std::apply(zip_iterator<T...>::inc_iters, m_iters);
        return *this;
    }

    auto operator++(int) -> zip_iterator 
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    auto operator!=(zip_iterator const & other) const
    {
        return !(*this == other);
    }

    auto operator==(zip_iterator const & other) const
    {
        return any_match(m_iters, other.m_iters);
    }

    auto operator*() -> value_type
    {
        return std::apply(zip_iterator<T...>::map_value, m_iters);
    }

private:
    Iters m_iters;

    static void inc_iters(select_iterator_for<T>& ... args){ 
      ((args += 1), ...);
    }

    static value_type map_value(select_iterator_for<T>& ... args){ 
      return value_type(*args...);
    }
};


template <typename ... T>
class zipper
{
public:
    using zip_type = zip_iterator<T ...>;

    template <typename ... Args>
    zipper(Args && ... args)
        : m_args{std::forward<Args>(args)...}
    {
    }

    auto begin() -> zip_type
    {
        return std::apply(zipper<T...>::map_begin, m_args);
    }
    auto end() -> zip_type
    {
        return std::apply(zipper<T...>::map_end, m_args);
    }

private:
    std::tuple<T ...> m_args;

    static zip_type map_begin(T && ... args){ 
      return zip_type(std::make_tuple(std::begin(args)...)); 
    }

    static zip_type map_end(T && ... args){ 
      return zip_type(std::make_tuple(std::end(args)...)); 
    }
};


template <typename ... T>
auto zip(T && ... t)
{
    return zipper<T ...>{std::forward<T>(t)...};
}

}
