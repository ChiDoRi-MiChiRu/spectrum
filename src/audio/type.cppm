
module;

#include <concepts>

export module audio.type;

namespace audio {

using int_16_t   = short;
using float_32_t = float;

export
template< typename _Ty >
concept is_sample_type =
                std::same_as< _Ty,   int_16_t > ||
                std::same_as< _Ty, float_32_t >;


} // namespace audio
