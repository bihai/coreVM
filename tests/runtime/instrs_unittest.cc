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
#include "../../include/dyobj/flags.h"
#include "../../include/runtime/closure.h"
#include "../../include/runtime/common.h"
#include "../../include/runtime/process.h"
#include "../../include/types/interfaces.h"
#include "../../include/types/native_type_handle.h"
#include "../../include/types/types.h"

#include <sneaker/testing/_unittest.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <list>


using corevm::runtime::process;

// -----------------------------------------------------------------------------

class instrs_unittest : public ::testing::Test
{
public:
  static const std::string DUMMY_PATH;

protected:
  corevm::runtime::closure_ctx m_ctx {
    .compartment_id = corevm::runtime::NONESET_COMPARTMENT_ID,
    .closure_id = corevm::runtime::NONESET_CLOSURE_ID
  };
};

// -----------------------------------------------------------------------------

const std::string instrs_unittest::DUMMY_PATH = "./my/path/file";

// -----------------------------------------------------------------------------

class instrs_obj_unittest : public instrs_unittest
{
protected:

  virtual void SetUp()
  {
    m_process.push_frame(*m_frame);
  }

  template<typename InstrHandlerCls>
  void execute_instr(corevm::runtime::instr instr, uint64_t expected_stack_size=1)
  {
    InstrHandlerCls instr_handler;

    instr_handler.execute(instr, m_process);

    uint64_t actual_stack_size = m_process.stack_size();

    ASSERT_EQ(expected_stack_size, actual_stack_size);
  }

