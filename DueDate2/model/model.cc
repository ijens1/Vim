#include "model.h"
#include "readable_buffer_interface.h"
#include "readable_session_data_interface.h"
#include "buffer_interface.h"
#include "session_data_manager.h"
#include "command.h"

void vm::Model::addBuffer(std::unique_ptr<vm::BufferInterface> buff) {
  this->buff = std::move(buff);
  this->buff->setIsOpen(true);
  this->buff->setHasUnsavedChanges(false);
}

void vm::Model::execute(vm::Command &c) {
  c.execute(buff.get());

  // If our one buffer has closed, stop running the session
  is_running = buff->isOpen();
  notifyObservers();
}

vm::ReadableBufferInterface* vm::Model::getBuffer() {
  return buff.get();
}
