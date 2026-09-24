
module;

#include <cmath>
#include <complex>
#include <ranges>

#include "pocketfft_hdronly.h"

export module dsp.units;

import util.type;
import util.erased_view;

namespace dsp {

export
class db {
private:
        float m_value;

public:
        constexpr db() = delete;
        constexpr explicit db( float _Value ) : m_value( _Value ) {}

        constexpr float value() const noexcept {
                return m_value;
        }

};

export
using db_view = util::erased_view< dsp::db >;

export
class to_db : public util::dsp_operation_base {

private:
        inline static auto amp_to_db = []( float _Amplitude ) -> dsp::db {
                if ( _Amplitude <= 0.0f ) {
                        return dsp::db{ -std::numeric_limits< float >::infinity() };
                }

                return dsp::db{ 20.0f * std::log10( _Amplitude ) };
        };

public:
        template< std::ranges::range _Range_Ty >
        auto apply_to( _Range_Ty&& _Range ) const {
                return std::forward< _Range_Ty >( _Range ) | std::views::transform( amp_to_db );
        }

};

}
