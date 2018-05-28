/*
 * Hera VM: eWASM virtual machine conforming to the Ethereum VM C API
 *
 * Copyright (c) 2016 Alex Beregszaszi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __VM_H
#define __VM_H

#include "hera.h"
#include "eei.h" /* for ExecutionResult */

#include <evmc/evmc.h>

using namespace std;
using namespace HeraVM;

/*
 * Base class for WASM execution engines.
 */
class WasmEngine
{
public:
  WasmEngine(wasm_vm const _vm, 
    vector<uint8_t> const& _code, 
    vector<uint8_t> const& _state_code,
    evmc_message const& _msg,
    evmc_context *_context,
    bool _meterGas,
    DebugStream & _debug
  ):
    vm(_vm),
    code(_code),
    state_code(_state_code),
    msg(_msg),
    context(_context),
    output(_msg.gas),
    meterGas(_meterGas),
    hera_debug(_debug)
  { }
 
 /* 
  * Pure virtual method implemented by child classes of WasmEngine
  * Calling this should trigger execution of the wasm code provided in the constructor
  *   and instantiation of an EEI object.
  */
  virtual int execute() = 0;

 /*
  * Method releasing execution output data. 
  * This should not be called until execution has occurred.
  */
  ExecutionResult & getResult() { return output; }

protected:
  wasm_vm vm;
  vector<uint8_t> code;
  vector<uint8_t> state_code;
  evmc_message msg;
  evmc_context *context;
  ExecutionResult output;
  bool meterGas;
  DebugStream & hera_debug;
};

/*
 * Binaryen interpreter class called by default. 
 */
class BinaryenVM : public WasmEngine
{
public:
  BinaryenVM(vector<uint8_t> const& _code,
    vector<uint8_t> const& _state_code,
    evmc_message const& _msg,
    evmc_context *_context,
    bool _meterGas,
    DebugStream _debug) : 
    WasmEngine(VM_BINARYEN, _code, _state_code, _msg, _context, _meterGas, _debug)
    { }
  
  int execute() override;
};

#if WABT_SUPPORTED
/*
 * WABT interpreter.
 */
class WabtVM : public WasmEngine
{
public:
  WabtVM(vector<uint8_t> const& _code,
    vector<uint8_t> const& _state_code,
    evmc_message const& msg,
    evmc_context *_context,
    bool _meterGas,
    DebugStream & _debug) :
    WasmEngine(VM_WABT, _code, _state_code, _msg, _context, _meterGas, _debug)
    { }

  int execute() override;
};
#endif

#if WAVM_SUPPORTED
/*
 * WAVM JIT compiler.
 */
class WavmVM : public WasmEngine
{
public:
  WavmVM(vector<uint8_t> const& _code,
    vector<uint8_t> const& _state_code,
    evmc_message const& msg,
    evmc_context *_context,
    bool _meterGas,
    DebugStream & _debug) :
    WasmEngine(VM_WAVM, _code, _state_code, _msg, _context, _meterGas, _debug)
    { }

  int execute() override;
};
#endif

#endif