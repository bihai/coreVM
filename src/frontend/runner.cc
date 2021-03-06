/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Yanzheng Li

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#include "runner.h"

#include "errors.h"
#include "bytecode_loader.h"
#include "configuration.h"
#include "corevm/macros.h"
#include "dyobj/common.h"
#include "dyobj/errors.h"
#include "runtime/common.h"
#include "runtime/process.h"
#include "runtime/process_runner.h"

#include <boost/format.hpp>
#include <sneaker/utility/stack_trace.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

// -----------------------------------------------------------------------------

static void
try_get_attr_name(
  corevm::runtime::process& process,
  corevm::dyobj::attr_key attr_key,
  corevm::dyobj::dyobj_id id,
  std::string* attr_name)
{
  auto& obj = corevm::runtime::process::adapter(process).help_get_dyobj(id);

  corevm::runtime::compartment* compartment = nullptr;
  process.get_compartment(obj.closure_ctx().compartment_id, &compartment);

  if (compartment)
  {
    compartment->get_encoding_string((uint64_t)attr_key, attr_name);
  }
}

// -----------------------------------------------------------------------------

const unsigned int DEFAULT_STACK_LEVEL = 5;

// -----------------------------------------------------------------------------

static void
print_stack_trace()
{
  // TODO: [SNEAKER-86] Print stack trace not working well
  sneaker::utility::stack_trace::print_stack_trace(std::cerr, DEFAULT_STACK_LEVEL);
}

// -----------------------------------------------------------------------------

corevm::frontend::runner::runner(
  const std::string& path,
  corevm::frontend::configuration& configuration)
  :
  m_path(path),
  m_configuration(configuration)
{
}

// -----------------------------------------------------------------------------

int
corevm::frontend::runner::run() const noexcept
{
  // TODO: [COREVM-163] Refactor configuration default values ingestion

  uint64_t heap_alloc_size = m_configuration.heap_alloc_size() ? \
    m_configuration.heap_alloc_size() : corevm::dyobj::COREVM_DEFAULT_HEAP_SIZE;

  uint64_t pool_alloc_size = m_configuration.pool_alloc_size() ? \
    m_configuration.pool_alloc_size() : corevm::runtime::COREVM_DEFAULT_NATIVE_TYPES_POOL_SIZE;

  uint32_t gc_interval = m_configuration.gc_interval() ? \
    m_configuration.gc_interval() : corevm::runtime::COREVM_DEFAULT_GC_INTERVAL;

  corevm::runtime::process process(heap_alloc_size, pool_alloc_size);

  try
  {
    corevm::frontend::bytecode_loader::load(m_path, process);

    bool res = corevm::runtime::process_runner(process, gc_interval).start();

    if (!res)
    {
      std::cerr << "Run failed: " << strerror(errno) << std::endl;

#if __DEBUG__
      print_stack_trace();
#endif

      corevm::runtime::process::unwind_stack(process);

      return -1;
    }
  }
  catch (const corevm::dyobj::object_attribute_not_found_error& ex)
  {
    // Try to find the attribute name.
    std::string attr_name;
    try_get_attr_name(process, ex.attr_key, ex.id, &attr_name);

    if (!attr_name.empty())
    {
      THROW(corevm::dyobj::object_attribute_not_found_error(
        str(
          boost::format(
            "Attribute %s not found in object %#x"
          ) % attr_name.c_str() % ex.id
        )
      ));
    }
  }
  catch (const corevm::runtime_error& ex)
  {
    std::cerr << "Runtime error: " << ex.what() << std::endl;
    std::cerr << "Abort" << std::endl;

#if __DEBUG__
    print_stack_trace();
#endif

    corevm::runtime::process::unwind_stack(process);

    return -1;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    std::cerr << "Abort" << std::endl;

#if __DEBUG__
    print_stack_trace();
#endif

    corevm::runtime::process::unwind_stack(process);

    return -1;
  }
  catch (...)
  {
    std::cerr << "Unknown error" << std::endl;
    std::cerr << "Abort" << std::endl;

#if __DEBUG__
    print_stack_trace();
#endif

    corevm::runtime::process::unwind_stack(process);

    return -1;
  }

  return 0;
}

// -----------------------------------------------------------------------------
