#ifndef MODEL_H
#define MODEL_H
#include "rx.h"
#include "session_data_manager.h"
#include "buffer_interface.h"
#include "ui.h"
#include <memory>
#include <string>

namespace vm {

  class BufferInterface;

  class ReadableBufferInterface;

  class Command;

  class ModelCopyNotImplemented {};

  class Model : public Subject {
    std::unique_ptr<vm::BufferInterface> buff = nullptr;

    // Normally this would stay true while any single buffer is open
    // But since we only have one buffer for this implementation, it only
    // depends on whether or not buff is open
    bool is_running;

    public:
      Model() : Subject{} {}

      // TODO Implement this. Relies on the copy ctor of the DllBuffer, which
      // isn't implemented
      Model(const Model& other) : Subject{other} {
        throw ModelCopyNotImplemented();
      }
      Model(Model&& other) : Subject{std::move(other)}, buff{std::move(other.buff)} {}
      Model& operator=(const Model& other) {
        // TODO remove this throw. see above copy ctor for info on why we're
        // throwing
        throw ModelCopyNotImplemented{};
      }
      Model& operator=(Model&& other) {
        Subject::operator=(std::move(other));
        buff = std::move(other.buff);
        return *this;
      }
      ~Model() {}

      void addBuffer(std::unique_ptr<vm::BufferInterface>);
      void execute(vm::Command&);
      vm::ReadableBufferInterface* getBuffer();
      bool isRunning() { return is_running; }
  };
}

#endif
