cmake_minimum_required( VERSION 4.3 )

if ( WIN32 )
        set( VCPKG_TARGET_TRIPLET x64-windows-static )
endif ()

project( Spectrum )

set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# ============================================================
# Dependencies
# ============================================================

find_package( SndFile CONFIG REQUIRED )
find_package( portaudio CONFIG REQUIRED )
find_package( doctest CONFIG REQUIRED )

# ============================================================
# Spectrum Core
# ============================================================

add_library( SpectrumCore )

target_compile_definitions( SpectrumCore PRIVATE
        $<$<CONFIG:Debug>:IS_DEBUG=1>
        $<$<CONFIG:Release>:IS_DEBUG=0>
)

target_sources( SpectrumCore
        PUBLIC
        FILE_SET CXX_MODULES
        FILES

        # audio
        src/audio/type.cppm
        src/audio/format.cppm
        # src/audio/buffer.cppm
        src/audio/device.cppm
        # src/audio/stream.cppm
        # src/audio/source.cppm
        # src/audio/output.cppm
        src/audio/pcm.cppm
        src/audio/ios_base.cppm
        src/audio/ios.cppm
        src/audio/stream_buffer.cppm
        src/audio/iostream.cppm
        src/audio/fstream.cppm
        src/audio/audio.cppm

        # dsp
        src/dsp/dsp.cppm
        src/dsp/window.cppm
        src/dsp/fft.cppm
        src/dsp/pipe.cppm
        src/dsp/frame.cppm
        src/dsp/gain.cppm
        src/dsp/units.cppm
        src/dsp/channel.cppm

        # ui
        src/ui/spectrum.cppm
        src/ui/render.cppm
        src/ui/ui.cppm

        # util
        src/util/pa_context_manager.cppm
        src/util/type.cppm
        src/util/ticker.cppm
        src/util/updater.cppm
        src/util/erased_view.cppm
        src/util/debug.cppm
        src/util/util.cppm
)

target_include_directories( SpectrumCore
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/pocketfft
)

target_link_libraries( SpectrumCore
        PUBLIC
        SndFile::sndfile
        portaudio_static
)


# ============================================================
# Spectrum
# ============================================================

add_executable( Spectrum
        src/main.cpp
        src/dsp/fft_pocketfft.cpp
)

target_link_libraries( Spectrum
        PRIVATE
        SpectrumCore
)


# ============================================================
# Tests
# ============================================================

enable_testing()

add_executable( SpectrumTests
        tests/test_main.cpp
        tests/test_ring_buffer.cpp
        # tests/test_updater.cpp
        # tests/test_ticker.cpp
        # tests/test_erased_view.cpp
)

target_link_libraries( SpectrumTests
        PRIVATE
        SpectrumCore
        doctest::doctest
)

include( doctest )

doctest_discover_tests( SpectrumTests )