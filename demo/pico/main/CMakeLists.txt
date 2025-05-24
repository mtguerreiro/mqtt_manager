if (TARGET tinyusb_device)

        add_executable(main
        main.c
        )

        add_subdirectory(tasks)

        # pull in common dependencies
        target_link_libraries(main tasks pico_stdlib FreeRTOS-Kernel FreeRTOS-Kernel-Heap4 tif hw_pico_modules)

        target_include_directories(main PUBLIC
                           "${TIF_PATH}"
                           )

        # enable usb output, disable uart output
        pico_enable_stdio_usb(main 1)
        pico_enable_stdio_uart(main 0)

        # create map/bin/hex/uf2 file etc.
        pico_add_extra_outputs(main)

        # add url via pico_set_program_url
        example_auto_set_url(main)
elseif(PICO_ON_DEVICE)
        message(WARNING "not building main because TinyUSB submodule is not initialized in the SDK")
endif()
