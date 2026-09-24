
module;

#include <vector>
#include <ranges>

export module audio.pcm;

import audio.format;
import audio.type;
import util.erased_view;
import util.type;

namespace audio {

export
template< audio::is_sample_type _Sample_Ty >
class pcm {

public:
        using value_type = _Sample_Ty;

private:
        std::vector< value_type > m_data;
        std::size_t m_frames		= 0;
        std::size_t m_valid_frames	= 0;
        audio::format m_format;

public:
        pcm( std::size_t _Frames, format _Format )
                : m_data( _Frames * _Format.channel_count() ), m_frames( _Frames ), m_format( _Format ) {}

        pcm( util::erased_view< value_type >&& _View, audio::format _Format )
                : m_data( std::forward< util::erased_view< value_type > >( _View ).to_vector() ), m_format( _Format ) {
                m_frames = m_data.size() / m_format.channel_count();
                m_valid_frames = m_frames;
        }

public:
        template< typename _Range_Ty >
        static pcm from_range( _Range_Ty&& _Range, audio::format _Format ) {

                auto data = std::forward< _Range_Ty >( _Range ) | std::ranges::to< std::vector< value_type > >();

                auto result = pcm( data.size() / _Format.channel_count(), _Format );

                result.m_valid_frames = result.m_frames;
                result.m_data = std::move( data );

                return result;
        }

        template< typename _Pipe_data_Ty >
        static pcm from_pipe( _Pipe_data_Ty&& _Pipe_data ) {

                auto data = std::move( _Pipe_data.data() ) | std::ranges::to< std::vector< value_type > >();
                audio::format format = std::move( _Pipe_data.meta() );

                auto result = pcm( data.size() / format.channel_count(), format );

                result.m_valid_frames = result.m_frames;
                result.m_data = std::move( data );

                return result;
        }

        [[nodiscard]]
        std::size_t frames() const noexcept {
                return m_frames;
        }

        [[nodiscard]]
        std::size_t& frames() noexcept {
                return m_frames;
        }

        [[nodiscard]]
        audio::format format() const noexcept {
                return m_format;
        }

        [[nodiscard]]
        audio::format& format() noexcept {
                return m_format;
        }

        [[nodiscard]]
        std::size_t valid_frames() const noexcept {
                return m_valid_frames;
        }

        [[nodiscard]]
        std::size_t& valid_frames() noexcept {
                return m_valid_frames;
        }

        [[nodiscard]]
        std::vector< value_type >& vector() noexcept {
                return m_data;
        }

        [[nodiscard]]
        const std::vector< value_type >& vector() const noexcept {
                return m_data;
        }

        [[nodiscard]]
        util::erased_view< value_type > view() noexcept {
                return m_data;
        }

}; // class pcm

} // namespace audio
