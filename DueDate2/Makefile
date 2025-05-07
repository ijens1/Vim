CXX = g++
current_dir = $(shell pwd)
model_include_dir = $(current_dir)/model
controller_include_dir = $(current_dir)/controller
view_include_dir = $(current_dir)/view
CXXFLAGS = -std=c++14 -Wall -g -MMD -I $(current_dir) -I $(model_include_dir) -I $(controller_include_dir) -I $(view_include_dir)
CXXENDFLAGS = -lncurses
EXEC = vm
BUILDDIR = obj
OBJECTS = main.o view/ui.o view/formatter.o controller/cmdgen.o rx.o controller/controller.o view/view.o model/model.o controller/command.o model/buffer_interface.o model/dll_buffer.o controller/register_manager.o controller/session_data_manager.o

DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${OBJECTS} -o ${EXEC} ${CXXENDFLAGS}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${EXEC} ${OBJECTS} ${DEPENDS}
