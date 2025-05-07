#ifndef BUFFER_H
#define BUFFER_H

#include <ostream>
#include "readable_buffer_interface.h"

namespace vm {

  class BufferInterface : public ReadableBufferInterface {
      virtual void doInsertText(size_t, size_t, std::string) = 0;
      virtual void doDeleteText(size_t, size_t, size_t) = 0;
      virtual void doSetHasUnsavedChanges(bool) = 0;
      virtual void doSetIsOpen(bool) = 0;
      virtual void doWriteToStream(std::ostream&) = 0;

    public:
      static const std::string end_of_line_expr;

      //BufferInterface() {}
      BufferInterface(int id) : ReadableBufferInterface{id} {}
      BufferInterface(const BufferInterface& other) : ReadableBufferInterface{other} {}
      BufferInterface(BufferInterface&& other) : ReadableBufferInterface{std::move(other)} {}
      BufferInterface& operator=(const BufferInterface& other) {
        ReadableBufferInterface::operator=(other);
        return *this;
      }
      BufferInterface& operator=(BufferInterface&& other) {
        ReadableBufferInterface::operator=(std::move(other));
        return *this;
      }
      virtual ~BufferInterface() {}

     /**
       * Starts inserting text str on line l (1-based) such that after
       * insertion, str begins at column c (1-based)
       * Also note that to start working on a new line, just pass in
       * an end_of_line_expr at the position you would like the line to end
       * This means there should never be a request to start inserting on
       * a line greater than the last line number.
       * @param l The line number the text will be inserted in. Between 1 and
       * num_lines
       * @param c The column to insert the text at (str will begin at c after
       * insert). Should be in the range 1 to line length + 1
      **/

      void insertText(size_t l, size_t c, std::string str) { doInsertText(l, c, str); }
      void deleteText(size_t l, size_t c, size_t len) { doDeleteText(l, c, len); }
      void setHasUnsavedChanges(bool has_unsaved_changes) { doSetHasUnsavedChanges(has_unsaved_changes); }

      /** We assume that commands are aware of the fact they can check
        * hasUnsavedChanges before calling this
      **/
      void setIsOpen(bool is_open) { doSetIsOpen(is_open); }
      void writeToStream(std::ostream& os) { doWriteToStream(os); }
  };
}

#endif

