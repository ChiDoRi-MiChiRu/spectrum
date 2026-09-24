
module;

#include <ranges>
#include <vector>
#include <complex>
#include <cmath>

export module dsp.channel;

import util;

namespace dsp {

export
enum class stereo : int {
        left  = 0,
        right = 1,

};

export
class split_channels : public util::dsp_operation_base {

private:
        stereo m_channel;

public:

        constexpr explicit split_channels( stereo _Channel ) : m_channel( _Channel ) {}

        template< std::ranges::range _Range_Ty >
        util::erased_view< float > apply_to( _Range_Ty&& _Range ) {
                return std::views::iota( 0, std::ranges::distance( std::forward< _Range_Ty >( _Range ) ) / 2 )
                        | std::views::transform(
                                [
                                        ch = static_cast< int >( m_channel ),
                                        ra = std::forward< _Range_Ty >( _Range )
                                ]( auto i ) {
                                        return ra[ i * 2 + ch ];
                                }
                        )
                ;
        }

};

export
class average : public util::mix_method_base {};

export
class rms : public util::mix_method_base {

private:
        inline static auto combine = []( std::complex< float > _Left, std::complex< float > _Right ) {
                return std::sqrt( ( std::norm( _Left  ) + std::norm( _Right ) ) / 2.0f );
        };

public:

        template< std::ranges::range _Left_Ty, std::ranges::range _Right_Ty >
        static auto mix( _Left_Ty&& _Left, _Right_Ty&& _Right ) {
                return std::views::zip_transform(
                        combine,
                        std::forward< _Left_Ty  >( _Left  ),
                        std::forward< _Right_Ty >( _Right )
                );
        }

};

export
class maximum : public util::mix_method_base {};

export
template< util::is_mix_method _Method >
class mix_by : public util::dsp_operation_base {

public:
        template< typename _Pair_Range_Ty >
        auto apply_to( _Pair_Range_Ty&& _Pair_range ) {
                auto&& [ _Left_Frame, _Right_Frame ] = _Pair_range;
                return _Method::mix(
                        std::forward< decltype( _Left_Frame  ) >( _Left_Frame  ),
                        std::forward< decltype( _Right_Frame ) >( _Right_Frame )
                );
        }

};

} // namespace dsp
