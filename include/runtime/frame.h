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
#ifndef COREVM_FRAME_H_
#define COREVM_FRAME_H_

#include "common.h"
#include "errors.h"
#include "../../include/dyobj/dyobj_id.h"
#include "../../include/types/native_type_handle.h"

#include <cstdint>
#include <list>
#include <stack>


namespace corevm {


namespace runtime {


/**
 * Each frame is supposed to have:
 *
 * - Return address.
 * - Pointer to the address caller frame.
 * - Pointer to frame with parent scope (if has one).
 * - Visible local variables.
 * - Invisible local variables.
 * - Parameter list (args).
 * - Optional parameter <-> default value mapping (kwargs).
 * - Evaluation stack.
 * - Compartment ID.
 * - Closure ID.
 */
class frame {
public:
  explicit frame();
  ~frame();

  uint32_t eval_stack_size() const;

  corevm::runtime::instr_addr get_start_addr() const;

  void set_start_addr(const corevm::runtime::instr_addr);

  corevm::runtime::instr_addr get_return_addr() const;

  void set_return_addr(const corevm::runtime::instr_addr);

  const corevm::runtime::frame* get_parent_scope_frame_ptr() const;

  void set_parent_scope_frame_ptr(corevm::runtime::frame*);

  void push_eval_stack(corevm::types::native_type_handle&);

  corevm::types::native_type_handle pop_eval_stack()
    throw(corevm::runtime::evaluation_stack_empty_error);

  bool has_visible_var(const corevm::runtime::variable_key) const;

  corevm::dyobj::dyobj_id get_visible_var(const corevm::runtime::variable_key)
    const throw(corevm::runtime::local_variable_not_found_error);

  corevm::dyobj::dyobj_id pop_visible_var(const corevm::runtime::variable_key)
    throw(corevm::runtime::local_variable_not_found_error);

  void set_visible_var(corevm::runtime::variable_key, corevm::dyobj::dyobj_id);

  bool has_invisible_var(const corevm::runtime::variable_key) const;

  corevm::dyobj::dyobj_id get_invisible_var(const corevm::runtime::variable_key)
    const throw(corevm::runtime::local_variable_not_found_error);

  corevm::dyobj::dyobj_id pop_invisible_var(const corevm::runtime::variable_key)
    throw(corevm::runtime::local_variable_not_found_error);

  void set_invisible_var(corevm::runtime::variable_key, corevm::dyobj::dyobj_id);

  bool has_params() const;

  void put_param(const corevm::dyobj::dyobj_id&);

  const corevm::dyobj::dyobj_id pop_param() throw(corevm::runtime::missing_parameter_error);

  bool has_param_value_pairs() const;

  void put_param_value_pair(const corevm::runtime::variable_key, const corevm::dyobj::dyobj_id&);

  const corevm::dyobj::dyobj_id pop_param_value_pair(const corevm::runtime::variable_key)
    throw(corevm::runtime::missing_parameter_error);

  std::list<corevm::runtime::variable_key> param_value_pair_keys() const;

  std::list<corevm::dyobj::dyobj_id> get_visible_objs() const;

  std::list<corevm::dyobj::dyobj_id> get_invisible_objs() const;

  const corevm::runtime::closure_id closure_id() const;

  void set_closure_id(corevm::runtime::closure_id);

  const corevm::runtime::compartment_id compartment_id() const;

  void set_compartment_id(corevm::runtime::compartment_id);

protected:
  corevm::runtime::compartment_id m_compartment_id;
  corevm::runtime::closure_id m_closure_id;
  corevm::runtime::instr_addr m_start_addr;
  corevm::runtime::instr_addr m_return_addr;
  corevm::runtime::frame* m_parent_scope_frame_ptr;
  std::unordered_map<corevm::runtime::variable_key, corevm::dyobj::dyobj_id> m_visible_vars;
  std::unordered_map<corevm::runtime::variable_key, corevm::dyobj::dyobj_id> m_invisible_vars;
  std::list<corevm::dyobj::dyobj_id> m_params_list;
  std::unordered_map<corevm::runtime::variable_key, corevm::dyobj::dyobj_id> m_param_value_map;
  std::stack<corevm::types::native_type_handle> m_eval_stack;
};


}; /* end namespace runtime */


}; /* end namespace corevm */


#endif /* COREVM_FRAME_H_ */
