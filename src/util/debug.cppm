
module;

#include <chrono>
#include <mutex>
#include <print>

export module util.debug;

namespace util::debug {

constexpr bool is_debug = IS_DEBUG;

export
class print_time_t {

private:
        const std::chrono::time_point< std::chrono::steady_clock > m_start = std::chrono::steady_clock::now();
        mutable std::mutex m_print_mutex;

public:
        template< typename... _Args_Ty >
        void operator()( std::format_string< _Args_Ty... > _Fmt, _Args_Ty&&... _Args ) const {

                if constexpr ( !is_debug ) {
                        return;
                }

                const auto now = std::chrono::steady_clock::now();
                const auto ms = std::chrono::duration_cast< std::chrono::milliseconds >( now - m_start ).count();

                std::lock_guard lock{ m_print_mutex };

                // std::print( "[{:>5}ms]: ", ms );
                // std::println( _Fmt, std::forward< _Args_Ty >( _Args )... );

        }

};

export auto print_time = print_time_t{};

}

