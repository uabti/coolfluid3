// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Python_ScriptEngine_hpp
#define cf3_Python_ScriptEngine_hpp

////////////////////////////////////////////////////////////////////////////////

#if MAC_FRAMEWORK_PYTHON
#include <Python/Python.h>
#else
#include <Python.h>
#endif
//python Py_ssize_t backward compatibility

#if (PY_VERSION_HEX < 0x02050000)
typedef int Py_ssize_t;
#endif


////////////////////////////////////////////////////////////////////////////////


#include "common/Component.hpp"

#include "python/LibPython.hpp"

#include "common/PE/Manager.hpp"

#include "common/CommonAPI.hpp"

#include <frameobject.h>

#include <vector>
#include <string>

namespace cf3 {
namespace python {

////////////////////////////////////////////////////////////////////////////////

/// @brief python tracing function, used to
int python_trace(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg);

/// @brief Threaded c function, used to contain the PyEval_CodeEval
void python_execute_function();

/// @brief Executes python scripts passed as a string
///
/// Exposes an execute_script signal, taking as single argument the string containing the python script to run
/// @author Bart Janssens
class Python_API ScriptEngine : public common::Component {
public: // functions

  /// Contructor
  /// @param name of the component
  ScriptEngine ( const std::string& name );

  /// Virtual destructor
  virtual ~ScriptEngine();

  /// Get the class name
  static std::string type_name () { return "ScriptEngine"; }

  /// Execute the script passed as a string
  void execute_script(std::string script,int code_fragment);

  /// Signal to execute a script
  void signal_execute_script(common::SignalArgs& node);

  /// Signal to change the current debugging state
  void signal_change_debug_state(common::SignalArgs& node);

  /// Signal to retrieve the completion list
  void signal_completion(common::SignalArgs& node);

  /// Signal to get the documentation of the symbol under the mouse in the gui
  void signal_get_documentation(common::SignalArgs& node);

  /// Called by the trace function when a new line is reached
  int new_line_reached(int code_fragment,int line_number);

  void check_python_change(int code_fragment);

  void no_more_instruction();

private:
  enum debug_command {
    INVALID=-1,
    STOP=0,
    CONTINUE=1,
    LINE_BY_LINE_EXECUTION=2,
    NORMAL_EXECUTION=3,
    BREAK=4,
    TOGGLE_BREAK_POINT=5
  };

  class PythonDictEntry{
  public:
    PythonDictEntry(){}
    PythonDictEntry(const PythonDictEntry &entry)
      :py_ref(entry.py_ref)
      ,name(entry.name)
      ,value(entry.value)
      ,entry_list(entry.entry_list){}

    PyObject *py_ref;
    std::string name;
    std::string value;
    std::vector<PythonDictEntry> entry_list;
  };

  /// Signature for the execute_script signal
  void signature_execute_script(common::SignalArgs& node);

  void emit_output(std::string output);

  void emit_debug_trace(int fragment,int line);

  void emit_completion_list(std::vector<std::string> *add, std::vector<std::string> *sub);

  void emit_documentation(std::string doc);

  void check_scope_difference(PythonDictEntry &entry,std::string name,std::vector<std::string> *add, std::vector<std::string> *sub,int rec = 0,bool child=false);

  void flush_python_stdout(int code_fragment);

  std::string current_instruction;
  bool new_command;
  Handle< common::PE::Manager > m_manager;
  PythonDictEntry local_scope_entry;
  debug_command interpreter_mode;
  std::vector<std::vector<bool> >break_points;
  static int python_close;
  int break_fragment;
  int break_line;
  bool stoped;
}; // ScriptEngine

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_ScriptEngine_hpp
