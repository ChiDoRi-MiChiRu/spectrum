
module;

#include <ranges>

export module dsp.pipe;

import audio.pcm;
import audio.format;
import util.type;
import util.erased_view;

namespace dsp {

export
template< typename _Value_Ty, typename _Metainfo_Ty >
class pipe_view {

private:
        util::erased_view< _Value_Ty > m_data;
        _Metainfo_Ty m_meta;

public:
        constexpr pipe_view( const util::erased_view< _Value_Ty >& _Data, _Metainfo_Ty&& _Meta )
                : m_data( _Data ), m_meta( std::forward< _Metainfo_Ty >( _Meta ) ) {}

        constexpr pipe_view( util::erased_view< _Value_Ty >&& _Data, _Metainfo_Ty&& _Meta )
                : m_data( std::forward< util::erased_view< _Value_Ty > >( _Data ) ), m_meta( std::forward< _Metainfo_Ty >( _Meta ) ) {}

        constexpr explicit pipe_view( const audio::pcm< _Value_Ty >& _Pcm )
                : m_data( _Pcm.vector() ), m_meta( _Pcm.format() ) {}

        constexpr explicit pipe_view( audio::pcm< _Value_Ty >&& _Pcm )
                : m_data( std::move( _Pcm.vector() ) ), m_meta( std::move( _Pcm.format() ) ) {}

        template< std::ranges::range _Range_Ty >
        constexpr pipe_view( _Range_Ty&& _Range, _Metainfo_Ty&& _Meta )
                : m_data( std::forward< _Range_Ty >( _Range ) ), m_meta( std::forward< _Metainfo_Ty >( _Meta ) ) {}

        constexpr util::erased_view< _Value_Ty >& data() noexcept {
                return m_data;
        }

        constexpr const util::erased_view< _Value_Ty >& data() const noexcept {
                return m_data;
        }

        constexpr _Metainfo_Ty& meta() noexcept {
                return m_meta;
        }

        constexpr const _Metainfo_Ty& meta() const noexcept {
                return m_meta;
        }

        template< util::is_dsp_operation_type _Operation_Ty >
        auto operator|( _Operation_Ty&& _Operation ) & {
                auto new_meta = _Operation.meta( m_meta );
                return dsp::pipe_view(
                        util::erased_view( std::forward< _Operation_Ty >( _Operation ).apply_to( m_data ) ),
                        new_meta
                );
        }

        template< util::is_dsp_operation_type _Operation_Ty >
        auto operator|( _Operation_Ty&& _Operation ) && {
                auto new_meta = _Operation.meta( std::forward< _Metainfo_Ty >( m_meta ) );
                return dsp::pipe_view(
                        util::erased_view(
                                std::forward< _Operation_Ty >( _Operation ).apply_to( std::forward< util::erased_view< _Value_Ty > >( m_data ) )
                        ),
                        new_meta
                );
        }

};

template< typename _Value_Ty, typename _Metainfo_Ty >
pipe_view( const util::erased_view< _Value_Ty >&, _Metainfo_Ty&& ) -> pipe_view< _Value_Ty, _Metainfo_Ty >;

template< typename _Value_Ty, typename _Metainfo_Ty >
pipe_view( util::erased_view< _Value_Ty >&&, _Metainfo_Ty&& ) -> pipe_view< _Value_Ty, _Metainfo_Ty >;

template< typename _Value_Ty >
pipe_view( const audio::pcm< _Value_Ty >& ) -> pipe_view< _Value_Ty, audio::format >;

template< typename _Value_Ty >
pipe_view( audio::pcm< _Value_Ty >&& ) -> pipe_view< _Value_Ty, audio::format >;

template< std::ranges::range _Range_Ty, typename _Metainfo_Ty >
pipe_view( _Range_Ty&&, _Metainfo_Ty&& ) -> pipe_view< std::ranges::range_value_t< _Range_Ty >, _Metainfo_Ty >;

export
template< typename _Left, typename _Right >
class composed_operation : public util::dsp_operation_base {
        _Left m_left;
        _Right m_right;

public:
        composed_operation( _Left&& _Left_op, _Right&& _Right_op ) :
                m_left ( std::forward< _Left  >( _Left_op  ) ), m_right( std::forward< _Right >( _Right_op ) ) {}

        // template< std::ranges::range _Range_Ty >
        // auto apply_to( _Range_Ty&& _Range ) {
        //         return std::forward< _Range_Ty >( _Range  )
        //                 | std::forward< _Left  >( m_left  )
        //                 | std::forward< _Right >( m_right );
        // }

        template< typename _Range_Ty >
        auto apply_to( _Range_Ty&& _Range ) && {
                return std::forward< _Right >( m_right ).apply_to(
                       std::forward< _Left  >( m_left  ).apply_to(
                                std::forward< _Range_Ty >( _Range )
                       )
                );
        }

