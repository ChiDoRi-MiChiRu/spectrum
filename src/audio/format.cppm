
module;

#include <cstddef>
#include <stdexcept>

#include <sndfile.h>
#include <portaudio.h>

export module audio.format;

import util.pa_context_manager;

namespace audio {

export
class format {

public:
	enum class channel_layout : int {
		mono   = 0,
		stereo = 1,
	};

private:
	channel_layout m_channels;
	std::size_t m_channel_count;
	std::size_t m_sample_rate;

        constexpr static std::size_t layout_count_map[] = {
                1, // mono
                2, // stereo
        };

private:
	std::size_t to_counts( channel_layout layout ) {
		switch ( layout ) {
		case channel_layout::mono: {
			return 1;
		}
		case channel_layout::stereo: {
			return 2;
		}
		default: {
			throw std::logic_error( "invalid channel layout" );
		}
		}
	}

	channel_layout to_layout( const SF_INFO& _Sf_info ) {
		switch ( _Sf_info.channels ) {
		case 1: {
			return channel_layout::mono;
		}
		case 2: {
			return channel_layout::stereo;
		}

		default: {
			throw std::logic_error( "invalid channel layout" );
		}
		}
	}

	channel_layout to_layout( const PaStreamParameters& _Pa_params ) {
		switch ( _Pa_params.channelCount ) {
		case 1: {
			return channel_layout::mono;
		}
		case 2: {
			return channel_layout::stereo;
		}

		default: {
			throw std::logic_error( "invalid channel layout" );
		}
		}
	}

public:
	format() : m_channels( channel_layout::stereo ), m_channel_count( 2 ), m_sample_rate( 48000 ) {}
	format( channel_layout _Channel_layout, std::size_t _Sample_rate ) :
		m_channels( _Channel_layout ), m_channel_count( to_counts( _Channel_layout ) ), m_sample_rate( _Sample_rate ) {

	}
	explicit format( const SF_INFO& _Sf_info ) :
		m_channels( to_layout( _Sf_info ) ), m_channel_count( _Sf_info.channels ), m_sample_rate( _Sf_info.samplerate ) {

	}

	explicit format( const PaStreamParameters& _Pa_params ) :
		m_channels( to_layout( _Pa_params ) ), m_channel_count( _Pa_params.channelCount ), 
		m_sample_rate( util::pa.call( Pa_GetDeviceInfo, _Pa_params.device )->defaultSampleRate ) {

	}

	bool operator==( const format& ) const noexcept = default;
	bool operator!=( const format& ) const noexcept = default;

	channel_layout channels() const noexcept {
		return m_channels;
	}

	channel_layout& channels() noexcept {
		return m_channels;
	}

	std::size_t channel_count() const noexcept {
		return layout_count_map[ static_cast< int >( m_channels ) ];
	}

	std::size_t sample_rate() const noexcept {
		return m_sample_rate;
	}

	std::size_t& sample_rate() noexcept {
		return m_sample_rate;
	}

};

}