  corevm::runtime::process m_process;
  corevm::runtime::frame* m_frame = new corevm::runtime::frame(m_ctx);
};

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrNEW)
{
  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  this->execute_instr<corevm::runtime::instr_handler_new>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrLDOBJ)
{
  corevm::runtime::compartment_id compartment_id = 0;
  corevm::runtime::closure_id closure_id = 10;
  corevm::runtime::closure_id parent_closure_id = 100;
  corevm::runtime::compartment compartment(DUMMY_PATH);
  corevm::runtime::vector vector;

  corevm::runtime::closure_ctx ctx1 {
    .compartment_id = compartment_id,
    .closure_id = closure_id
  };

  corevm::runtime::closure_ctx ctx2 {
    .compartment_id = compartment_id,
    .closure_id = parent_closure_id
  };

  corevm::runtime::closure closure {
    .id = closure_id,
    .parent_id = parent_closure_id,
    .vector = vector
  };

  corevm::runtime::closure parent_closure {
    .id = parent_closure_id,
    .parent_id = corevm::runtime::NONESET_CLOSURE_ID,
    .vector = vector
  };

  corevm::runtime::closure_table closure_table {
    closure,
    parent_closure
  };

  corevm::runtime::frame frame(ctx1);
  corevm::runtime::frame parent_frame(ctx2);
  compartment.set_closure_table(closure_table);

  m_process.insert_compartment(compartment);

  corevm::runtime::variable_key key = 123;
  corevm::dyobj::dyobj_id id = 456;

  parent_frame.set_visible_var(key, id);

  m_process.push_frame(parent_frame);
  m_process.push_frame(frame);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(key),
    .oprd2=0
  };

  execute_instr<corevm::runtime::instr_handler_ldobj>(instr, 1);

  ASSERT_EQ(id, m_process.top_stack());
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrSTOBJ)
{
  corevm::runtime::instr instr { .code=0, .oprd1=1, .oprd2=0 };

  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::dyobj::dyobj_id id = 1;
  m_process.push_stack(id);

  execute_instr<corevm::runtime::instr_handler_stobj>(instr, 0);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  ASSERT_TRUE(actual_frame.has_visible_var(1));

  corevm::dyobj::dyobj_id actual_id = actual_frame.get_visible_var(1);

  ASSERT_EQ(id, actual_id);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrGETATTR)
{
  corevm::dyobj::attr_key attr_key = 333;
  corevm::runtime::instr instr { .code=0, .oprd1=attr_key, .oprd2=0 };

  corevm::dyobj::dyobj_id id1 = process::adapter(m_process).help_create_dyobj();
  corevm::dyobj::dyobj_id id2 = process::adapter(m_process).help_create_dyobj();

  auto &obj = process::adapter(m_process).help_get_dyobj(id1);
  obj.putattr(attr_key, id2);
  m_process.push_stack(id1);

  execute_instr<corevm::runtime::instr_handler_getattr>(instr, 1);

  corevm::dyobj::dyobj_id expected_id = id2;
  corevm::dyobj::dyobj_id actual_id = m_process.top_stack();

  ASSERT_EQ(expected_id, actual_id);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrSETATTR)
{
  corevm::dyobj::attr_key attr_key = 789;
  corevm::runtime::instr instr { .code=0, .oprd1=attr_key, .oprd2=0 };

  corevm::dyobj::dyobj_id id1 = process::adapter(m_process).help_create_dyobj();
  corevm::dyobj::dyobj_id id2 = process::adapter(m_process).help_create_dyobj();

  m_process.push_stack(id1);
  m_process.push_stack(id2);

  execute_instr<corevm::runtime::instr_handler_setattr>(instr, 1);

  corevm::dyobj::dyobj_id expected_id = id1;
  corevm::dyobj::dyobj_id actual_id = m_process.top_stack();

  ASSERT_EQ(expected_id, actual_id);

  auto &obj = process::adapter(m_process).help_get_dyobj(actual_id);

  ASSERT_TRUE(obj.hasattr(attr_key));

  ASSERT_EQ(id2, obj.getattr(attr_key));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrDELATTR)
{
  corevm::dyobj::attr_key attr_key = 777;
  corevm::runtime::instr instr { .code=0, .oprd1=attr_key, .oprd2=0 };

  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  corevm::dyobj::dyobj_id attr_id = process::adapter(m_process).help_create_dyobj();

  auto& obj = process::adapter(m_process).help_get_dyobj(id);
  obj.putattr(attr_key, attr_id);

  ASSERT_TRUE(obj.hasattr(attr_key));

  m_process.push_stack(id);

  execute_instr<corevm::runtime::instr_handler_delattr>(instr, 1);

  corevm::dyobj::dyobj_id expected_id = id;
  corevm::dyobj::dyobj_id actual_id = m_process.top_stack();

  ASSERT_EQ(expected_id, actual_id);

  auto &actual_obj = process::adapter(m_process).help_get_dyobj(actual_id);

  ASSERT_FALSE(actual_obj.hasattr(attr_key));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrMUTE)
{
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  m_process.push_stack(id);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_mute>(instr, 1);

  corevm::dyobj::dyobj_id expected_id = id;
  corevm::dyobj::dyobj_id actual_id = m_process.top_stack();

  ASSERT_EQ(expected_id, actual_id);

  auto &actual_obj = process::adapter(m_process).help_get_dyobj(actual_id);

  ASSERT_EQ(false, actual_obj.get_flag(corevm::dyobj::DYOBJ_IS_IMMUTABLE));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrUNMUTE)
{
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  m_process.push_stack(id);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_unmute>(instr, 1);

  corevm::dyobj::dyobj_id expected_id = id;
  corevm::dyobj::dyobj_id actual_id = m_process.top_stack();

  ASSERT_EQ(expected_id, actual_id);

  auto &actual_obj = process::adapter(m_process).help_get_dyobj(actual_id);

  ASSERT_EQ(true, actual_obj.get_flag(corevm::dyobj::DYOBJ_IS_IMMUTABLE));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrPOP)
{
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  m_process.push_stack(id);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_pop>(instr, 0);

  ASSERT_THROW(
    {
      m_process.top_stack();
    },
    corevm::runtime::object_stack_empty_error
  );
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrLDOBJ2)
{
  corevm::runtime::compartment_id compartment_id = 0;
  corevm::runtime::closure_id closure_id = 10;
  corevm::runtime::closure_id parent_closure_id = 100;
  corevm::runtime::compartment compartment(DUMMY_PATH);
  corevm::runtime::vector vector;

  corevm::runtime::closure_ctx ctx1 {
    .compartment_id = compartment_id,
    .closure_id = closure_id
  };

  corevm::runtime::closure_ctx ctx2 {
    .compartment_id = compartment_id,
    .closure_id = parent_closure_id
  };

  corevm::runtime::closure closure {
    .id = closure_id,
    .parent_id = parent_closure_id,
    .vector = vector
  };

  corevm::runtime::closure parent_closure {
    .id = parent_closure_id,
    .parent_id = corevm::runtime::NONESET_CLOSURE_ID,
    .vector = vector
  };

  corevm::runtime::closure_table closure_table {
    closure,
    parent_closure
  };

  corevm::runtime::frame frame(ctx1);
  corevm::runtime::frame parent_frame(ctx2);

  compartment.set_closure_table(closure_table);

  m_process.insert_compartment(compartment);

  corevm::runtime::variable_key key = 123;
  corevm::dyobj::dyobj_id id = 456;

  parent_frame.set_invisible_var(key, id);

  m_process.push_frame(parent_frame);
  m_process.push_frame(frame);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(key),
    .oprd2=0
  };

  execute_instr<corevm::runtime::instr_handler_ldobj2>(instr, 1);

  ASSERT_EQ(id, m_process.top_stack());
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrSTOBJ2)
{
  corevm::runtime::instr instr { .code=0, .oprd1=1, .oprd2=0 };

  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::dyobj::dyobj_id id = 1;
  m_process.push_stack(id);

  execute_instr<corevm::runtime::instr_handler_stobj2>(instr, 0);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  ASSERT_TRUE(actual_frame.has_invisible_var(1));

  corevm::dyobj::dyobj_id actual_id = actual_frame.get_invisible_var(1);

  ASSERT_EQ(id, actual_id);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrDELOBJ)
{
  corevm::runtime::variable_key key = 1;
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(key),
    .oprd2=0
  };

  corevm::runtime::frame frame(m_ctx);
  frame.set_visible_var(key, id);
  m_process.push_frame(frame);

  execute_instr<corevm::runtime::instr_handler_delobj>(instr, 0);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  ASSERT_FALSE(actual_frame.has_visible_var(key));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrDELOBJ2)
{
  corevm::runtime::variable_key key = 1;
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(key),
    .oprd2=0
  };

  corevm::runtime::frame frame(m_ctx);
  frame.set_invisible_var(key, id);
  m_process.push_frame(frame);

  execute_instr<corevm::runtime::instr_handler_delobj2>(instr, 0);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  ASSERT_FALSE(actual_frame.has_invisible_var(key));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrGETHNDL)
{
  uint32_t expected_value = 123;

  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  m_process.push_stack(id);

  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::types::native_type_handle hndl = corevm::types::uint32(expected_value);
  corevm::dyobj::ntvhndl_key ntvhndl_key = m_process.insert_ntvhndl(hndl);

  auto &obj = process::adapter(m_process).help_get_dyobj(id);
  obj.set_ntvhndl_key(ntvhndl_key);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_gethndl>(instr, 1);

  corevm::runtime::frame& actual_frame = m_process.top_frame();
  corevm::types::native_type_handle actual_handle = actual_frame.pop_eval_stack();

  uint32_t actual_value = corevm::types::get_value_from_handle<uint32_t>(
    actual_handle
  );

  ASSERT_EQ(expected_value, actual_value);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrSETHNDL)
{
  uint32_t expected_value = 123;

  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  m_process.push_stack(id);

  corevm::runtime::frame frame(m_ctx);
  corevm::types::native_type_handle hndl = corevm::types::uint32(expected_value);
  frame.push_eval_stack(hndl);
  m_process.push_frame(frame);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_sethndl>(instr, 1);

  auto &obj = process::adapter(m_process).help_get_dyobj(id);

  ASSERT_NE(corevm::dyobj::NONESET_NTVHNDL_KEY, obj.get_ntvhndl_key());
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrCLRHNDL)
{
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  m_process.push_stack(id);

  corevm::types::native_type_handle hndl = corevm::types::uint32(123);
  corevm::dyobj::ntvhndl_key ntvhndl_key = m_process.insert_ntvhndl(hndl);

  auto &obj = process::adapter(m_process).help_get_dyobj(id);
  obj.set_ntvhndl_key(ntvhndl_key);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_clrhndl>(instr, 1);

  auto &obj2 = process::adapter(m_process).help_get_dyobj(id);
  ASSERT_EQ(corevm::dyobj::NONESET_NTVHNDL_KEY, obj2.get_ntvhndl_key());

  ASSERT_FALSE(m_process.has_ntvhndl(ntvhndl_key));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrOBJEQ)
{
  corevm::dyobj::dyobj_id id1 = 1;
  corevm::dyobj::dyobj_id id2 = 1;

  m_process.push_stack(id1);
  m_process.push_stack(id2);

  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_objeq>(instr, 0);

  corevm::runtime::frame& actual_frame = m_process.top_frame();
  corevm::types::native_type_handle result_handle = actual_frame.pop_eval_stack();

  bool expected_result = true;
  bool actual_result = corevm::types::get_value_from_handle<bool>(
    result_handle
  );

  ASSERT_EQ(expected_result, actual_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrOBJNEQ)
{
  corevm::dyobj::dyobj_id id1 = 1;
  corevm::dyobj::dyobj_id id2 = 2;

  m_process.push_stack(id1);
  m_process.push_stack(id2);

  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  execute_instr<corevm::runtime::instr_handler_objneq>(instr, 0);

  corevm::runtime::frame& actual_frame = m_process.top_frame();
  corevm::types::native_type_handle result_handle = actual_frame.pop_eval_stack();

  bool expected_result = true;
  bool actual_result = corevm::types::get_value_from_handle<bool>(
    result_handle
  );

  ASSERT_EQ(expected_result, actual_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_obj_unittest, TestInstrSETCXT)
{
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();

  m_process.push_stack(id);

  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::SETCTX,
    .oprd1 = static_cast<corevm::runtime::instr_oprd>(m_ctx.closure_id),
    .oprd2 = 0
  };

  execute_instr<corevm::runtime::instr_handler_setctx>(instr);

  corevm::dyobj::dyobj_id actual_id = m_process.top_stack();

  ASSERT_EQ(id, actual_id);

  auto &obj = process::adapter(m_process).help_get_dyobj(actual_id);

  const corevm::runtime::closure_ctx& ctx = obj.closure_ctx();

  ASSERT_EQ(m_ctx.compartment_id, ctx.compartment_id);
  ASSERT_EQ(m_ctx.closure_id, ctx.closure_id);
}

// -----------------------------------------------------------------------------

class instrs_functions_instrs_test : public instrs_unittest {
protected:
  corevm::runtime::process m_process;
};

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrPUTARG)
{
  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::dyobj::dyobj_id id = 1;
  m_process.push_stack(id);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  ASSERT_THROW(
    {
      actual_frame.pop_param();
    },
    corevm::runtime::missing_parameter_error
  );

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_putarg handler;
  handler.execute(instr, m_process);

  corevm::dyobj::dyobj_id actual_id = actual_frame.pop_param();

  ASSERT_EQ(id, actual_id);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrPUTKWARG)
{
  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::runtime::variable_key key = 3;
  corevm::dyobj::dyobj_id id = 2;
  m_process.push_stack(id);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  ASSERT_THROW(
    {
      actual_frame.pop_param_value_pair(key);
    },
    corevm::runtime::missing_parameter_error
  );

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(key),
    .oprd2=2
  };

  corevm::runtime::instr_handler_putkwarg handler;
  handler.execute(instr, m_process);

  corevm::dyobj::dyobj_id actual_id = actual_frame.pop_param_value_pair(key);

  ASSERT_EQ(id, actual_id);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrPUTARGS)
{
  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  auto& obj = process::adapter(m_process).help_get_dyobj(id);

  corevm::dyobj::dyobj_id id1 = process::adapter(m_process).help_create_dyobj();
  corevm::dyobj::dyobj_id id2 = process::adapter(m_process).help_create_dyobj();
  corevm::dyobj::dyobj_id id3 = process::adapter(m_process).help_create_dyobj();

  corevm::types::native_type_handle hndl = corevm::types::native_array {
    id1,
    id2,
    id3
  };

  auto key = m_process.insert_ntvhndl(hndl);
  obj.set_ntvhndl_key(key);

  m_process.push_stack(id);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_putargs handler;
  handler.execute(instr, m_process);

  corevm::dyobj::dyobj_id arg_id1 = actual_frame.pop_param();
  corevm::dyobj::dyobj_id arg_id2 = actual_frame.pop_param();
  corevm::dyobj::dyobj_id arg_id3 = actual_frame.pop_param();

  ASSERT_EQ(id1, arg_id1);
  ASSERT_EQ(id2, arg_id2);
  ASSERT_EQ(id3, arg_id3);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrPUTKWARGS)
{
  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  auto& obj = process::adapter(m_process).help_get_dyobj(id);

  corevm::dyobj::dyobj_id id1 = process::adapter(m_process).help_create_dyobj();
  corevm::dyobj::dyobj_id id2 = process::adapter(m_process).help_create_dyobj();
  corevm::dyobj::dyobj_id id3 = process::adapter(m_process).help_create_dyobj();

  corevm::runtime::variable_key key1 = 1;
  corevm::runtime::variable_key key2 = 2;
  corevm::runtime::variable_key key3 = 3;

  corevm::types::native_type_handle hndl = corevm::types::native_map {
    { key1, id1 },
    { key2, id2 },
    { key3, id3 }
  };

  auto key = m_process.insert_ntvhndl(hndl);
  obj.set_ntvhndl_key(key);

  m_process.push_stack(id);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_putkwargs handler;
  handler.execute(instr, m_process);

  corevm::dyobj::dyobj_id arg_id1 = actual_frame.pop_param_value_pair(key1);
  corevm::dyobj::dyobj_id arg_id2 = actual_frame.pop_param_value_pair(key2);
  corevm::dyobj::dyobj_id arg_id3 = actual_frame.pop_param_value_pair(key3);

  ASSERT_EQ(id1, arg_id1);
  ASSERT_EQ(id2, arg_id2);
  ASSERT_EQ(id3, arg_id3);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrGETARG)
{
  corevm::dyobj::dyobj_id id = 1;
  corevm::runtime::frame frame(m_ctx);
  frame.put_param(id);

  m_process.push_frame(frame);

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_getarg handler;
  handler.execute(instr, m_process);

  corevm::dyobj::dyobj_id actual_id = m_process.pop_stack();

  ASSERT_EQ(id, actual_id);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrGETKWARG)
{
  corevm::runtime::variable_key key = 22;
  corevm::dyobj::dyobj_id id = 100;

  corevm::runtime::frame frame(m_ctx);
  frame.put_param_value_pair(key, id);
  m_process.push_frame(frame);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(key),
    .oprd2=0
  };

  corevm::runtime::instr_handler_getkwarg handler;
  handler.execute(instr, m_process);

  corevm::dyobj::dyobj_id actual_id = m_process.pop_stack();

  ASSERT_EQ(id, actual_id);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrGETARGS)
{
  corevm::dyobj::dyobj_id id1 = 100;
  corevm::dyobj::dyobj_id id2 = 200;
  corevm::dyobj::dyobj_id id3 = 300;

  corevm::runtime::frame frame(m_ctx);
  frame.put_param(id1);
  frame.put_param(id2);
  frame.put_param(id3);
  m_process.push_frame(frame);

  ASSERT_EQ(true, frame.has_params());

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_getargs handler;
  handler.execute(instr, m_process);

  corevm::runtime::frame actual_frame = m_process.top_frame();

  ASSERT_EQ(false, actual_frame.has_params());

  corevm::types::native_type_handle hndl = actual_frame.pop_eval_stack();

  corevm::types::native_type_handle result_handle1;
  corevm::types::native_type_handle result_handle2;
  corevm::types::native_type_handle result_handle3;

  corevm::types::interface_array_back(hndl, result_handle3);
  corevm::types::interface_array_pop(hndl, hndl);

  corevm::types::interface_array_back(hndl, result_handle2);
  corevm::types::interface_array_pop(hndl, hndl);

  corevm::types::interface_array_back(hndl, result_handle1);
  corevm::types::interface_array_pop(hndl, hndl);

  corevm::dyobj::dyobj_id actual_id1 = corevm::types::get_value_from_handle<corevm::dyobj::dyobj_id>(result_handle1);
  corevm::dyobj::dyobj_id actual_id2 = corevm::types::get_value_from_handle<corevm::dyobj::dyobj_id>(result_handle2);
  corevm::dyobj::dyobj_id actual_id3 = corevm::types::get_value_from_handle<corevm::dyobj::dyobj_id>(result_handle3);

  ASSERT_EQ(id1, actual_id1);
  ASSERT_EQ(id2, actual_id2);
  ASSERT_EQ(id3, actual_id3);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_functions_instrs_test, TestInstrGETKWARGS)
{
  corevm::runtime::variable_key key1 = 1;
  corevm::runtime::variable_key key2 = 22;
  corevm::runtime::variable_key key3 = 333;

  corevm::dyobj::dyobj_id id1 = 100;
  corevm::dyobj::dyobj_id id2 = 200;
  corevm::dyobj::dyobj_id id3 = 300;

  corevm::runtime::frame frame(m_ctx);
  frame.put_param_value_pair(key1, id1);
  frame.put_param_value_pair(key2, id2);
  frame.put_param_value_pair(key3, id3);
  m_process.push_frame(frame);

  ASSERT_EQ(true, frame.has_param_value_pairs());

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_getkwargs handler;
  handler.execute(instr, m_process);

  corevm::runtime::frame actual_frame = m_process.top_frame();

  ASSERT_EQ(false, actual_frame.has_param_value_pairs());

  corevm::types::native_type_handle hndl = actual_frame.pop_eval_stack();

  corevm::types::native_type_handle key_handle1 = corevm::types::uint64(key1);
  corevm::types::native_type_handle key_handle2 = corevm::types::uint64(key2);
  corevm::types::native_type_handle key_handle3 = corevm::types::uint64(key3);
  corevm::types::native_type_handle result_handle1;
  corevm::types::native_type_handle result_handle2;
  corevm::types::native_type_handle result_handle3;

  corevm::types::interface_map_at(hndl, key_handle1, result_handle1);
  corevm::types::interface_map_at(hndl, key_handle2, result_handle2);
  corevm::types::interface_map_at(hndl, key_handle3, result_handle3);

  corevm::dyobj::dyobj_id actual_id1 = corevm::types::get_value_from_handle<corevm::dyobj::dyobj_id>(result_handle1);
  corevm::dyobj::dyobj_id actual_id2 = corevm::types::get_value_from_handle<corevm::dyobj::dyobj_id>(result_handle2);
  corevm::dyobj::dyobj_id actual_id3 = corevm::types::get_value_from_handle<corevm::dyobj::dyobj_id>(result_handle3);

  ASSERT_EQ(id1, actual_id1);
  ASSERT_EQ(id2, actual_id2);
  ASSERT_EQ(id3, actual_id3);
}

