
module;

#include <memory>
#include <vector>
#include <array>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <thread>

export module util.erased_view;

namespace util {

export
template< typename _Value_Ty >
class erased_view {

public:
        using value_type = _Value_Ty;

public:
        class iterator {
        public:
                using value_type        = _Value_Ty;
                using difference_type   = std::ptrdiff_t;
                using iterator_category = std::random_access_iterator_tag;
                using iterator_concept  = std::random_access_iterator_tag;

        private:
                const erased_view* m_view_ptr;
                std::size_t m_index;

        public:
                iterator() : m_view_ptr( nullptr ), m_index( 0 ) {}
                iterator( const erased_view* _View_ptr, std::size_t _Pos = 0 ) : m_view_ptr( _View_ptr ), m_index( _Pos ) {}

                value_type operator*() const {
                        return ( *m_view_ptr )[ m_index ];
                }

                iterator& operator++() {
                        ++m_index;
                        return *this;
                }

                iterator operator++( int ) {
                        auto temp = *this;
                        ++*this;
                        return temp;
                }

                iterator& operator--() {
                        --m_index;
                        return *this;
                }

                iterator operator--( int ) {
                        auto temp = *this;
                        --*this;
                        return temp;
                }

                iterator& operator+=( difference_type _N ) {
                        m_index += _N;
                        return *this;
                }

                iterator& operator-=( difference_type _N ) {
                        m_index -= _N;
                        return *this;
                }

                value_type operator[]( difference_type _N ) const {
                        return ( *m_view_ptr )[ m_index + _N ];
                }

                friend iterator operator+( iterator _It, difference_type _N ) {
                        _It += _N;
                        return _It;
                }

                friend iterator operator+( difference_type _N, iterator _It ) {
                        _It += _N;
                        return _It;
                }

                friend iterator operator-( iterator _It, difference_type _N ) {
                        _It -= _N;
                        return _It;
                }

                friend difference_type operator-( const iterator& _Lhs, const iterator& _Rhs ) {
                        if ( _Lhs.m_view_ptr != _Rhs.m_view_ptr )
                                throw std::logic_error( "Iterator pointing to another range" );

                        return static_cast< difference_type >( _Lhs.m_index ) -
                               static_cast< difference_type >( _Rhs.m_index );
                }

                friend bool operator==( const iterator& _Lhs, const iterator& _Rhs ) {
                        if ( _Lhs.m_view_ptr != _Rhs.m_view_ptr )
                                throw std::logic_error( "Iterator pointing to another range" );

                        return _Lhs.m_index == _Rhs.m_index;
                }

                friend auto operator<=>( const iterator& _Lhs, const iterator& _Rhs ) {
                        if ( _Lhs.m_view_ptr != _Rhs.m_view_ptr )
                                throw std::logic_error( "Iterator pointing to another range" );

                        return _Lhs.m_index <=> _Rhs.m_index;
                }
        };

private:
        class view_concept {

        public:
                virtual ~view_concept() = default;

                virtual std::size_t size() const = 0;
                virtual value_type index_access( std::size_t ) const = 0;

                // virtual std::unique_ptr< view_concept > clone() const = 0;
                virtual std::vector< value_type > to_vector() const = 0;

        };

        template< std::ranges::random_access_range _Range_Ty >
                requires std::same_as< std::ranges::range_value_t< _Range_Ty >, value_type >
        class model : public view_concept {

        public:
                using range_type = _Range_Ty;

        private:
                range_type m_range;

        public:
                explicit model( range_type&& _Range ) : m_range( std::forward< range_type >( _Range ) ) {}

                ~model() noexcept override = default;

                std::size_t size() const override {
                        if constexpr ( std::ranges::sized_range< range_type > ) {
                                return m_range.size();
                        }
                        else {
                                return std::ranges::distance( m_range );
                        }
                }

                value_type index_access( std::size_t _I ) const override {
                        return m_range[ _I ];
                }

                // std::unique_ptr< view_concept > clone() const override {
                //         static_assert( std::copy_constructible< range_type >, "This range can't copy constructed" );
                //         return std::make_unique< model >( *this );
                // }

                std::vector< value_type > to_vector() const override {
                        return m_range | std::ranges::to< std::vector< value_type > >();
                }

        };

private:
        std::unique_ptr< view_concept > m_view_impl;

public:
        erased_view() : m_view_impl( nullptr ) {}

        template< std::ranges::random_access_range _Range_Ty >
                requires std::same_as< std::ranges::range_value_t< _Range_Ty >, value_type >
        erased_view( _Range_Ty&& _Range )
                : m_view_impl(
                        std::make_unique< model< _Range_Ty > >( std::forward< _Range_Ty >( _Range ) )
                )
        {}

        erased_view( const std::vector< value_type >& _Range )
                : m_view_impl(
                        std::make_unique< model< std::span< const value_type > > >( std::span< const value_type >( _Range ) )
                )
        {}

        template< std::size_t _N >
        erased_view( const std::array< value_type, _N >& _Range )
                : m_view_impl(
                        std::make_unique< model< std::span< const value_type > > >( std::span< const value_type >( _Range ) )
                )
        {}

        erased_view( std::vector< value_type >&& _Range ) = delete;

        template< std::size_t _N >
        erased_view( std::array< value_type, _N >&& _Range ) = delete;

        // erased_view( const erased_view& _Other ) : m_view_impl( _Other.m_view_impl ? _Other.m_view_impl->clone() : nullptr ) {}
        erased_view( const erased_view& ) = delete;

        // erased_view& operator=( const erased_view& _Other ) {
        //
        //         if ( this == &_Other ) {
        //                 return *this;
        //         }
        //
        //         m_view_impl = _Other.m_view_impl ? _Other.m_view_impl->clone() : nullptr;
        //
        //         return *this;
        // }
        erased_view& operator=( const erased_view& ) = delete;

        erased_view( erased_view&& ) noexcept = default;

        erased_view& operator=( erased_view&& ) noexcept = default;

        ~erased_view() noexcept = default;

        std::size_t size() const {
                return m_view_impl->size();
        }

        value_type operator[]( std::size_t _I ) const {
                return m_view_impl->index_access( _I );
        }

        auto to_vector() const {
                return m_view_impl->to_vector();
        }

        iterator begin() const {
                return iterator( this, 0 );
        }

        iterator end() const {
                return iterator( this, m_view_impl->size() );
        }

        bool has_view() const {
                return m_view_impl != nullptr;
        }

}; // class erased_view

export
template< std::ranges::random_access_range _Range_Ty >
erased_view( _Range_Ty&& ) -> erased_view< std::ranges::range_value_t< _Range_Ty > >;

} // namespace util
