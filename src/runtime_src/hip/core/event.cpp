// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024 Advanced Micro Device, Inc. All rights reserved.

#include "event.h"

namespace xrt::core::hip {
bool
event::
synchronize()
{
  for(auto it = recorded_commands.begin(); it != recorded_commands.end(); it++){
    state command_state = (*it)->get_state();
    while(command_state != state::completed){
      command_state = (*it)->get_state();
    }
  }
  set_state(state::success);
  for(auto it = chain_of_commands.begin(); it != chain_of_commands.end(); it++){
    (*it)->submit(true);//not sure if true as input is fine. temporary!
  }
  return true;
}

bool
event::
wait()
{
  return synchronize();
}

kernel_start::
kernel_start(std::shared_ptr<stream>&& s, std::shared_ptr<function> &&f, void** args)
{
    //xrt::kernel k = f->get_kernel();
    xrt::kernel k; // just for compilation purpose we have to get it from function.
    const auto& m_arginfo = std::move(xrt_core::kernel_int::get_args(k));
    size_t idx = 0;
    for (auto itr = m_arginfo.begin(); itr != m_arginfo.end(); ++itr, ++idx) {
        xrt_core::kernel_int::set_arg_at_index(r, (*itr)->index, args[idx], (*itr)->size);
    }
    r = xrt::run(k);
}

bool
kernel_start::
submit(bool)
{
  state kernel_start_state = get_state();
  if(kernel_start_state == state::init)
  {
    r.start();
    set_state(state::running);
    return true;
  }
  return false;
}

bool
kernel_start::
wait()
{
  state kernel_start_state = get_state();
  if(kernel_start_state == state::running)
  {
    r.wait();
    set_state(state::completed);
    return true;
  }
  return false;
}

bool
copy_buffer::
submit(bool)
{
    return true; //temporary
}

bool
copy_buffer::
wait()
{
    return true; //temporary
}

// Global map of commands
xrt_core::handle_map<command_handle, std::shared_ptr<command>> command_cache;

} // xrt::core::hip
