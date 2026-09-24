
export module audio;

import audio.type;

export import audio.pcm;
export import audio.format;
export import audio.device;

import audio.ios_base;

export import audio.ios;

import audio.stream_buffer;
import audio.iostream;
import audio.fstream;

export
namespace audio {

template< audio::is_sample_type _Sample_Ty >
using device_stream_buffer = pa_stream_buffer< _Sample_Ty >;

template< audio::is_sample_type _Sample_Ty >
using file_stream_buffer = snd_stream_buffer< _Sample_Ty >;

using istream = basic_istream< float >;
using ostream = basic_ostream< float >;

using ifstream = basic_ifstream< float >;
using ofstream = basic_ofstream< float >;

} // namespace audio



