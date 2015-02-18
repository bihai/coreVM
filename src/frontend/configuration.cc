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
#include "../../include/frontend/configuration.h"

#include "../../include/frontend/errors.h"

#include <boost/format.hpp>
#include <sneaker/json/json.h>
#include <sneaker/json/json_schema.h>

#include <fstream>
#include <ios>
#include <sstream>
#include <string>


using sneaker::json::JSON;


// -----------------------------------------------------------------------------

const std::string corevm::frontend::configuration::schema = \
  "{"
    "\"type\": \"object\","
    "\"properties\": {"
      "\"alloc-size\": {"
        "\"type\": \"integer\""
      "}"
    "}"
  "}";

// -----------------------------------------------------------------------------

corevm::frontend::configuration::configuration(const std::string& path)
  :
  m_path(path)
{
}

// -----------------------------------------------------------------------------

void
corevm::frontend::configuration::load_config(JSON* res) const
  throw(corevm::frontend::configuration_loading_error)
{
  std::ifstream fs(m_path, std::ios::binary);
  std::stringstream buffer;

  try
  {
    fs.exceptions(std::ios::failbit | std::ios::badbit);
    buffer << fs.rdbuf();
    fs.close();
  }
  catch (const std::ios_base::failure& ex)
  {
    throw corevm::frontend::configuration_loading_error(
      str(
        boost::format(
          "Error loading configuration file %s: %s"
        ) % m_path % ex.what()
      )
    );
  }

  std::string content = buffer.str();

  JSON content_json;

  try
  {
    content_json = sneaker::json::parse(content);
  }
  catch (const sneaker::json::invalid_json_error& ex)
  {
    throw corevm::frontend::configuration_loading_error(
      str(
        boost::format(
          "Error parsing configuration file %s: %s"
        ) % m_path % ex.what()
      )
    );
  }

  JSON schema_json = sneaker::json::parse(configuration::schema);

  try
  {
    sneaker::json::json_schema::validate(content_json, schema_json);
  }
  catch (const sneaker::json::json_validation_error& ex)
  {
    throw corevm::frontend::configuration_loading_error(
      str(
        boost::format(
          "Invalid configuration file %s: %s"
        ) % m_path % ex.what()
      )
    );
  }

  *res = content_json;
}

// -----------------------------------------------------------------------------
