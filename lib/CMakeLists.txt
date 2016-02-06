enable_language(C ASM)

set(ASM_OPTIONS "-x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")

set(LIBTASK_SRC
        channel.c
        context.c
        fd.c
        net.c
        qlock.c
        rendez.c
        task.c
        asm.S)
add_library(task STATIC ${LIBTASK_SRC})