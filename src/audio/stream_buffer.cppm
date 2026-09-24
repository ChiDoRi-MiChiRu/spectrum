
module;

#include <expected>
#include <format>
#include <vector>
#include <filesystem>
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>

export module audio.stream_buffer;

import audio.type;
import audio.format;
import audio.device;
import audio.pcm;
import util.pa_context_manager;
import util.erased_view;

namespace audio {

export
template< audio::is_sample_type _Sample_Ty >
class basic_stream_buffer {

public:
        using sample_type = _Sample_Ty;

public:
        virtual ~basic_stream_buffer() = default;

        virtual std::size_t write( const audio::pcm< sample_type >& ) = 0;
        virtual std::size_t read ( audio::pcm< sample_type >& ) = 0;
        virtual bool is_open() const noexcept = 0;
        virtual bool is_end()  const noexcept = 0;

}; // class stream_buffer

template< audio::is_sample_type >
struct portaudio_traits;

template<>
struct portaudio_traits< float > {
        static constexpr PaSampleFormat sample_format = paFloat32;
};

template<>
struct portaudio_traits< std::int16_t > {
        static constexpr PaSampleFormat sample_format = paInt16;
};

export
template< audio::is_sample_type _Sample_Ty, typename _Traits = portaudio_traits< _Sample_Ty > >
class pa_stream_buffer : public basic_stream_buffer< _Sample_Ty > {

public:
        using sample_type = _Sample_Ty;
        using traits_type = _Traits;

private:
        constexpr static unsigned int input_bit  = 1u;
        constexpr static unsigned int output_bit = 2u;

private:
        audio::device m_device;
        PaStreamParameters m_iparam{};
        PaStreamParameters m_oparam{};
        PaStream* m_stream = nullptr;
        bool m_is_istream_buffer = false;
        bool m_is_ostream_buffer = false;

private:
        pa_stream_buffer( audio::device _Device, PaStreamParameters _Iparams, PaStreamParameters _Oparams, PaStream* _Stream, unsigned int _Flag )
                : m_device( _Device ), m_iparam( _Iparams ), m_oparam( _Oparams ), m_stream( _Stream ) {
                m_is_istream_buffer = ( _Flag & input_bit  ) != 0;
                m_is_ostream_buffer = ( _Flag & output_bit ) != 0;
        }

public:
        static std::expected< pa_stream_buffer, PaError > create( audio::device _Device, audio::format _Format, std::size_t _Frame_per_buffer ) {

                if ( !_Device.is_valid() ) {
                        return std::unexpected( paInvalidDevice );
                }

                PaStreamParameters* ipptr = nullptr;
                PaStreamParameters* opptr = nullptr;

                PaStreamParameters ip{};
                PaStreamParameters op{};

                if ( _Device.is_input() ) {
                        ip = PaStreamParameters{
                                .device = _Device.index(),
                                .channelCount = static_cast< int >( _Format.channel_count() ),
                                .sampleFormat = traits_type::sample_format,
                                .suggestedLatency = util::pa.call( Pa_GetDeviceInfo, _Device.index() )->defaultLowInputLatency,
                                .hostApiSpecificStreamInfo = nullptr,
                        };
                        ipptr = &ip;
                }

                if ( _Device.is_output() ) {
                        op = PaStreamParameters{
                                .device = _Device.index(),
                                .channelCount = static_cast< int >( _Format.channel_count() ),
                                .sampleFormat = traits_type::sample_format,
                                .suggestedLatency = util::pa.call( Pa_GetDeviceInfo, _Device.index() )->defaultLowOutputLatency,
                                .hostApiSpecificStreamInfo = nullptr,
                        };
                        opptr = &op;
                }

                double sample_rate = static_cast< double >( _Format.sample_rate() );

                if ( util::pa.call( Pa_IsFormatSupported, ipptr, opptr, sample_rate ) != paFormatIsSupported ) {

                        sample_rate = _Device.info()->defaultSampleRate;

                        auto error = util::pa.call( Pa_IsFormatSupported, ipptr, opptr, sample_rate );
                        if ( error != paFormatIsSupported ) {
                                return std::unexpected( error );
                        }

                }

                PaStream* stream = nullptr;

                auto error = util::pa.call( Pa_OpenStream,
                        &stream,
                        ipptr,
                        opptr,
                        sample_rate,
                        _Frame_per_buffer,
                        paNoFlag,
                        nullptr,
                        nullptr
                );

                if ( error != paNoError ) {
                        return std::unexpected( error );
                }

                error = util::pa.call( Pa_StartStream, stream );

                if ( error != paNoError ) {
                        if ( stream != nullptr ) {
                                util::pa.call( Pa_StopStream , stream );
                                util::pa.call( Pa_CloseStream, stream );
                        }
                        return std::unexpected( error );
                }

                unsigned int flag = ( ipptr != nullptr ? input_bit : 0u ) | ( opptr != nullptr ? output_bit : 0u );

                return pa_stream_buffer( _Device, ip, op, stream, flag );

        }

        pa_stream_buffer( const pa_stream_buffer& ) = delete;
        pa_stream_buffer& operator=( const pa_stream_buffer& ) = delete;

        pa_stream_buffer( pa_stream_buffer&& _Other ) noexcept 
                : m_device( _Other.m_device )
                , m_iparam( _Other.m_iparam )
                , m_oparam( _Other.m_oparam )
                , m_stream( std::exchange( _Other.m_stream, nullptr ) )
                , m_is_istream_buffer( _Other.m_is_istream_buffer )
                , m_is_ostream_buffer( _Other.m_is_ostream_buffer ) {}

