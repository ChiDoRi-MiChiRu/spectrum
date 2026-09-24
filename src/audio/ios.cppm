

export module audio.ios;

import audio.ios_base;
import audio.format;

export
namespace audio::ios {

audio::ios_base& stereo( audio::ios_base& _Stream ) {
        _Stream.format().channels() = audio::format::channel_layout::stereo;
        return _Stream;
}

audio::ios_base& mono( audio::ios_base& _Stream ) {
        _Stream.format().channels() = audio::format::channel_layout::mono;
        return _Stream;
}

} // namespace audio::ios
