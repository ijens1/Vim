#include "dll_buffer.h"
#include "command.h"

void vm::DllBuffer::doInsertText(size_t l, size_t c, std::string str) {
  Line* curr_line = start_of_file.get();
  size_t i = 1;
  while (i++ != l && curr_line != nullptr) curr_line = curr_line->next.get();
  if (curr_line == nullptr) return;
  else {
    if (str.find(vm::BufferInterface::end_of_line_expr) == std::string::npos) {
      bool inserted_in_starting_chunk = false;
      int last_index_prev_chunk = -1;
      LineChunk *curr_chunk = curr_line->start_of_line.get();
      while (last_index_prev_chunk + curr_chunk->sizeOfChunk() < (c - 1) && curr_chunk->next != nullptr) {
        last_index_prev_chunk += curr_chunk->sizeOfChunk();
        curr_chunk = curr_chunk->next.get();
      }

    // If we're inserting at the very end of the line
    // Set up empty chunk and move last_index_prev chunk to the end of the
    // chunk before it. Make curr_chunk the newly created empty chunk.
      if (last_index_prev_chunk + curr_chunk->sizeOfChunk() < (c - 1)) {
          curr_chunk->next = std::make_unique<LineChunk>();
          curr_chunk->next->prev = curr_chunk;
          last_index_prev_chunk += curr_chunk->sizeOfChunk();
          curr_chunk = curr_chunk->next.get();
      }

      if (curr_chunk == curr_line->start_of_line.get()) inserted_in_starting_chunk = true;

    // curr_chunk now points at the chunk where the new string will be
    // inserted between
      size_t index_of_insert_in_chunk = (c - 1) - (last_index_prev_chunk + 1);
      std::string pre_insert_chunk_str = curr_chunk->str_chunk.substr(0, index_of_insert_in_chunk),
        post_insert_chunk_str = curr_chunk->str_chunk.substr(index_of_insert_in_chunk);

    // Ceiling the number of new chunks required
      size_t num_new_chunks_for_insert = (str.length() + chunk_size) / chunk_size;
      std::unique_ptr<LineChunk> curr = nullptr;
      LineChunk *start = nullptr, *prev = nullptr, *next = nullptr;
      for (size_t j = 0; j < num_new_chunks_for_insert; ++j) {
        curr = std::make_unique<LineChunk>();
        curr->next = nullptr;
        std::string new_chunk_str = str.substr(0, (str.length() >= chunk_size) ? chunk_size : str.length());
        str = str.substr(new_chunk_str.length());
        curr->str_chunk = new_chunk_str;
        if (j == 0) start = curr.get();
        curr->prev = prev;
        next = curr.get();
        if (prev != nullptr) prev->next = std::move(curr);
        else curr.release();
        prev = next;
      }

    // These two chunks are the ones created from splitting curr_chunk
      std::unique_ptr<LineChunk> pre_insert_chunk = std::make_unique<LineChunk>();
      std::unique_ptr<LineChunk> post_insert_chunk = std::make_unique<LineChunk>();

    // These two chunks are the ones that the split chunks should point to
      LineChunk* insert_after_chunk = curr_chunk->prev;
      std::unique_ptr<LineChunk> insert_before_chunk = std::move(curr_chunk->next);

    // Set up the first part of the split chunk
      pre_insert_chunk->prev = insert_after_chunk;
      pre_insert_chunk->next.reset(start);
      pre_insert_chunk->next->prev = pre_insert_chunk.get();
      pre_insert_chunk->str_chunk = pre_insert_chunk_str;
      if (insert_after_chunk != nullptr)
        insert_after_chunk->next = std::move(pre_insert_chunk);


    // Set up the last part of the split chunk
      post_insert_chunk->str_chunk = post_insert_chunk_str;
      post_insert_chunk->next = std::move(insert_before_chunk);
      if (post_insert_chunk->next != nullptr)
        post_insert_chunk->next->prev = post_insert_chunk.get();
      post_insert_chunk->prev = prev;
      prev->next = std::move(post_insert_chunk);

    // Have to set up ownership for the pre_insert_chunk
      if (inserted_in_starting_chunk) curr_line->start_of_line = std::move(pre_insert_chunk);

  // We're inserting at least one new line into the buffer
    } else {
      --i;
      if (c <= curr_line->concatenateAllChunks().length()) {
        std::string removed_extra = curr_line->concatenateAllChunks().substr(c - 1);
        str += removed_extra;
        doDeleteText(i, c, std::string::npos);
      }
      size_t first_nl_index = str.find(vm::BufferInterface::end_of_line_expr);
      if (first_nl_index != 0) {
        doInsertText(i, curr_line->concatenateAllChunks().length() + 1, str.substr(0, first_nl_index));
        str = str.substr(first_nl_index);
      }
      while (!str.empty()) {
        str = str.substr(1);
        size_t next_nl_index = str.find(vm::BufferInterface::end_of_line_expr);
        std::unique_ptr<Line> nl = std::make_unique<Line>();
        if (curr_line->next != nullptr)
          curr_line->next->prev = nl.get();
        nl->next = std::move(curr_line->next);
        nl->prev = curr_line;
        curr_line->next = std::move(nl);
        doInsertText(++i, 1, str.substr(0, next_nl_index));
        if (next_nl_index == std::string::npos) str.clear();
        else str = str.substr(next_nl_index);
        curr_line = curr_line->next.get();
      }
    }

  // Clean the line in case we've added any extraneous chunks
    cleanLine(curr_line);
  }
}

