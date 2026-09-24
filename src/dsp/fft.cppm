
module;

#include <ranges>
#include <complex>
#include <pocketfft_hdronly.h>

export module dsp.fft;

import util;

namespace dsp {

export
class fft : public util::dsp_operation_base {

private:
        std::size_t m_input_size;
        std::size_t m_output_size;

        class normalization : public util::dsp_operation_base {

        private:
                std::size_t m_size;

        public:
                explicit normalization( std::size_t _Size ) : m_size( _Size ) {}

                template< std::ranges::range _Range_Ty >
                auto apply_to( _Range_Ty&& _Range ) {
                        return std::forward< _Range_Ty >( _Range ) | std::views::transform(
                                [ _Size = m_size ]( auto _A ) { return _A / _Size; }
                        );
                }

        };

        normalization m_norm;

        std::vector< float > padding_zero( util::erased_view< float >&& _Range ) const {

                if ( _Range.size() > m_input_size ) {
                        throw std::invalid_argument{ "Input size exceeds FFT size" };
                }

                auto vec = _Range.to_vector();
                vec.insert( vec.end(), m_input_size - vec.size(), 0.0f );

                return vec;
        }

public:
        fft() = delete;
        explicit fft( std::size_t _Input_size )
                : m_input_size( _Input_size ), m_output_size( _Input_size / 2 + 1 ), m_norm( _Input_size ) {}

        // template< std::ranges::range _Range_Ty >
        // auto apply_to( _Range_Ty&& _Range ) {
        //
        //         auto input = std::forward< _Range_Ty >( _Range ) | std::ranges::to< std::vector >();
        //         auto output = std::vector< std::complex< float > >( m_output_size );
        //
        //         pocketfft::r2c(
        //                 m_shape,
        //                 m_stride_in,
        //                 m_stride_out,
        //                 { 0 },
        //                 pocketfft::FORWARD,
        //                 input.data(),
        //                 output.data(),
        //                 1.0f
        //         );
        //
        //         return output;
        //
        // }

        std::vector< std::complex< float > > apply_to( util::erased_view< float >&& _Range );

        static auto normalize( std::size_t _Fft_size ) {
                return normalization( _Fft_size );
        }

};

}
