
module;

#include <portaudio.h>

export module audio.device;

import util.pa_context_manager;

namespace audio {

export
class device {

private:
	int m_index = paNoDevice;

public:
        explicit device( int _Index ) {
                m_index = _Index;
        }

        static device default_input() {
                return device( util::pa.call( Pa_GetDefaultInputDevice ) );
        }

        static device default_output() {
                return device( util::pa.call( Pa_GetDefaultOutputDevice ) );
        }

	~device() noexcept = default;

        int index() const noexcept {
	        return m_index;
	}

        const PaDeviceInfo* info() const noexcept {
	        return util::pa.call( Pa_GetDeviceInfo, m_index );
	}

        bool is_valid() const noexcept {
	        return m_index != paNoDevice && this->info() != nullptr;
	}

        bool is_input() const noexcept {
                return this->info()->maxInputChannels > 0;
        }

        bool is_output() const noexcept {
                return this->info()->maxOutputChannels > 0;
        }

        const char* name() const noexcept {
	        return this->info()->name;
	}

};

export
int get_device_count() {
        return util::pa.call( Pa_GetDeviceCount );
}

}

