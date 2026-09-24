#include <complex>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <thread>
#include <vector>
#include <mutex>

import audio;
import dsp;
import ui;
import util;

static void wait_for_input() {
        std::string line;
        std::getline(std::cin, line);
}

static std::filesystem::path input_song_file_path() {
        std::string path_str;

        std::cout << "Input a song path: " << std::endl;
        std::getline( std::cin, path_str );

        if ( path_str.front() == '\"' && path_str.back() == '\"' ) {
                path_str = std::string( path_str.begin() + 1, path_str.end() - 1 );
        }

        if ( path_str.front() == '\'' && path_str.back() == '\'' ) {
                path_str = std::string( path_str.begin() + 1, path_str.end() - 1 );
        }

        return std::filesystem::path( path_str );
}

int main() {

        std::filesystem::path file_path = input_song_file_path();
        auto file_in = audio::ifstream( file_path );
        if ( !file_in.is_open() ) {
                std::cerr << std::format( "File( path: {} ) can't open.", file_path.c_str() ) << std::endl;

                wait_for_input();
                return 1;
        }

        auto default_device_sb = audio::device_stream_buffer< float >::create( audio::device::default_output(), file_in.format(), 1024 );
        if ( !default_device_sb.has_value() ) {
                std::cerr << std::format( "Stream buffer can't create: {}.", util::pa.error_text( default_device_sb.error() ) ) << std::endl;

                wait_for_input();
                return 1;
        }

        auto out = audio::ostream( default_device_sb.value() );

	auto window_size = 8192;
	auto samples_per_frame = 2048;
	auto hop_size = 128;
        auto fft_size = 8192 * 2;

        auto buffer = audio::pcm< float >( samples_per_frame, file_in.format() );

        auto framing_for_left  = dsp::frame_stream( window_size, hop_size );
        auto framing_for_right = dsp::frame_stream( window_size, hop_size );

        std::vector< std::vector< float > > framing_buffer_left ;
        std::vector< std::vector< float > > framing_buffer_right;

        auto window_fft = [ window_size, fft_size ] { return dsp::for_each_frame( dsp::window::hanning( window_size ) | dsp::fft( fft_size ) ); };
        auto stereo_fft_to_db = dsp::for_each_frame( dsp::mix_by< dsp::rms >() | dsp::fft::normalize( fft_size ) | dsp::to_db() );

        auto bin_count = 200, height = 40;
        auto attack = 0.4f, release = 0.75f;
        auto renderer = ui::renderer( height, bin_count, attack, release );
        auto spectrum = ui::spectrum( file_in.format().sample_rate(), fft_size, bin_count );

        dsp::db_view sp_frame;
        dsp::db_view for_render;
        std::mutex frame_mutex;
        auto updater = util::updater< dsp::db_view >(
                [ & ]( std::optional< dsp::db_view > _Data ) {
                        if ( _Data.has_value() ) {

                                std::lock_guard lock{ frame_mutex };
                                sp_frame = std::move( _Data.value() );

                        }
                },
                file_in.format().sample_rate(), hop_size
        );

        int fps = 120;
        auto ui_frame_interval = std::chrono::milliseconds( static_cast< int >( 1000 / static_cast< float >( fps ) ) );
        auto terminal_ticker = util::ticker(
                [ & ] {
                        {

                                std::lock_guard lock{ frame_mutex };

                                if ( sp_frame.has_view() ) {
                                        for_render = std::move( sp_frame );
                                }

                        }

                        if ( for_render.has_view() ) {
                                renderer.render( spectrum.db_transeform( for_render ) );
                        }

                },
                ui_frame_interval
        );

        updater.start();
        terminal_ticker.start();

        for ( ; !file_in.is_eof(); ) {

                file_in >> buffer;

                const auto& samples = buffer.vector();

                framing_for_left  << ( samples | dsp::split_channels( dsp::stereo::left  ) );
                framing_for_right << ( samples | dsp::split_channels( dsp::stereo::right ) );

                framing_for_left  >> framing_buffer_left ;
                framing_for_right >> framing_buffer_right;

                auto dbs_view = dsp::zip_frame(
                        framing_buffer_left  | window_fft(),
                        framing_buffer_right | window_fft()
                ) | stereo_fft_to_db;

                for ( auto frame_dbs_view : dbs_view ) {

                        updater.push( std::move( frame_dbs_view ) );

                }

                out << ( buffer | dsp::gain( dsp::db{ -12.0 } ) | dsp::to< audio::pcm >() );

        }

        terminal_ticker.stop();
        updater.stop();

	std::cout << "Done" << std::endl;
        int a; std::cin >> a;

}
