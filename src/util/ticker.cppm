
module;

#include <thread>
#include <chrono>
#include <functional>
#include <shared_mutex>

export module util.ticker;

namespace util {

export
class ticker {

private:
        std::function< void() > m_function;
        std::chrono::milliseconds m_interval;

        std::thread m_thread;
        std::atomic_bool m_is_running{ false };

public:
        explicit ticker( std::function< void() > _Callback, std::chrono::milliseconds _Interval ) : m_function( _Callback ), m_interval{ _Interval } {}

        ticker& start() {

                if ( m_thread.joinable() ) {
                        return *this;
                }

                m_is_running.store( true );

                m_thread = std::thread( [ this ] {

                        for ( auto next = std::chrono::steady_clock::now(); m_is_running.load(); ) {

                                m_function();

                                next += m_interval;
                                std::this_thread::sleep_until( next );

                        }

                } );

                return *this;

        }

        ticker& stop() {

                if ( !m_thread.joinable() ) {
                        return *this;
                }

                m_is_running.store( false );
                m_thread.join();

                return *this;

        }

}; // class ticker

} // namespace ui

