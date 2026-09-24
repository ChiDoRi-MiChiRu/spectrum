
module;

#include <algorithm>
#include <cassert>
#include <span>
#include <ranges>
#include <vector>

export module dsp.frame;

import util;

namespace dsp {

export
class frame_stream {

private:
        std::vector< float > m_buffer;

        std::size_t m_frame_size;
        std::size_t m_hop_size;

        std::size_t m_offset{ 0 };

public:
        frame_stream( std::size_t _Frame_size, std::size_t _Hop_size )
                : m_frame_size( _Frame_size ), m_hop_size( _Hop_size ) {

                assert( _Frame_size > 0 );
                assert( _Hop_size > 0 );
                assert( _Hop_size <= _Frame_size );

                m_buffer.reserve( _Frame_size * 2 );
        }

        frame_stream& operator<<( util::erased_view< float >&& _Buffer ) {

                m_buffer.insert( m_buffer.end(), _Buffer.begin(), _Buffer.end() );

                return *this;
        }

        frame_stream& operator<<( const std::vector< float >& _Buffer ) {

                m_buffer.insert( m_buffer.end(), _Buffer.begin(), _Buffer.end() );

                return *this;
        }

        frame_stream& operator<<( const util::erased_view< float >& _Buffer ) {

                m_buffer.insert( m_buffer.end(), _Buffer.begin(), _Buffer.end() );

                return *this;
        }

        frame_stream& operator>>( std::vector< std::vector< float > >& _Buffer ) {

                auto frame_count = m_buffer.size() >= m_frame_size ?
                                ( m_buffer.size() - m_frame_size ) / m_hop_size + 1 : 0;

                _Buffer.clear();
                _Buffer.reserve( frame_count );

                for ( ; m_buffer.size() - m_offset >= m_frame_size; ) {

                        _Buffer.emplace_back(
                                m_buffer.begin(),
                                m_buffer.begin() + m_frame_size
                        );

                        m_offset += m_hop_size;

                }

                if ( m_offset != 0 ) {

                        m_buffer.erase(
                                m_buffer.begin(),
                                m_buffer.begin() + m_offset
                        );

                        m_offset = 0;
                }

                return *this;
        }

}; // frame_stream

export
template< util::is_dsp_operation_type _Operation_Ty >
class for_each_frame : public util::dsp_operation_base {

private:
        _Operation_Ty m_operation;

public:
        explicit for_each_frame( _Operation_Ty&& _Operation ) : m_operation( std::forward< _Operation_Ty >( _Operation ) ) {}

        template< std::ranges::range _Range_Ty >
        auto apply_to( _Range_Ty&& _Range )
                requires ( std::is_lvalue_reference_v< _Operation_Ty > )
        {
                return std::forward< _Range_Ty >( _Range )
                        | std::views::transform(
                                [ &op = m_operation ]< typename _Frame_Ty >
                                ( _Frame_Ty&& _Frame ) mutable {
                                       return op.apply_to( std::forward< _Frame_Ty >( _Frame ) );
                        } );
        }

        template< std::ranges::range _Range_Ty >
        auto apply_to( _Range_Ty&& _Range ) &
                requires ( !std::is_lvalue_reference_v< _Operation_Ty > )
        {
                return std::forward< _Range_Ty >( _Range )
                        | std::views::transform(
                                [ &op = m_operation ]< typename _Frame_Ty >
                                ( _Frame_Ty&& _Frame ) mutable {
                                        return op.apply_to( std::forward< _Frame_Ty >( _Frame ) );
                        } );
        }

        template< std::ranges::range _Range_Ty >
        auto apply_to( _Range_Ty&& _Range ) &&
                requires ( !std::is_lvalue_reference_v< _Operation_Ty > )
        {
                return std::forward< _Range_Ty >( _Range )
                        | std::views::transform(
                                [ op = std::forward< _Operation_Ty >( m_operation ) ]< typename _Frame_Ty >
                                ( _Frame_Ty&& _Frame ) mutable {
                                        return std::forward< _Operation_Ty >( op ).apply_to( std::forward< _Frame_Ty >( _Frame ) );
                        } );
        }

}; // for_each_frame

export
template< class _Operation_Ty >
for_each_frame( _Operation_Ty&& ) -> for_each_frame< _Operation_Ty >;

export
template< std::ranges::range _First_Ty, std::ranges::range _Seconde_Ty >
constexpr auto zip_frame( _First_Ty&& _First, _Seconde_Ty&& _Seconde ) {
        return std::views::zip(
                std::forward< _First_Ty   >(  _First  ),
                std::forward< _Seconde_Ty >( _Seconde )
        );
}

} // namespace dsp
