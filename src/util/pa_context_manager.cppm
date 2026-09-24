
module;

#include <iostream>

#include <portaudio.h>

export module util.pa_context_manager;

namespace util {
export class pa_context_manager {

private:
	inline static bool m_is_initialized = false;

private:
	pa_context_manager() {
		initialize();
	}
	~pa_context_manager() {
		terminate();
	}

public:
	pa_context_manager( const pa_context_manager& ) = delete;
	pa_context_manager& operator=( const pa_context_manager& ) = delete;

	pa_context_manager( pa_context_manager&& ) = default;
	pa_context_manager& operator=( pa_context_manager&& ) = default;

	static const pa_context_manager& get() {
		static auto manager = pa_context_manager();
		return manager;
	}

	static PaError initialize() {
		if ( !m_is_initialized ) {
			m_is_initialized = true;
			return Pa_Initialize();
		}
		return paNoError;
	}

	static PaError terminate() {
		if ( m_is_initialized ) {
			m_is_initialized = false;
			return Pa_Terminate();
		}
		return paNoError;
	}

	template < typename _Func_Ty, typename... _Arg_Ty >
	static decltype( auto ) call( _Func_Ty _Func_ptr, _Arg_Ty&&... _Arg_values ) {
		initialize();
		return _Func_ptr( std::forward< _Arg_Ty >( _Arg_values )... );
	}

	static bool is_initialized() noexcept {
		return m_is_initialized;
	}

        static const char* error_text( PaError _Err ) {
	        return Pa_GetErrorText( _Err ) ;
	}
};

}

namespace util {

export const pa_context_manager& pa = pa_context_manager::get();

}
