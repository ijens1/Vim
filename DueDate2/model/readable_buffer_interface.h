#ifndef READABLE_BUFFER_INTERFACE_H
#define READABLE_BUFFER_INTERFACE_H

#include <string>

namespace vm {

  class ReadableBufferInterface {
      int id;
      virtual std::string doGetLine(size_t) const = 0;
      virtual size_t doGetNumLines() const = 0;
      virtual bool doHasUnsavedChanges() const = 0;
      virtual bool doIsOpen() const = 0;

    public:
      ReadableBufferInterface() {}
      ReadableBufferInterface(int id) : id{id} {}
      ReadableBufferInterface(const ReadableBufferInterface&) = default;
      ReadableBufferInterface(ReadableBufferInterface&&) = default;
      ReadableBufferInterface& operator=(const ReadableBufferInterface&) = default;
      ReadableBufferInterface& operator=(ReadableBufferInterface&&) = default;
      virtual ~ReadableBufferInterface() = default;
      /**
       * First line starts at 1
      **/
      std::string getLine(size_t l) const { return doGetLine(l); }
      size_t getNumLines() const { return doGetNumLines(); }
      int getId() const { return id; }
      bool hasUnsavedChanges() { return doHasUnsavedChanges();}
      bool isOpen() const { return doIsOpen(); }
  };
}

#endif
