#include <type_traits>
#include <array>
#include <initializer_list>
#include <memory>
#include <utility>
#include <variant>
#include <numeric>
#include <iostream>
#include <print>
#include <format>

namespace qc::tools{
    template<std::size_t... Dimensions>
    constexpr auto build_coeffs_array(){
        constexpr std::array<std::size_t, sizeof...(Dimensions)> dimensions { Dimensions... };
        constexpr std::size_t coeffs_size = sizeof...(Dimensions);
        std::array<std::size_t, coeffs_size> coeffs = {};
        
        coeffs[coeffs_size - 1] = 1;

        for(int i{coeffs_size - 2}; i >= 0; i--){
            coeffs[i] = dimensions[i + 1] * coeffs[i + 1];
        }
        return coeffs;
    }
    /*
    coeff[0] = v1size * ... * v[n-1] size
    coeff[1] = v2size * .... *v[n-1] size
    ...
    coeff[n-2] = v[n-1] size
    coeff[n-1] = 1
    */
    template<std::size_t... Dimensions>
    std::size_t constexpr to_1d_index(auto... indices){
        constexpr std::array<std::size_t, sizeof...(Dimensions)> dimensions { Dimensions... };
        constexpr std::size_t coeffs_size = sizeof...(Dimensions);
        constexpr auto coeffs = build_coeffs_array<Dimensions...>();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
    
        return [&]<size_t... Is>(std::index_sequence<Is...>){
            return ((coeffs[Is] * indices) + ...);
        }(std::make_index_sequence<coeffs_size>());
    }

    /*
    Suppose we have the dimensions <3, 5, 2> and the coordinates (1, 3, 1).
    (1, 3, 1) maps to the linear index 17. 
    */

    template<size_t... Dimensions>
    constexpr decltype(auto) from_1d_index(size_t initState){ 
        size_t state = initState;
        static constexpr auto coeffs = build_coeffs_array<Dimensions...>();
        std::array<size_t, sizeof...(Dimensions)> coords{};
        for(size_t i{0}; i < sizeof...(Dimensions); ++i){
            coords[i] = static_cast<size_t>(state / coeffs[i]);
            state -= coords[i] * coeffs[i];
        }
        return coords;
    }
}