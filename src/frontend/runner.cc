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
#include "../../include/frontend/runner.h"

#include "../../include/errors.h"
#include "../../include/frontend/bytecode_loader.h"
#include "../../include/frontend/configuration.h"
#include "../../include/runtime/process.h"
#include "../../include/runtime/process_runner.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>


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
  // TODO: [COREVM-158] Fill in configuration values to process
  corevm::runtime::process process;

  try
  {
    corevm::frontend::bytecode_loader::load(m_path, process);

    bool res = corevm::runtime::process_runner(process).start();

    if (!res)
    {
      std::cerr << "Run failed: " << strerror(errno) << std::endl;
      return -1;
    }
  }
  catch (const corevm::runtime_error& ex)
  {
    std::cerr << "Runtime error: " << ex.what() << std::endl;
    std::cerr << "Abort" << std::endl;
    return -1;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    std::cerr << "Abort" << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cerr << "Unknown error" << std::endl;
    std::cerr << "Abort" << std::endl;
    return -1;
  }

  return 0;
}

// -----------------------------------------------------------------------------