// -----------------------------------------------------------------------------

class instrs_runtime_instrs_test : public instrs_unittest
{
protected:
  corevm::runtime::process m_process;
};

// -----------------------------------------------------------------------------

TEST_F(instrs_runtime_instrs_test, TestInstrGC)
{
  process::adapter(m_process).help_create_dyobj();

  ASSERT_EQ(1, m_process.heap_size());

  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_gc handler;
  handler.execute(instr, m_process);

  ASSERT_EQ(0, m_process.heap_size());
}

// -----------------------------------------------------------------------------

TEST_F(instrs_runtime_instrs_test, TestInstrDEBUG)
{
  corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };
  corevm::runtime::instr_handler_debug handler;
  handler.execute(instr, m_process);
}

// -----------------------------------------------------------------------------

class instrs_control_instrs_test : public instrs_unittest
{
public:
  static bool signal_fired;

protected:
  virtual void SetUp()
  {
    corevm::runtime::vector vector {
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
      { .code=0, .oprd1=0, .oprd2=0 },
    };
    m_process.append_vector(vector);
  }

  corevm::runtime::process m_process;
};

// -----------------------------------------------------------------------------

bool instrs_control_instrs_test::signal_fired = false;

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrPINVK)
{
  corevm::dyobj::dyobj_id id = process::adapter(m_process).help_create_dyobj();
  auto& obj = process::adapter(m_process).help_get_dyobj(id);

  m_ctx.compartment_id = 1;
  m_ctx.closure_id = 2;
  obj.set_closure_ctx(m_ctx);

  m_process.push_stack(id);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=0,
    .oprd2=0
  };

  corevm::runtime::instr_handler_pinvk handler;
  handler.execute(instr, m_process);

  corevm::runtime::frame& frame = m_process.top_frame();

  corevm::runtime::closure_ctx ctx = frame.closure_ctx();

  ASSERT_TRUE(m_ctx == ctx);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrINVK)
{
  corevm::runtime::closure_id closure_id = 1;
  corevm::runtime::compartment_id compartment_id = 0;

  m_ctx.compartment_id = compartment_id;
  m_ctx.closure_id = closure_id;

  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::runtime::vector vector;
  corevm::runtime::closure closure {
    .id = closure_id,
    .parent_id = corevm::runtime::NONESET_CLOSURE_ID,
    .vector = vector
  };
  corevm::runtime::compartment compartment(DUMMY_PATH);
  corevm::runtime::closure_table closure_table {
    closure
  };

  compartment.set_closure_table(closure_table);
  m_process.insert_compartment(compartment);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=0,
    .oprd2=0
  };

  m_process.set_pc(8);

  corevm::runtime::instr_handler_invk handler;
  handler.execute(instr, m_process);

  corevm::runtime::frame& actual_frame = m_process.top_frame();

  ASSERT_EQ(m_process.pc(), actual_frame.get_return_addr());
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrRTRN)
{
  // TODO: [COREVM-49] Complete instruction set and implementations
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrJMP)
{
  corevm::runtime::frame frame(m_ctx);
  m_process.push_frame(frame);

  corevm::runtime::instr_addr current_addr = m_process.current_addr();
  ASSERT_EQ(0, current_addr);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(8),
    .oprd2=0
  };

  corevm::runtime::instr_handler_jmp handler;
  handler.execute(instr, m_process);

  current_addr = m_process.current_addr();

  ASSERT_EQ(8, current_addr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrJMPIF)
{
  corevm::runtime::frame frame(m_ctx);

  corevm::types::native_type_handle hndl = corevm::types::boolean(true);
  frame.push_eval_stack(hndl);
  m_process.push_frame(frame);

  corevm::runtime::instr_addr current_addr = m_process.current_addr();
  ASSERT_EQ(0, current_addr);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(8),
    .oprd2=0
  };

  corevm::runtime::instr_handler_jmpif handler;
  handler.execute(instr, m_process);

  current_addr = m_process.current_addr();

  ASSERT_EQ(8, current_addr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrJMPIF_OnFalseCondition)
{
  corevm::runtime::frame frame(m_ctx);

  corevm::types::native_type_handle hndl = corevm::types::boolean(false);
  frame.push_eval_stack(hndl);
  m_process.push_frame(frame);

  corevm::runtime::instr_addr current_addr = m_process.current_addr();
  ASSERT_EQ(0, current_addr);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=static_cast<corevm::runtime::instr_oprd>(8),
    .oprd2=0
  };

  corevm::runtime::instr_handler_jmpif handler;
  handler.execute(instr, m_process);

  current_addr = m_process.current_addr();

  ASSERT_EQ(0, current_addr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrEXC)
{
  // TODO: [COREVM-49] Complete instruction set and implementations
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrEXC2)
{
  // TODO: [COREVM-49] Complete instruction set and implementations
}

