#ifndef DLL_BUFFER_H
#define DLL_BUFFER_H
#include "buffer_interface.h"
#include <stack>
#include <memory>

namespace vm {

  class DllBufferCopyNotImplemented {};

  class DllBuffer : public BufferInterface {
      static const size_t chunk_size = 50;

      class LineChunk {
        public:
          LineChunk *prev = nullptr;
          std::unique_ptr<LineChunk> next = nullptr;
          std::string str_chunk;

          size_t sizeOfChunk() { return str_chunk.length(); }
      };

      class Line {
        public:
          Line *prev = nullptr;
          std::unique_ptr<Line> next = nullptr;
          std::unique_ptr<LineChunk> start_of_line = std::make_unique<LineChunk>();

          std::string concatenateAllChunks() {
            std::string line;
            LineChunk* lc = start_of_line.get();
            while (lc != nullptr) {
              line += lc->str_chunk;
              lc = lc->next.get();
            }
            return line;
          }

        // Note: only returns number of lines including this one and those
        // after it (NOT ENTIRE LIST LENGTH)
          size_t getNumLines() const {
            size_t count = 1;
            Line* curr = next.get();
            while (curr != nullptr) {
              count++;
              curr = curr->next.get();
            }
            return count;
          }
      };

      std::unique_ptr<Line> start_of_file = std::make_unique<Line>();
      bool has_unsaved_changes;
      bool is_open;

      void doInsertText(size_t, size_t, std::string) override;
      void doSetHasUnsavedChanges(bool) override;
      void doSetIsOpen(bool) override;
      void doWriteToStream(std::ostream&) override;

    // Note: to delete to the end of a line, just pass std::string::npos to
    // len param.
      void doDeleteText(size_t, size_t, size_t) override;
      std::string doGetLine(size_t) const override;
      size_t doGetNumLines() const override;
      bool doHasUnsavedChanges() const override;
      bool doIsOpen() const override;

    // Removes empty LineChunks from the Line provided
    // Used after inserting or deleting text
    // Note: It will not remove empty chunks at the beginning of a Line if
    // they aren't pointing to a next chunk. To remove these chunks, call
    // delete with c param value of zero, len param value one.
      void cleanLine(Line *);
    public:

      DllBuffer() : BufferInterface{0} {}
      DllBuffer(int id) : BufferInterface{id} {}

    // TODO: Implement this
    // Need to copy all of the buffer data
    // Can most likely delegate the copying of that to the classes Line and
    // LineChunk, but they have yet to be written.
      DllBuffer(const DllBuffer& other) : BufferInterface{other} {
        throw DllBufferCopyNotImplemented{};
      }
      DllBuffer(DllBuffer&& other) : BufferInterface{std::move(other)}, start_of_file{std::move(other.start_of_file)} {}

    // See above copy ctor about copying this class
      DllBuffer& operator=(const DllBuffer& other) {
      // TODO remove this throw
        throw DllBufferCopyNotImplemented{};
        BufferInterface::operator=(other);
        return *this;
      }
      DllBuffer& operator=(DllBuffer&& other) {
        BufferInterface::operator=(std::move(other));
        return *this;
      }
      ~DllBuffer() {}
  };

}

#endif