        template< typename _Range_Ty >
        auto apply_to( _Range_Ty&& _Range ) & {
                return m_right.apply_to(
                        m_left.apply_to(
                                std::forward< _Range_Ty >( _Range )
                        )
                );
        }

};

export
template< template< typename > typename _Target_Ty >
class to_closure {
public:
        // template< typename _Range_Ty >
        // auto apply_to( _Range_Ty&& _Range ) {
        //
        //         if constexpr ( std::ranges::range< _Range_Ty > ) {
        //                 using value_type = std::ranges::range_value_t< _Range_Ty >;
        //                 return _Target_Ty< value_type >::from_range( std::forward< _Range_Ty >( _Range ) );
        //         }
        //         else {
        //                 using value_type = _Range_Ty::value_type;
        //                 return _Target_Ty< value_type >::from_range( std::forward< _Range_Ty >( _Range ) );
        //         }
        // }

        template< typename _Value_Ty, typename _Metainfo_Ty >
        auto apply_to( dsp::pipe_view< _Value_Ty, _Metainfo_Ty >&& _Pipe_view ) {
                using pipe_view_t = dsp::pipe_view< _Value_Ty, _Metainfo_Ty >;
                return _Target_Ty< _Value_Ty >::from_pipe( std::forward< pipe_view_t >( _Pipe_view ) );
        }

};

export
template< template< typename > typename _Target_Ty, typename _Value_Ty, typename _Metainfo_Ty >
auto operator|( dsp::pipe_view< _Value_Ty, _Metainfo_Ty >&& _Pipe_view, dsp::to_closure< _Target_Ty > _To ) {
        using pipe_view_t = dsp::pipe_view< _Value_Ty, _Metainfo_Ty >;
        return _To.apply_to( std::forward< pipe_view_t >( _Pipe_view ) );
}

export
template< template< typename > typename _Target_Ty >
auto to() {
        return to_closure< _Target_Ty >{};
}

// export
// template< typename _Range_Ty, util::is_dsp_operation_type _Operation_Ty >
//         requires(
//                 util::is_buffer_type< _Range_Ty >
//         )
// auto operator|( _Range_Ty&& _Range, _Operation_Ty&& _Operation ) {
//         return std::forward< _Operation_Ty >( _Operation ).apply_to( std::forward< _Range_Ty >( _Range ).samples() );
// }

template< typename _Ty >
struct is_audio_pcm : std::false_type {};

template< typename _Sample_Ty >
struct is_audio_pcm< audio::pcm< _Sample_Ty > > : std::true_type {};

template< typename _Ty >
concept audio_pcm_type = is_audio_pcm< std::remove_cvref_t< _Ty > >::value;

export
template< typename _Range_Ty, util::is_dsp_operation_type _Operation_Ty >
        requires(
                std::ranges::range< _Range_Ty >
                && !util::is_buffer_type< _Range_Ty >
                && !std::same_as< util::erased_view< std::ranges::range_value_t< _Range_Ty > >, _Range_Ty >
                && !audio_pcm_type< _Range_Ty >
        )
auto operator|( _Range_Ty&& _Range, _Operation_Ty&& _Operation ) {
        return std::forward< _Operation_Ty >( _Operation ).apply_to( std::forward< _Range_Ty >( _Range ) );
}

export
template< typename _Value_Ty, util::is_dsp_operation_type _Operation_Ty >
auto operator|( const audio::pcm< _Value_Ty >& _Pcm, _Operation_Ty&& _Operation ) {
        return dsp::pipe_view(
                _Pcm.vector() | std::forward< _Operation_Ty >( _Operation ),
                _Pcm.format()
        );
}

export
template< typename _Value_Ty, util::is_dsp_operation_type _Operation_Ty >
auto operator|( audio::pcm< _Value_Ty >&& _Pcm, _Operation_Ty&& _Operation ) {
        auto pv = dsp::pipe_view( std::forward< audio::pcm< _Value_Ty > >( _Pcm ) );
        return std::move( pv ) | std::forward< _Operation_Ty >( _Operation );
}

export
template< typename _Value_Ty, util::is_dsp_operation_type _Operation_Ty >
auto operator|( const util::erased_view< _Value_Ty >& _Range, _Operation_Ty&& _Operation ) {
        return util::erased_view(
                std::forward< _Operation_Ty >( _Operation ).apply_to( _Range )
        );
}

export
template< typename _Value_Ty, util::is_dsp_operation_type _Operation_Ty >
auto operator|( util::erased_view< _Value_Ty >&& _Range, _Operation_Ty&& _Operation ) {
        return util::erased_view(
                std::forward< _Operation_Ty >( _Operation ).apply_to( std::forward< util::erased_view< _Value_Ty > >( _Range ) )
        );
}

export
template< util::is_dsp_operation_type _Left, util::is_dsp_operation_type _Right >
auto operator|( _Left&& _Left_op, _Right&& _Right_op ) {
        return composed_operation< _Left, _Right >(
                std::forward< _Left >( _Left_op ), std::forward< _Right >( _Right_op )
        );
}

} // namespace dsp
