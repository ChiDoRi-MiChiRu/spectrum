
module;

#include <ranges>
#include <cmath>

export module dsp.gain;

import util.type;
import dsp.units;

namespace dsp {

export
class gain : public util::dsp_operation_base {

private:
        double m_gain_ratio;

public:
        gain() = delete;
        explicit gain( dsp::db _Db ) : m_gain_ratio( std::pow( 10.0f, _Db.value() / 20.0f ) ) {}

        template< std::ranges::range _Range_Ty >
        auto apply_to( _Range_Ty&& _Samples ) & {
                return
                        std::forward< _Range_Ty >( _Samples )
                        | std::views::transform( [ &gain = m_gain_ratio ]( auto _Sample ) -> std::ranges::range_value_t< _Range_Ty > {
                                return _Sample * gain;
                        } );
        }

        template< std::ranges::range _Range_Ty >
        auto apply_to( _Range_Ty&& _Samples ) && {
                return
                        std::forward< _Range_Ty >( _Samples )
                        | std::views::transform( [ gain = m_gain_ratio ]( auto _Sample ) -> std::ranges::range_value_t< _Range_Ty > {
                                return _Sample * gain;
                        } );
        }

        template< typename _Meta_Ty >
        auto meta( _Meta_Ty&& _Meta ) {
                return std::forward< _Meta_Ty >( _Meta );
        }

};

}
