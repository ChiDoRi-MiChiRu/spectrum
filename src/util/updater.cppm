
module;

#include <functional>
#include <optional>
#include <queue>
#include <thread>
#include <shared_mutex>

export module util.updater;

import util.ticker;

namespace util {

export
template < typename _Tp >
class updater {

public:
        using value_type = _Tp;

private:
        std::queue< value_type > m_data_queue;

        std::size_t m_max_size;

        std::shared_mutex m_data_mutex;

private:
        std::optional< value_type > try_update() {
                {

                        std::unique_lock lock{ m_data_mutex };

                        if ( m_data_queue.empty() ) {
                                return std::nullopt;
                        }

                        auto data = std::move( m_data_queue.front() );
                        m_data_queue.pop();

                        return data;

                }

        }

        util::ticker m_ticker;

public:
        template< typename _Func_Ty >
        // updater( _Func_Ty _Callback, std::chrono::milliseconds _Interval, std::size_t _Max_size = 1024 )
        updater( _Func_Ty _Callback, std::size_t _Sample_rate, std::size_t _Hop_size, std::size_t _Max_size = 1024 )
                : m_max_size( _Max_size ), m_ticker(
                        [ this, func = std::move( _Callback ) ] {
                                auto result = try_update();

                                func( std::move( result ) );

                        },
                        std::chrono::milliseconds( static_cast< int >( 1000 * static_cast< float >( _Hop_size ) / _Sample_rate ) )
        ) {

        }

        template< typename... _Args_Ty >
        updater& push( _Args_Ty&&... _Args ) {

                std::unique_lock lock{ m_data_mutex };

                for ( ; m_data_queue.size() >= m_max_size - 1; ) {
                        m_data_queue.pop();
                }

                m_data_queue.push( std::forward< _Args_Ty >( _Args )... );

                return *this;

        }

        updater& start() {

                m_ticker.start();

                return *this;

        }

        updater& stop() {

                m_ticker.stop();

                return *this;

        }

        bool has_pending_update() const {

                std::unique_lock lock{ m_data_mutex };
                return !m_data_queue.empty();

        }


}; // class updater

} // namespace util
