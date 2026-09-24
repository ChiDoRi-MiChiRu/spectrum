module;

#include <algorithm>
#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include <mutex>
#include <ranges>
#include <thread>

export module ui.spectrum;

import dsp;
import util.type;
import util.erased_view;

namespace ui {

export
class spectrum {

private:
        using frequency_t = float;

private:
        int m_sample_rate;
        int m_fft_size;

        float m_max_db;
        float m_min_db;
        float m_max_f ;
        float m_min_f ;

        std::size_t m_sample_point_count;

        std::vector< frequency_t > m_linear_frequencies;
        std::vector< frequency_t > m_log_frequencies;

        float norm_db_0_to_1( auto _Db, float _Max, float _Min ) {
                return std::clamp( ( _Db - _Min ) / ( _Max - _Min ), 0.0f, 1.0f );
        }

        auto resample_log_freq( std::size_t _Sample_point, float _Max_f, float _Min_f ) {
                auto log_freq = [ _Sample_point, _Max_f, _Min_f ]( auto _I ) {
                        auto x = static_cast< float >( _I ) / ( _Sample_point - 1 );
                        return _Min_f * std::pow( _Max_f / _Min_f, x );
                };

                m_log_frequencies = std::views::iota( std::size_t{ 0 }, _Sample_point )
                        | std::views::transform( log_freq )
                        | std::ranges::to< std::vector< frequency_t > >();
        }

        auto resample_db( std::size_t _Sample_point_number, const dsp::db_view& _Db_view ) {

                float position = m_log_frequencies[ _Sample_point_number ] * m_fft_size / m_sample_rate;

                int k = std::floor( position );
                float t = position - k;

                float db = std::lerp( _Db_view[ k ].value(), _Db_view[ k + 1 ].value(), t );

                return norm_db_0_to_1( db, m_max_db, m_min_db );

        }

public:
        spectrum( int _Sample_rate, int _Fft_size, std::size_t _Sample_point,
                float _Max_db = 0.0f, float _Min_db = -100.0f, float _Max_f = 22000.0f, float _Min_f = 20.0f )
                : m_sample_rate( _Sample_rate ), m_fft_size( _Fft_size )
                , m_max_db( _Max_db ), m_min_db( _Min_db ), m_max_f( _Max_f ), m_min_f( _Min_f )
                , m_sample_point_count( _Sample_point )
                , m_linear_frequencies( _Fft_size / 2 + 1 ), m_log_frequencies( _Sample_point )
        {
                for ( int i = 0; i < _Fft_size / 2 + 1; i++ ) {
                        m_linear_frequencies[ i ] = static_cast< float >( i ) * _Sample_rate / _Fft_size;
                        // m_norm_level [ i ].store( 0.0f, std::memory_order_relaxed );
                }

                resample_log_freq( _Sample_point, _Max_f, _Min_f );

        }

        util::erased_view< float > db_transeform( dsp::db_view&& _Db_view ) {
                return std::views::iota( std::size_t{ 0 }, m_sample_point_count )
                        | std::views::transform( [ this, view = std::forward< dsp::db_view >( _Db_view ) ]( auto i ) {
                                return this->resample_db( i, view );
                        } );
        }

        util::erased_view< float > db_transeform( const dsp::db_view& _Db_view ) {
                return std::views::iota( std::size_t{ 0 }, m_sample_point_count )
                        | std::views::transform( [ this, &view = _Db_view ]( auto i ) {
                                return this->resample_db( i, view );
                        } );
        }

}; // spectrum_relaxed

} // namespace ui