// -----------------------------------------------------------------------------

TEST_F(instrs_control_instrs_test, TestInstrEXIT)
{
  auto sig_handler = [](int signum) {
    instrs_control_instrs_test::signal_fired = true;
    signal(SIGTERM, SIG_IGN);
  };

  signal(SIGTERM, sig_handler);

  ASSERT_EQ(false, instrs_control_instrs_test::signal_fired);

  corevm::runtime::instr instr {
    .code=0,
    .oprd1=EXIT_SUCCESS,
    .oprd2=0
  };

  corevm::runtime::instr_handler_exit handler;
  handler.execute(instr, m_process);

  ASSERT_EQ(true, instrs_control_instrs_test::signal_fired);
}

// -----------------------------------------------------------------------------

class instrs_eval_stack_instrs_test : public instrs_unittest
{
public:
  typedef uint32_t IntrinsicType;

  typedef corevm::types::uint32 NativeType;

  typedef std::list<corevm::types::native_type_handle> eval_oprds_list;

protected:
  void push_eval_stack_and_frame(const eval_oprds_list& elements)
  {
    std::for_each(
      elements.begin(),
      elements.end(),
      [this](corevm::types::native_type_handle oprd) {
        this->m_frame->push_eval_stack(oprd);
      }
    );

    m_process.push_frame(*m_frame);
  }

