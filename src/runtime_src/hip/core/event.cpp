// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024 Advanced Micro Device, Inc. All rights reserved.

#include "event.h"

namespace xrt::core::hip {
event::
event()
//  : ctype{type::event}
{}

void
event::
record(std::shared_ptr<stream> s)
{
  cstream = std::move(s);
  auto ev = std::dynamic_pointer_cast<event>(command_cache.get(static_cast<command_handle>(this)));
  throw_invalid_handle_if(!ev, "event passed is invalid");
  if (is_recorded()) {
    // already recorded
    cstream->erase_cmd(ev);
  }
  // update recorded commands list
  cstream->enqueue_event(std::move(ev));
  set_state(state::recorded);
  //return true;
}

bool
event::
is_recorded()
{
  return get_state() >= command::state::recorded;
}

bool
event::
query()
{
  for (auto it = recorded_commands.begin(); it != recorded_commands.end(); it++){
    state command_state = (*it)->get_state();
    if (command_state != state::completed){
      return false;
    }
  }
  return true;
}

bool
event::
synchronize()
{
  for (auto it = recorded_commands.begin(); it != recorded_commands.end(); it++){
    state command_state = (*it)->get_state();
    if (command_state < state::completed){
      (*it)->wait();
      (*it)->set_state(state::completed);
    }
  }
  set_state(state::success);
  for (auto it = chain_of_commands.begin(); it != chain_of_commands.end(); it++){
    (*it)->submit(true);
  }
  return true;
}

bool
event::
wait()
{
  return synchronize();
}

bool
event::
submit(bool)
{
  return true;
}

std::shared_ptr<stream>
event::
get_stream()
{
  return cstream;
}

void
event::
add_to_chain(std::shared_ptr<command> cmd)
{
  // lock and add
}

void
event::
add_dependency(std::shared_ptr<command> cmd)
{
  // lock and add
}

kernel_start::
kernel_start(std::shared_ptr<stream>&& s, std::shared_ptr<function> &&f, void** args)
  : command(std::move(s))
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