        pa_stream_buffer& operator=( pa_stream_buffer&& _Other ) noexcept {
                if ( this == &_Other ) {
                        return *this;
                }

                if ( m_stream != nullptr ) {
                        util::pa.call( Pa_StopStream , m_stream );
                        util::pa.call( Pa_CloseStream, m_stream );
                }

                m_device = _Other.m_device;
                m_iparam = _Other.m_iparam;
                m_oparam = _Other.m_oparam;
                m_stream = std::exchange( _Other.m_stream, nullptr );

                m_is_istream_buffer = _Other.m_is_istream_buffer;
                m_is_ostream_buffer = _Other.m_is_ostream_buffer;

                return *this;
        }

        ~pa_stream_buffer() noexcept override {
                if ( m_stream != nullptr ) {
                        util::pa.call( Pa_StopStream , m_stream );
                        util::pa.call( Pa_CloseStream, m_stream );
                }
        }

        std::size_t write( const audio::pcm< sample_type >& _Buffer ) override {

                if ( !m_is_ostream_buffer ) {
                        throw std::logic_error( "This stream buffer unsupported to output" );
                }

                util::pa.call( Pa_WriteStream,
                        m_stream,
                        _Buffer.vector().data(),
                        _Buffer.valid_frames()
                );

                return _Buffer.valid_frames();
        }

        std::size_t read( audio::pcm< sample_type >& _Buffer ) override {

                if ( !m_is_istream_buffer ) {
                        throw std::logic_error( "This stream buffer unsupported to input" );
                }

                util::pa.call( Pa_ReadStream,
                        m_stream,
                        _Buffer.vector().data(),
                        _Buffer.frames()
                );

                _Buffer.valid_frames() = _Buffer.frames();

                return _Buffer.valid_frames();
        }

        bool is_open() const noexcept override {
                return m_stream != nullptr;
        }

        bool is_end() const noexcept override {
                return false;
        }

}; // class pa_stream_buffer

// export
// template< audio::is_sample_type _Sample_Ty >
// class basic_file_stream_buffer : public basic_stream_buffer< _Sample_Ty > {
//
// public:
//         virtual bool is_end() const = 0;
//
// }; // class basic_file_stream_buffer

template< audio::is_sample_type _Sample_Ty >
struct snd_traits{};

template<>
struct snd_traits< float > {
        static constexpr auto read_func  = sf_readf_float;
        static constexpr auto write_func = sf_writef_float;
};

export
template< audio::is_sample_type _Sample_Ty, typename _Traits = snd_traits< _Sample_Ty > >
class snd_stream_buffer : public basic_stream_buffer< _Sample_Ty > {

public:
        using sample_type = _Sample_Ty;
        using traits_type = _Traits;

private:
        SNDFILE* m_file = nullptr;
        SF_INFO m_info{};
        audio::format m_format;
        sf_count_t m_read_count = 0;

private:
        snd_stream_buffer( const char* _Path ) {
                m_file = sf_open( _Path , SFM_READ, &m_info );
        }

#ifdef _WIN32
        snd_stream_buffer( const wchar_t* _Path ) {
                m_file = sf_wchar_open( _Path , SFM_READ, &m_info );
        }
#endif

public:
        static std::expected< snd_stream_buffer, std::exception > create( std::filesystem::path _Path ) {

                if constexpr ( std::is_same_v< std::filesystem::path::value_type, char > ) {
                        return snd_stream_buffer( _Path.c_str() );
                }
                else if constexpr ( std::is_same_v< std::filesystem::path::value_type, wchar_t > ) {
#ifdef _WIN32
                        return snd_stream_buffer( _Path.c_str() );
#endif
                }
                else {
                        static_assert(
                                std::is_same_v< std::filesystem::path::value_type, char > ||
                                std::is_same_v< std::filesystem::path::value_type, wchar_t >,
                                "No matching file path encoding found"
                        );
                }

                return std::unexpected( std::runtime_error( "" ) );
        }

        snd_stream_buffer( const snd_stream_buffer& ) = delete;
        snd_stream_buffer& operator=( const snd_stream_buffer& ) = delete;

        snd_stream_buffer( snd_stream_buffer&& _Other ) noexcept
                : m_file( std::exchange( _Other.m_file, nullptr ) )
                , m_info( _Other.m_info )
                , m_format( _Other.m_format )
                , m_read_count( _Other.m_read_count ) {}

        snd_stream_buffer& operator=( snd_stream_buffer&& _Other ) noexcept {
                if ( this == &_Other ) {
                        return *this;
                }

                if ( m_file ) {
                        sf_close( m_file );
                }

                m_file = std::exchange( _Other.m_file, nullptr );
                m_info = _Other.m_info;
                m_format = _Other.m_format;
                m_read_count = _Other.m_read_count;

                return *this;
        }

        ~snd_stream_buffer() noexcept override {
                if ( m_file != nullptr ) {
                        sf_close( m_file );
                }
        }

        bool is_end() const noexcept override {
                return m_read_count >= m_info.frames;
        }

        bool is_open() const noexcept override {
                return m_file != nullptr;
        }

        std::size_t read( audio::pcm< sample_type >& _Buffer ) override {

                auto read = traits_type::read_func(
                        m_file,
                        _Buffer.vector().data(),
                        _Buffer.frames()
                );

                m_read_count += read;
                _Buffer.valid_frames() = read;

                return read;
        }

        std::size_t write( const audio::pcm< sample_type >& _Buffer ) override {

                auto last_write_frames = traits_type::write_func(
                        m_file,
                        _Buffer.vector().data(),
                        _Buffer.valid_frames()
                );

                return last_write_frames;
        }

        audio::format format() const noexcept {
                return audio::format( m_info );
        }

}; // class snd_stream_buffer

} // namespace audio