  template<typename InstrHandlerCls, typename IntrinsicType=uint32_t>
  void execute_instr_and_assert_result(IntrinsicType expected_result)
  {
    InstrHandlerCls instr_handler;

    corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };

    instr_handler.execute(instr, m_process);

    corevm::runtime::frame& frame = m_process.top_frame();
    ASSERT_EQ(m_expected_eval_stack_size, frame.eval_stack_size());

    corevm::types::native_type_handle result_handle = frame.pop_eval_stack();

    IntrinsicType actual_result = corevm::types::get_value_from_handle<IntrinsicType>(
      result_handle
    );

    ASSERT_EQ(expected_result, actual_result);
  }

  template<typename InstrHandlerCls, typename IntrinsicType=uint32_t>
  void execute_instr_and_assert_result_equals_operand(const corevm::runtime::instr& instr)
  {
    InstrHandlerCls instr_handler;

    instr_handler.execute(instr, m_process);

    corevm::runtime::frame& frame = m_process.top_frame();
    ASSERT_EQ(m_expected_eval_stack_size, frame.eval_stack_size());

    corevm::types::native_type_handle result_handle = frame.pop_eval_stack();

    IntrinsicType expected_result = static_cast<IntrinsicType>(instr.oprd1);

    IntrinsicType actual_result = corevm::types::get_value_from_handle<IntrinsicType>(
      result_handle
    );

    ASSERT_EQ(expected_result, actual_result);
  }

  template<typename InstrHandlerCls, typename IntrinsicType>
  void execute_native_floating_type_creation_instr_and_assert_result(
    const corevm::runtime::instr& instr, IntrinsicType expected_result)
  {
    InstrHandlerCls instr_handler;

    instr_handler.execute(instr, m_process);

    corevm::runtime::frame& frame = m_process.top_frame();
    ASSERT_EQ(m_expected_eval_stack_size, frame.eval_stack_size());

    corevm::types::native_type_handle result_handle = frame.pop_eval_stack();

    IntrinsicType actual_result = corevm::types::get_value_from_handle<IntrinsicType>(
      result_handle
    );

    ASSERT_DOUBLE_EQ(expected_result, actual_result);
  }

  uint32_t m_expected_eval_stack_size = 1;
  corevm::runtime::process m_process;
  corevm::runtime::frame* m_frame = new corevm::runtime::frame(m_ctx);
};

// -----------------------------------------------------------------------------

class instrs_arithmetic_instrs_test : public instrs_eval_stack_instrs_test {};

// -----------------------------------------------------------------------------

