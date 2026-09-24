
module;

#include <vector>
#include <ranges>
#include <cmath>
#include <iostream>
#include <numbers>

export module dsp.window;

import util.type;

namespace dsp {

export class window : public util::dsp_operation_base {

private:
        std::vector< float > m_window;

private:
        constexpr window() = delete;
        constexpr explicit window( std::vector< float >&& _Window ) : m_window( std::move( _Window ) ) {}

public:
        constexpr ~window() noexcept = default;

        constexpr static window hanning( std::size_t _Size ) {

                if ( _Size < 2 ) {
                        throw std::invalid_argument( "hanning window size must be greater than 2" );
                }

                auto value = [ _Size ]( std::size_t _N ) -> float {
                        return 0.5 * ( 1.0 - std::cos( 2.0 * std::numbers::pi * _N / ( _Size - 1 ) ) );
                };

                return window(
                        std::views::iota( static_cast< std::size_t >( 0 ), _Size )
                        | std::views::transform( value )
                        | std::ranges::to< std::vector<float> >()
                );

        }

        constexpr auto size() const {
                return m_window.size();
        }

        constexpr auto apply_to( std::ranges::range auto&& _Frame_data ) {

                if ( std::ranges::distance( _Frame_data ) != m_window.size() ) {
                        throw std::invalid_argument( "Frame data must be the same size as window" );
                }

                return std::views::zip_transform(
                        std::multiplies{},
                        _Frame_data, m_window
                );
        }

};

} // namespace dsp
