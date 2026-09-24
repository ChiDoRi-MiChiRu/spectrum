

export module audio.ios_base;

import audio.format;
import audio.type;
import audio.stream_buffer;

namespace audio {

export
class ios_base {

public:
        using io_state = unsigned int;

public:
        static constexpr io_state good_bit = 0x0;
        static constexpr io_state bad_bit  = 0x1 << 0;
        static constexpr io_state eof_bit  = 0x1 << 1;
        static constexpr io_state fail_bit = 0x1 << 2;

private:
        io_state m_state{ good_bit };
        audio::format m_format{};

public:

        bool is_good() const noexcept {
                return m_state & good_bit;
        }

        bool is_bad() const noexcept {
                return m_state & bad_bit;
        }

        bool is_eof() const noexcept {
                return m_state & eof_bit;
        }

        bool is_fail() const noexcept {
                return m_state & fail_bit;
        }

        void clear( io_state _State = good_bit ) noexcept {
                m_state = _State;
        }

        void set_state( io_state _State ) noexcept {
                m_state |= _State;
        }

        audio::format format() const noexcept {
                return m_format;
        }

        audio::format& format() noexcept {
                return m_format;
        }

}; // class ios_base

export
template< audio::is_sample_type _Sample_Ty >
class basic_ios : public ios_base {
public:
        using stream_buffer_type = audio::basic_stream_buffer< _Sample_Ty >;

private:
        stream_buffer_type* m_stream_buffer;

public:
        explicit basic_ios( stream_buffer_type* _Sb ) : m_stream_buffer{ _Sb } {

        }

        stream_buffer_type* stream_buffer() const noexcept {
                return m_stream_buffer;
        }

        stream_buffer_type* stream_buffer( stream_buffer_type* _Sb ) noexcept {
                auto* before = m_stream_buffer;
                m_stream_buffer = _Sb;
                return before;
        }

}; // class basic_ios

} // namespace audio