class instrs_unary_arithmetic_instrs_test : public instrs_arithmetic_instrs_test
{
public:
  virtual void SetUp()
  {
    corevm::types::native_type_handle hndl = NativeType(m_oprd);
    push_eval_stack_and_frame(eval_oprds_list{hndl});
  }

protected:
  IntrinsicType m_oprd = 5;
};

// -----------------------------------------------------------------------------

TEST_F(instrs_unary_arithmetic_instrs_test, TestInstrPOS)
{
  execute_instr_and_assert_result<corevm::runtime::instr_handler_pos>(+m_oprd);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_unary_arithmetic_instrs_test, TestInstrNEG)
{
  execute_instr_and_assert_result<corevm::runtime::instr_handler_neg>(-m_oprd);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_unary_arithmetic_instrs_test, TestInstrINC)
{
  execute_instr_and_assert_result<corevm::runtime::instr_handler_inc>(++m_oprd);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_unary_arithmetic_instrs_test, TestInstrDEC)
{
  execute_instr_and_assert_result<corevm::runtime::instr_handler_dec>(--m_oprd);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_unary_arithmetic_instrs_test, TestInstrBNOT)
{
  execute_instr_and_assert_result<corevm::runtime::instr_handler_bnot>(~m_oprd);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_unary_arithmetic_instrs_test, TestInstrLNOT)
{
  execute_instr_and_assert_result<corevm::runtime::instr_handler_lnot>(!m_oprd);
}

// -----------------------------------------------------------------------------

class instrs_binary_arithmetic_instrs_test : public instrs_arithmetic_instrs_test
{
public:
  virtual void SetUp()
  {
    corevm::types::native_type_handle hndl1 = NativeType(m_oprd1);
    corevm::types::native_type_handle hndl2 = NativeType(m_oprd2);
    push_eval_stack_and_frame({hndl2, hndl1});
  }

protected:
  IntrinsicType m_oprd1 = 10;
  IntrinsicType m_oprd2 = 5;
};

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrADD)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_add>(m_oprd1 + m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrSUB)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_sub>(m_oprd1 - m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrMUL)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_mul>(m_oprd1 * m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrDIV)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_div>(m_oprd1 / m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrMOD)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_mod>(m_oprd1 % m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrPOW)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_pow>(pow(m_oprd1, m_oprd2));
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrBAND)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_band>(m_oprd1 & m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrBOR)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_bor>(m_oprd1 | m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrBXOR)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_bxor>(m_oprd1 ^ m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrBLS)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_bls>(m_oprd1 << m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrBRS)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_brs>(m_oprd1 >> m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrEQ)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_eq>(m_oprd1 == m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrNEQ)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_neq>(m_oprd1 != m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrGT)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_gt>(m_oprd1 > m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrLT)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_lt>(m_oprd1 < m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrGTE)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_gte>(m_oprd1 >= m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrLTE)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_lte>(m_oprd1 <= m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrLAND)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_land>(m_oprd1 && m_oprd2);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_binary_arithmetic_instrs_test, TestInstrLOR)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_lor>(m_oprd1 || m_oprd2);
}

// -----------------------------------------------------------------------------

class instrs_native_types_instrs_test : public instrs_eval_stack_instrs_test {};

// -----------------------------------------------------------------------------

class instrs_native_integer_type_creation_instrs_test : public instrs_native_types_instrs_test
{
public:
  virtual void SetUp()
  {
    push_eval_stack_and_frame(eval_oprds_list{});
  }
};

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrINT8)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::INT8,
    .oprd1 = std::numeric_limits<int8_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_int8, int8_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrUINT8)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::UINT8,
    .oprd1 = std::numeric_limits<uint8_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_uint8, uint8_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrINT16)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::INT16,
    .oprd1 = std::numeric_limits<int16_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand
    <corevm::runtime::instr_handler_int16, int16_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrUINT16)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::UINT16,
    .oprd1 = std::numeric_limits<uint16_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_uint16, uint16_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrINT32)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::INT32,
    .oprd1 = std::numeric_limits<int32_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_int32, int32_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrUINT32)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::UINT32,
    .oprd1 = std::numeric_limits<uint32_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_uint32, uint32_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrINT64)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::INT64,
    .oprd1 = std::numeric_limits<int64_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_int64, int64_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrUINT64)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::UINT64,
    .oprd1 = std::numeric_limits<uint64_t>::max(),
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_uint64, uint64_t>(instr);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_integer_type_creation_instrs_test, TestInstrBOOL)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::BOOL,
    .oprd1 = 1,
    .oprd2 = 0
  };

  execute_instr_and_assert_result_equals_operand<
    corevm::runtime::instr_handler_bool, bool>(instr);
}

// -----------------------------------------------------------------------------

class instrs_native_type_creation_instrs_test : public instrs_native_types_instrs_test
{
public:
  virtual void SetUp()
  {
    push_eval_stack_and_frame(eval_oprds_list{});
  }
};

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_creation_instrs_test, TestInstrDEC1)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::DEC1,
    .oprd1 = 12345,
    .oprd2 = 98760
  };

  execute_native_floating_type_creation_instr_and_assert_result<
    corevm::runtime::instr_handler_dec1, float>(instr, 12345.06789);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_creation_instrs_test, TestInstrDEC2)
{
  corevm::runtime::instr instr {
    .code = corevm::runtime::instr_enum::DEC2,
    .oprd1 = 1234567890,
    .oprd2 = 9876543210
  };

  execute_native_floating_type_creation_instr_and_assert_result<
    corevm::runtime::instr_handler_dec2, double>(instr, 1234567890.0123456789);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_creation_instrs_test, TestInstrSTR)
{
  corevm::types::native_string expected_result;
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_str, corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_creation_instrs_test, TestInstrARY)
{
  corevm::types::native_array expected_result;
  execute_instr_and_assert_result<corevm::runtime::instr_handler_ary,
    corevm::types::native_array>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_creation_instrs_test, TestInstrMAP)
{
  corevm::types::native_map expected_result;
  execute_instr_and_assert_result<corevm::runtime::instr_handler_map,
    corevm::types::native_map>(expected_result);
}

// -----------------------------------------------------------------------------

class instrs_native_type_conversion_instrs_test : public instrs_native_types_instrs_test
{
public:
  virtual void SetUp()
  {
    corevm::types::native_type_handle hndl = corevm::types::uint8(m_oprd);
    push_eval_stack_and_frame(eval_oprds_list{hndl});
  }

