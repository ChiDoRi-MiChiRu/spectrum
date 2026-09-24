
module;

#include <memory>

export module audio.iostream;

import audio.type;
import audio.pcm;
import audio.format;
import audio.ios_base;

namespace audio {

export
template< audio::is_sample_type _Sample_Ty >
class basic_istream : virtual public basic_ios< _Sample_Ty > {

public:
        using base_type = basic_ios< _Sample_Ty >;
        using sample_type = _Sample_Ty;
        using stream_buffer_type = base_type::stream_buffer_type;

public:
        explicit basic_istream( stream_buffer_type* _Sb ) : base_type(  _Sb ) {}
        explicit basic_istream( stream_buffer_type& _Sb ) : base_type( &_Sb ) {}

        basic_istream& read( const audio::pcm< sample_type >& _Data ) {

                auto* sb = this->stream_buffer();

                if ( !sb ) {
                        this->set_state( audio::ios_base::bad_bit );
                        return *this;
                }

                auto write_result = sb->read( _Data );

                if ( write_result == 0 ) {
                        this->set_state( audio::ios_base::fail_bit );
                        return *this;
                }

                return *this;
        }

        basic_istream& operator>>( const audio::pcm< sample_type >& _Data ) {
                read( _Data );
                return *this;
        }

        basic_istream& operator>>( ios_base& ( *_Func )( ios_base& ) ) {
                _Func( *this );
                return *this;
        }


}; // class basic_istream

export
template< audio::is_sample_type _Sample_Ty >
class basic_ostream : virtual public basic_ios< _Sample_Ty > {

public:
        using base_type = basic_ios< _Sample_Ty >;
        using sample_type = _Sample_Ty;
        using stream_buffer_type = base_type::stream_buffer_type;

public:
        explicit basic_ostream( stream_buffer_type* _Sb ) : base_type(  _Sb ) {}
        explicit basic_ostream( stream_buffer_type& _Sb ) : base_type( &_Sb ) {}

        basic_ostream& write( const audio::pcm< sample_type >& _Data ) {

                auto* sb = this->stream_buffer();

                if ( !sb ) {
                        this->set_state( audio::ios_base::bad_bit );
                        return *this;
                }

                auto write_result = sb->write( _Data );

                if ( write_result == 0 ) {
                        this->set_state( audio::ios_base::fail_bit );
                        return *this;
                }

                return *this;
        }

        basic_ostream& operator<<( const audio::pcm< sample_type >& _Data ) {
                write( _Data );
                return *this;
        }

        basic_ostream& operator<<( ios_base& ( *_Func )( ios_base& ) ) {
                _Func( *this );
                return *this;
        }

}; // class basic_ostream

} // namespace audio