void vm::DllBuffer::doSetHasUnsavedChanges(bool has_unsaved_changes) {
  this->has_unsaved_changes = has_unsaved_changes;
}

void vm::DllBuffer::doSetIsOpen(bool is_open) {
  this->is_open = is_open;
}

void vm::DllBuffer::doWriteToStream(std::ostream& os) {
  Line* curr_line = start_of_file.get();
  while (curr_line != nullptr) {
    os << curr_line->concatenateAllChunks() << vm::BufferInterface::end_of_line_expr;
    curr_line = curr_line->next.get();
  }
}

void vm::DllBuffer::doDeleteText(size_t l, size_t c, size_t len) {
  Line* curr_line = start_of_file.get();
  size_t i = 1;
  while (i++ != l && curr_line != nullptr) curr_line = curr_line->next.get();
  if (curr_line == nullptr) return;
  else {
    if (c > curr_line->concatenateAllChunks().length()) return;
    if (c == 0) {
      if (curr_line->prev != nullptr) {
        Line* prev_line = curr_line->prev;
        LineChunk* curr_chunk = prev_line->start_of_line.get();
        while (curr_chunk->next != nullptr) curr_chunk = curr_chunk->next.get();
        curr_chunk->next = std::move(curr_line->start_of_line);
        curr_chunk->next->prev = curr_chunk;
        prev_line->next = std::move(curr_line->next);
        if (prev_line->next != nullptr)
          prev_line->next->prev = prev_line;
        cleanLine(prev_line);
      }
    } else {
      int last_index_prev_chunk = -1;
      LineChunk* curr_chunk = curr_line->start_of_line.get();
      while (last_index_prev_chunk + curr_chunk->sizeOfChunk() < (c-1)) {
        last_index_prev_chunk += curr_chunk->sizeOfChunk();
        curr_chunk = curr_chunk->next.get();
      }
      LineChunk* delete_from = curr_chunk;
      size_t removed_count = delete_from->sizeOfChunk();
      delete_from->str_chunk.erase(
          (c-2)-(last_index_prev_chunk),
          (len==std::string::npos || (c-2)-(last_index_prev_chunk)+len >= delete_from->sizeOfChunk())
          ? std::string::npos : len);
      if (len == std::string::npos) {
        curr_chunk->next.reset(nullptr);
      } else {
        removed_count -= delete_from->sizeOfChunk();
        if (removed_count != len) {
          last_index_prev_chunk += curr_chunk->sizeOfChunk() + removed_count;
          curr_chunk = curr_chunk->next.get();
          while (last_index_prev_chunk + curr_chunk->sizeOfChunk() < (c-1) + len) {
            removed_count += curr_chunk->sizeOfChunk();
            last_index_prev_chunk += curr_chunk->sizeOfChunk();
            curr_chunk = curr_chunk->next.get();
          }
          curr_chunk->str_chunk=curr_chunk->str_chunk.substr(len - removed_count);
          delete_from->next = std::move(curr_chunk->prev->next);
        }
      }
      cleanLine(curr_line);
    }
  }
}

std::string vm::DllBuffer::doGetLine(size_t l) const {
  Line* curr = start_of_file.get();
  size_t i = 1;
  while (i++ != l && curr != nullptr) curr = curr->next.get();
  return (curr != nullptr) ? curr->concatenateAllChunks() : std::string();
}

size_t vm::DllBuffer::doGetNumLines() const {
  return start_of_file->getNumLines();
}

bool vm::DllBuffer::doHasUnsavedChanges() const {
  return has_unsaved_changes;
}

bool vm::DllBuffer::doIsOpen() const {
  return is_open;
}

void vm::DllBuffer::cleanLine(Line *line) {
  LineChunk* curr = line->start_of_line.get();
  while (curr != nullptr && (curr->next != nullptr || curr->prev != nullptr)) {
    if (curr->sizeOfChunk() == 0) {
      if (curr->prev == nullptr) {
        line->start_of_line = std::move(curr->next);
        if (line->start_of_line != nullptr)
          line->start_of_line->prev = nullptr;
        curr = line->start_of_line.get();
      } else if (curr->next != nullptr) {
        LineChunk* prev = curr->prev;
        prev->next = std::move(curr->next);
        if (prev->next != nullptr)
          prev->next->prev = prev;
        curr = prev->next.get();
      } else {
        curr = curr->prev;
        curr->next.reset(nullptr);
      }
    } else {
      curr = curr->next.get();
    }
  }
}