  template<typename InstrHandlerCls, typename TargetNativeType>
  void execute_instr_and_assert_result()
  {
    InstrHandlerCls instr_handler;

    corevm::runtime::instr instr { .code=0, .oprd1=0, .oprd2=0 };

    instr_handler.execute(instr, m_process);

    corevm::runtime::frame& frame = m_process.top_frame();

    ASSERT_EQ(m_expected_eval_stack_size, frame.eval_stack_size());

    corevm::types::native_type_handle hndl = m_oprd;

    auto expected_value = corevm::types::get_value_from_handle<
      typename TargetNativeType::value_type>(hndl);

    corevm::types::native_type_handle result_handle = frame.pop_eval_stack();

    auto actual_value = corevm::types::get_value_from_handle<
      typename TargetNativeType::value_type>(result_handle);

    ASSERT_EQ(expected_value, actual_value);
  }

private:
  corevm::types::uint8 m_oprd = std::numeric_limits<corevm::types::uint8::value_type>::max();
};

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2INT8)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2int8, corevm::types::int8>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2INT16)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2int16, corevm::types::int16>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2UINT16)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2uint16, corevm::types::uint16>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2INT32)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2int32, corevm::types::int32>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2UINT64)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2uint64, corevm::types::uint64>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2INT64)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2int64, corevm::types::int64>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2BOOL)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2bool, corevm::types::boolean>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2DEC1)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2dec1, corevm::types::decimal>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2DEC2)
{
  execute_instr_and_assert_result<
    corevm::runtime::instr_handler_2dec2, corevm::types::decimal2>();
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2STR)
{
  /* Lambda defined to avoid compilation error on the `ASSERT_THROW` macro.
   * (Somehow calling a templated function wrapped in a code block makes
   * `ASSERT_THROW` thinks it's getting an initializer list)
   **/
  auto test = [&]() {
    execute_instr_and_assert_result<
      corevm::runtime::instr_handler_2str, corevm::types::string>();
  };

  ASSERT_THROW(
    {
      test();
    },
    corevm::types::conversion_error
  );
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2ARY)
{
  /* Lambda defined to avoid compilation error on the `ASSERT_THROW` macro.
   * (Somehow calling a templated function wrapped in a code block makes
   * `ASSERT_THROW` thinks it's getting an initializer list)
   **/
  auto test = [&]() {
    execute_instr_and_assert_result<
      corevm::runtime::instr_handler_2ary, corevm::types::array>();
  };

  ASSERT_THROW(
    {
      test();
    },
    corevm::types::conversion_error
  );
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_type_conversion_instrs_test, TestInstr2MAP)
{
  /* Lambda defined to avoid compilation error on the `ASSERT_THROW` macro.
   * (Somehow calling a templated function wrapped in a code block makes
   * `ASSERT_THROW` thinks it's getting an initializer list)
   **/
  auto test = [&]() {
    execute_instr_and_assert_result<
      corevm::runtime::instr_handler_2map, corevm::types::map>();
  };

  ASSERT_THROW(
    {
      test();
    },
    corevm::types::conversion_error
  );
}

// -----------------------------------------------------------------------------

class instrs_native_type_complex_instrs_test : public instrs_native_types_instrs_test
{
public:
  virtual void SetUp()
  {
    // Override this method to do nothing here.
  }
};

// -----------------------------------------------------------------------------

class instrs_native_string_type_complex_instrs_test : public instrs_native_type_complex_instrs_test {};

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRLEN)
{
  corevm::types::native_string hello_world = "Hello world!";
  corevm::types::native_string::size_type expected_result = hello_world.size();
  corevm::types::native_type_handle oprd = corevm::types::string(hello_world);

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strlen,
    corevm::types::native_string::size_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRCLR)
{
  corevm::types::native_string hello_world = "Hello world!";
  corevm::types::native_string expected_result = "";
  corevm::types::native_type_handle oprd = hello_world;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strclr,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRAPD)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::native_string exclamation = "!";
  corevm::types::native_string expected_result = "Hello world!";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = exclamation;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strapd,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRPSH)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::int8 exclamation = static_cast<corevm::types::int8::value_type>('!');
  corevm::types::native_string expected_result = "Hello world!";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = exclamation;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strpsh,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRIST)
{
  corevm::types::native_string hello_world = "Helloworld";
  corevm::types::int8 pos = static_cast<corevm::types::int8::value_type>(5);
  corevm::types::native_string space = " ";
  corevm::types::native_string expected_result = "Hello world";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = pos;
  corevm::types::native_type_handle oprd3 = space;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strist,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRIST2)
{
  corevm::types::native_string hello_world = "Helloworld";
  corevm::types::int8 pos = static_cast<corevm::types::int8::value_type>(5);
  corevm::types::int8 space = static_cast<corevm::types::int8::value_type>(' ');
  corevm::types::native_string expected_result = "Hello world";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = pos;
  corevm::types::native_type_handle oprd3 = space;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strist2,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRERS)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::int8 pos = static_cast<corevm::types::int8::value_type>(5);
  corevm::types::native_string expected_result = "Hello";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = pos;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strers,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRERS2)
{
  corevm::types::native_string hello_world = "Hello  world";
  corevm::types::int8 pos = static_cast<corevm::types::int8::value_type>(5);
  corevm::types::int8 len = static_cast<corevm::types::int8::value_type>(1);
  corevm::types::native_string expected_result = "Hello world";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = pos;
  corevm::types::native_type_handle oprd3 = len;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strers2,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRRPLC)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::int8 pos = static_cast<corevm::types::int8::value_type>(5);
  corevm::types::int8 len = static_cast<corevm::types::int8::value_type>(6);
  corevm::types::native_string exclamation = "!!!!!!";
  corevm::types::native_string expected_result = "Hello!!!!!!";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = pos;
  corevm::types::native_type_handle oprd3 = len;
  corevm::types::native_type_handle oprd4 = exclamation;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3, oprd4});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strrplc,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRSWP)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::native_string okla = "OKLA!";
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = okla;
  corevm::types::native_string expected_result = okla;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strswp,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRSUB)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::int8 pos = 6;
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = pos;
  corevm::types::native_string expected_result = "world";

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strsub,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrSTRSUB2)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::int8 pos = 6;
  corevm::types::int8 len = 2;
  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = pos;
  corevm::types::native_type_handle oprd3 = len;
  corevm::types::native_string expected_result = "wo";

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strsub2,
    corevm::types::native_string>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrFND)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::native_string word = "world";
  corevm::types::native_string::size_type expected_result = 6;

  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = word;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strfnd,
    corevm::types::native_string::size_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrFND2)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::native_string word = "world";
  corevm::types::uint32 pos = 6;
  corevm::types::native_string::size_type expected_result = 6;

  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = word;
  corevm::types::native_type_handle oprd3 = pos;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strfnd2,
    corevm::types::native_string::size_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrRFND)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::native_string word = "world";
  corevm::types::native_string::size_type expected_result = 6;

  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = word;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strrfnd,
    corevm::types::native_string::size_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_string_type_complex_instrs_test, TestInstrRFND2)
{
  corevm::types::native_string hello_world = "Hello world";
  corevm::types::native_string word = "world";
  corevm::types::uint32 pos = 6;
  corevm::types::native_string::size_type expected_result = 6;

  corevm::types::native_type_handle oprd1 = hello_world;
  corevm::types::native_type_handle oprd2 = word;
  corevm::types::native_type_handle oprd3 = pos;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_strrfnd2,
    corevm::types::native_string::size_type>(expected_result);
}

