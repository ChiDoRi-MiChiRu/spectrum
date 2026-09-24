
module;

#include <concepts>

export module util.type;

namespace util {

export
class dsp_operation_base {};

export
template< class _Operation_Ty >
concept is_dsp_operation_type = std::derived_from< std::remove_cvref_t< _Operation_Ty >, dsp_operation_base >;

export
class buffer_base {};

export
template< class _Buffer_Ty >
concept is_buffer_type = std::derived_from< std::remove_cvref_t< _Buffer_Ty >, buffer_base >;

export
class mix_method_base {};

export
template< typename _Method >
concept is_mix_method = std::derived_from< std::remove_cvref_t< _Method >, mix_method_base >;

} // namespace util
