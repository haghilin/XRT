// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2024 Advanced Micro Device, Inc. All rights reserved.
#ifndef xrthip_event_h
#define xrthip_event_h

#include "common.h"
#include "module.h"
#include "stream.h"
#include "xrt/xrt_kernel.h"
#include "core/common/api/kernel_int.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

namespace xrt::core::hip {

// command_handle - opaque command handle
using command_handle = void*;

class command
{
public:
  enum class state : uint8_t
  {
    init,
    recorded,
    running,
    completed,
    success,
    error,
    abort
  };

  enum class type : uint8_t
  {
    event,
    buffer_copy,
    kernel_start
  };

private:
  std::shared_ptr<stream> cstream;
  uint64_t ctime;
  type ctype;
  state cstate;

public:
  virtual bool submit(bool) = 0;
  virtual bool wait() = 0;
  state get_state() { return cstate; }
  void set_state(state newstate) { cstate = newstate; };

};

class event : public command
{
private:
  std::shared_ptr<stream> m_stream;
  std::vector<std::shared_ptr<command>> recorded_commands;
  std::vector<std::shared_ptr<command>> chain_of_commands;

public:
  event(std::shared_ptr<stream>&& s);

  void
  record();

  bool submit(bool) override = 0;
  bool wait() override;
  bool synchronize();

  bool is_recorded();

  std::shared_ptr<stream>
  get_stream();

  void
  add_to_chain(std::shared_ptr<command> cmd);

  void
  add_dependency(std::shared_ptr<command>&& cmd);
};

class kernel_start : public command
{
private:
  std::shared_ptr<function> func;
  xrt::run r;

public:
  kernel_start(std::shared_ptr<stream>&& s, std::shared_ptr<function> &&f, void** args);
  bool submit(bool) override;
  bool wait() override;
};

class copy_buffer : public command
{
public:
  bool submit(bool) override;
  bool wait() override;

};

// Global map of commands
extern xrt_core::handle_map<command_handle, std::shared_ptr<command>> command_cache;

} // xrt::core::hip

#endif