// -----------------------------------------------------------------------------

class instrs_native_array_type_complex_instrs_test : public instrs_native_type_complex_instrs_test {};

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYLEN)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::native_array::size_type expected_result = array.size();
  corevm::types::native_type_handle oprd = array;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_arylen,
    corevm::types::native_array::size_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYEMP)
{
  corevm::types::native_array array { 1, 2, 3 };
  bool expected_result = false;
  corevm::types::native_type_handle oprd = array;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_aryemp, bool>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYAT)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::uint32 index = 1;
  corevm::types::native_array::value_type expected_result = 2;

  corevm::types::native_type_handle oprd1 = array;
  corevm::types::native_type_handle oprd2 = index;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_aryat,
    corevm::types::native_array::value_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYFRT)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::native_array::value_type expected_result = 1;
  corevm::types::native_type_handle oprd = array;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_aryfrt,
    corevm::types::native_array::value_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYBAK)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::native_array::value_type expected_result = 3;
  corevm::types::native_type_handle oprd = array;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_arybak,
    corevm::types::native_array::value_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYAPND)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::uint64 data = 4;
  corevm::types::native_array expected_result { 1, 2, 3, 4 };

  corevm::types::native_type_handle oprd1 = array;
  corevm::types::native_type_handle oprd2 = data;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_aryapnd,
    corevm::types::native_array>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYPOP)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::native_array expected_result { 1, 2 };
  corevm::types::native_type_handle oprd = array;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_arypop,
    corevm::types::native_array>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYSWP)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::native_array other { 4, 5, 6 };
  corevm::types::native_array expected_result = other;

  corevm::types::native_type_handle oprd1 = array;
  corevm::types::native_type_handle oprd2 = other;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_aryswp,
    corevm::types::native_array>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_array_type_complex_instrs_test, TestInstrARYCLR)
{
  corevm::types::native_array array { 1, 2, 3 };
  corevm::types::native_array expected_result;

  corevm::types::native_type_handle oprd = array;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_aryclr,
    corevm::types::native_array>(expected_result);
}

// -----------------------------------------------------------------------------

class instrs_native_map_type_complex_instrs_test : public instrs_native_type_complex_instrs_test {};

// -----------------------------------------------------------------------------

TEST_F(instrs_native_map_type_complex_instrs_test, TestInstrMAPLEN)
{
  corevm::types::native_map map {
    { 1, 1 }
  };

  corevm::types::native_map::size_type expected_result = 1;
  corevm::types::native_type_handle oprd = map;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_maplen,
    corevm::types::native_map::size_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_map_type_complex_instrs_test, TestInstrMAPEMP)
{
  corevm::types::native_map map {
    { 1, 1 }
  };

  bool expected_result = false;
  corevm::types::native_type_handle oprd = map;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_mapemp, bool>(
    expected_result
  );
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_map_type_complex_instrs_test, TestInstrMAPAT)
{
  corevm::types::native_map map {
    { 1, 100 }
  };

  corevm::types::uint64 key = 1;
  corevm::types::native_map::mapped_type expected_result = 100;
  corevm::types::native_type_handle oprd1 = map;
  corevm::types::native_type_handle oprd2 = key;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_mapat,
    corevm::types::native_map::mapped_type>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_map_type_complex_instrs_test, TestInstrMAPPUT)
{
  corevm::types::native_map map;
  corevm::types::uint64 key = 1;
  corevm::types::uint64 value = 11;
  corevm::types::native_map expected_result;
  expected_result[1] = 11;

  corevm::types::native_type_handle oprd1 = map;
  corevm::types::native_type_handle oprd2 = key;
  corevm::types::native_type_handle oprd3 = value;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2, oprd3});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_mapput,
    corevm::types::native_map>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_map_type_complex_instrs_test, TestInstrMAPERS)
{
  corevm::types::native_map map {
    { 1, 11 },
    { 2, 22 },
  };

  corevm::types::uint64 key = 1;
  corevm::types::native_map expected_result;
  expected_result[2] = 22;

  corevm::types::native_type_handle oprd1 = map;
  corevm::types::native_type_handle oprd2 = key;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_mapers,
    corevm::types::native_map>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_map_type_complex_instrs_test, TestInstrMAPCLR)
{
  corevm::types::native_map map {
    { 1, 11 },
    { 2, 222 },
  };

  corevm::types::native_map expected_result;
  corevm::types::native_type_handle oprd = map;

  push_eval_stack_and_frame(eval_oprds_list{oprd});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_mapclr,
    corevm::types::native_map>(expected_result);
}

// -----------------------------------------------------------------------------

TEST_F(instrs_native_map_type_complex_instrs_test, TestInstrMAPSWP)
{
  corevm::types::native_map map {
    { 1, 11 },
    { 2, 22 },
  };

  corevm::types::native_map other {
    { 1, 22 },
    { 2, 11 },
  };

  corevm::types::native_map expected_result {
    { 1, 22 },
    { 2, 11 },
  };

  corevm::types::native_type_handle oprd1 = map;
  corevm::types::native_type_handle oprd2 = other;

  push_eval_stack_and_frame(eval_oprds_list{oprd1, oprd2});

  execute_instr_and_assert_result<corevm::runtime::instr_handler_mapswp,
    corevm::types::native_map>(expected_result);
}

// -----------------------------------------------------------------------------
