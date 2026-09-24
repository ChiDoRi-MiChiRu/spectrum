
module;

#include <ranges>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>

export module ui.render;

import util.erased_view;

namespace ui {

export
class renderer {

private:
        std::size_t m_height;
        std::size_t m_bin_count;

        float m_attack;
        float m_release;

private:
        mutable std::vector< float > m_levels;

        void smooth( const util::erased_view< float >& _Data ) {
                std::size_t i = 0;

                for ( auto value : _Data ) {
                        if ( value > m_levels[ i ] ) {
                                m_levels[ i ] = m_levels[ i ] * m_attack  + value * ( 1 - m_attack  );
                        }
                        else {
                                m_levels[ i ] = m_levels[ i ] * m_release + value * ( 1 - m_release );
                        }

                        ++i;
                }
        }

public:
        renderer( std::size_t _Height, std::size_t _Bin_count, float _Attack, float _Release )
                : m_height( _Height ), m_bin_count( _Bin_count ), m_attack( _Attack ), m_release( _Release ), m_levels( _Bin_count, 0.0f ) {}


        void render( const util::erased_view< float >& _Data ) {
                std::string output;

                smooth( _Data );

                for ( int y = m_height - 1; y >= 0; --y ) {
                        for ( std::size_t x = 0; x < m_bin_count; ++x ) {
                                const auto bar_height = static_cast< int >( std::ceil( m_levels[x] * m_height ) );

                                output += y < bar_height ? '|' : ' ';
                        }

                        output += '\n';
                }

                std::cout << "\033[H" << output << std::flush;
        }

};

}
