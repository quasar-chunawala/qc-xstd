#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <array>
#include <functional>
#include <variant>

namespace qc{
    /**
     * @brief A light n-dimensional array.
     * 
     */
    template<typename T, std::size_t Size>
    struct poly_array{
        T m_buffer[Size] = {};

        constexpr T& operator[](std::size_t idx){
            return m_buffer[idx];
        }
 
        decltype(auto) constexpr at(std::size_t index){
            return m_buffer[index];
        }
        template<typename... Indices>
        decltype(auto) constexpr at(std::size_t index, Indices... indices){
            return m_buffer[index].at(indices...);
        }
    };

    template<size_t... Indices>
    struct dispatcher{
        template<typename Func, typename... Vs>
        static constexpr auto dispatch(Func&& f, Vs&&... vs){
            using return_type_t = decltype(std::forward<Func>(f)(std::get<0>(std::forward<Vs>(vs))...));
            if constexpr(std::is_same_v<return_type_t, void>){
                std::invoke(std::forward<Func>(f), std::get<Indices>(std::forward<Vs>(vs))...);
            }else{
                return std::invoke(std::forward<Func>(f), std::get<Indices>(std::forward<Vs>(vs))...);
            }
        };
    };

    template<typename Func, typename... Vs, size_t... Indices>
    constexpr auto make_dispatch(std::index_sequence<Indices...>){
        return dispatcher<Indices...>::template dispatch<Func, Vs...>;
    }

    /**
     * @brief The terminal case, when we are at the leaf of the multi-dimensional
     * array.
     */
    template<typename Func, typename... Vs, size_t... Is>
    constexpr auto make_func_poly_array_impl(std::index_sequence<Is...>){
        return make_dispatch<Func, Vs...>(std::index_sequence<Is...>());
    }

    /**
     * @brief Constructs a 1d-array indexed by non-type template parameter <Is...,J>
     *        whose each element is a recursive call to make_func_poly_array_impl
     */
    template<typename Func, typename... Vs, size_t... Is, size_t... Js, typename... Seqs>
    constexpr auto make_func_poly_array_impl(std::index_sequence<Is...>, std::index_sequence<Js...>, Seqs... seqs){
        using result_t = decltype(make_func_poly_array_impl<Func, Vs...>(std::index_sequence<Is...,0>(), seqs...));
        return poly_array<result_t,sizeof...(Js)>{ (make_func_poly_array_impl<Func, Vs...>(std::index_sequence<Is..., Js>(), seqs...))...};
    }

    template<typename Func, typename... Vs>
    constexpr auto make_func_poly_array(){
        return make_func_poly_array_impl<Func, Vs...>(std::index_sequence<>(), std::make_index_sequence<std::variant_size_v<std::remove_cvref_t<Vs>>>()...);
    }
}