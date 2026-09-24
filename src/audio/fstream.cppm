
module;

#include <filesystem>

export module audio.fstream;

import audio.type;
import audio.pcm;
import audio.ios_base;
import audio.stream_buffer;

namespace audio {

export
template< audio::is_sample_type _Sample_Ty >
class basic_ifstream : virtual public audio::basic_ios< _Sample_Ty > {

public:
        using base_type = basic_ios< _Sample_Ty >;
        using sample_type = _Sample_Ty;
        using stream_buffer_type = base_type::stream_buffer_type;

private:
        bool m_is_path_constructed = false;

public:
        explicit basic_ifstream( stream_buffer_type& _Buffer ) : base_type{ &_Buffer } {}
        explicit basic_ifstream( stream_buffer_type* _Buffer ) : base_type{  _Buffer } {}

        explicit basic_ifstream( std::filesystem::path _Path ) : base_type{ nullptr }, m_is_path_constructed( true ) {
                auto sb = audio::snd_stream_buffer< _Sample_Ty >::create( _Path );

                if ( sb.has_value() ) {
                        auto* sb_ptr = new audio::snd_stream_buffer< _Sample_Ty >( std::move( sb.value() ) );

                        this->format() = sb_ptr->format();
                        this->stream_buffer( sb_ptr );
                }
                else {
                        this->set_state( audio::ios_base::fail_bit );
                }
        }

        ~basic_ifstream() noexcept {
                if ( m_is_path_constructed ) {
                        delete this->stream_buffer();
                        this->stream_buffer( nullptr );
                }
        }

        bool is_open() const noexcept {

                if ( this->stream_buffer() == nullptr ) {
                        return false;
                }

                return reinterpret_cast< stream_buffer_type* >( this->stream_buffer() )->is_open();
        }

        basic_ifstream& read( audio::pcm< sample_type >& _Buffer ) {

                auto* sb = this->stream_buffer();

                if ( !sb ) {
                        this->set_state( audio::ios_base::bad_bit );
                        return *this;
                }

                if ( sb->is_end() ) {
                        this->set_state( audio::ios_base::eof_bit );
                        return *this;
                }

                auto read_result = sb->read( _Buffer );

                // if ( read_result == 0 ) {
                //         this->set_state( audio::ios_base::eof_bit | audio::ios_base::fail_bit );
                //         return *this;
                // }

                return *this;
        }

        basic_ifstream& operator>>( audio::pcm< sample_type >& _Buffer ) {
                read( _Buffer );
                return *this;
        }

}; // class basic_ifstream

export
template< audio::is_sample_type _Sample_Ty >
class basic_ofstream : virtual public audio::basic_ios< _Sample_Ty > {

public:
        using base_type = basic_ios< _Sample_Ty >;
        using sample_type = _Sample_Ty;
        using stream_buffer_type = base_type::stream_buffer_type;

private:
        bool m_is_path_constructed = false;

public:
        explicit basic_ofstream( stream_buffer_type& _Buffer ) : base_type{ &_Buffer } {}
        explicit basic_ofstream( stream_buffer_type* _Buffer ) : base_type{  _Buffer } {}

        explicit basic_ofstream( std::filesystem::path _Path ) : base_type{ nullptr }, m_is_path_constructed( true ) {
                auto sb = audio::snd_stream_buffer< _Sample_Ty >::create( _Path );

                if ( sb.has_value() ) {
                        auto* sb_ptr = new audio::snd_stream_buffer< _Sample_Ty >( std::move( sb.value() ) );
                        this->stream_buffer( sb_ptr );
                }
                else {
                        this->set_state( audio::ios_base::fail_bit );
                }
        }

        ~basic_ofstream() noexcept {
                if ( m_is_path_constructed ) {
                        delete this->stream_buffer();
                        this->stream_buffer( nullptr );
                }
        }

        basic_ofstream& write( const audio::pcm< sample_type >& _Buffer ) {

                auto* sb = this->stream_buffer();

                if ( !sb ) {
                        this->set_state( audio::ios_base::bad_bit );
                        return *this;
                }

                auto write_result = sb->write( _Buffer );

                if ( write_result == 0 ) {
                        this->set_state( audio::ios_base::fail_bit );
                        return *this;
                }

                return *this;
        }

        basic_ofstream& operator<<( const audio::pcm< sample_type >& _Buffer ) {
                write( _Buffer );
                return *this;
        }

}; // class basic_ofstream

} // namespace audio
