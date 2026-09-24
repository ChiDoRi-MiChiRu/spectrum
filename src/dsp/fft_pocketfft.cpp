
module;

#include <pocketfft_hdronly.h>

module dsp.fft;

std::vector< std::complex< float > > dsp::fft::apply_to( util::erased_view< float >&& _Range ) {

        auto input = padding_zero( std::forward< util::erased_view< float > >( _Range ) );
        auto output = std::vector< std::complex< float > >( m_output_size );

        pocketfft::r2c(
                pocketfft::shape_t{ m_input_size },
                pocketfft::stride_t{ sizeof( float ) },
                pocketfft::stride_t{ sizeof( std::complex< float > ) },
                std::size_t{ 0 },
                pocketfft::FORWARD,
                input.data(),
                output.data(),
                1.0f
        );

        return output;

}
