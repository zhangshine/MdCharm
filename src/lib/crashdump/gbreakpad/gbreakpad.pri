BREAKPAD_PATH = $$PWD
INCLUDEPATH += $$BREAKPAD_PATH

# every *nix
unix {
        SOURCES += $$BREAKPAD_PATH/client/minidump_file_writer.cc \
                $$BREAKPAD_PATH/common/string_conversion.cc \
                $$BREAKPAD_PATH/common/convert_UTF.c \
                $$BREAKPAD_PATH/common/md5.cc
}

# mac os x
mac {
	# hack to make minidump_generator.cc compile as it uses
	# esp instead of __esp
        # DEFINES += __DARWIN_UNIX03=0 -- looks like we doesn't need it anymore

        SOURCES += $$BREAKPAD_PATH/client/mac/handler/exception_handler.cc \
                $$BREAKPAD_PATH/client/mac/handler/minidump_generator.cc \
                $$BREAKPAD_PATH/client/mac/handler/dynamic_images.cc \
                $$BREAKPAD_PATH/common/mac/string_utilities.cc \
                $$BREAKPAD_PATH/common/mac/file_id.cc \
                $$BREAKPAD_PATH/common/mac/macho_id.cc \
                $$BREAKPAD_PATH/common/mac/macho_utilities.cc \
                $$BREAKPAD_PATH/common/mac/macho_walker.cc
}

# other *nix
unix:!mac {
        SOURCES += $$BREAKPAD_PATH/client/linux/handler/exception_handler.cc \
                #$$BREAKPAD_PATH/client/linux/handler/minidump_generator.cc \
                #$$BREAKPAD_PATH/client/linux/handler/linux_thread.cc \
                $$BREAKPAD_PATH/client/linux/log/log.cc \
                $$BREAKPAD_PATH/client/linux/crash_generation/crash_generation_client.cc \
                $$BREAKPAD_PATH/client/linux/minidump_writer/minidump_writer.cc \
                $$BREAKPAD_PATH/client/linux/minidump_writer/linux_core_dumper.cc \
                $$BREAKPAD_PATH/client/linux/minidump_writer/linux_dumper.cc \
                $$BREAKPAD_PATH/client/linux/minidump_writer/linux_ptrace_dumper.cc \
                $$BREAKPAD_PATH/common/linux/guid_creator.cc \
                $$BREAKPAD_PATH/common/linux/file_id.cc \
                $$BREAKPAD_PATH/common/linux/safe_readlink.cc \
                $$BREAKPAD_PATH/common/linux/memory_mapped_file.cc \
}

win32 {
        SOURCES += $$BREAKPAD_PATH/client/windows/handler/exception_handler.cc \
                $$BREAKPAD_PATH/client/windows/crash_generation/crash_generation_client.cc \
                $$BREAKPAD_PATH/common/windows/guid_string.cc
}
