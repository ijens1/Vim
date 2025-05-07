#include "session_data_manager.h"
#include "command.h"
#include <iostream>

std::unique_ptr<vm::Command> vm::SessionDataManager::doUndoTop() {
    if (undo_s.empty()) return nullptr;
    std::unique_ptr<vm::CommandSequence> undo_top = std::make_unique<vm::CommandSequence>(this);
    while (!undo_s.empty() && undo_s.top().second == undo_chunk) {
      undo_top->add(std::move(undo_s.top().first));
      undo_s.pop();
    }
    return std::move(undo_top);
}
