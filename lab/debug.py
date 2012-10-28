#!/usr/bin/env python
import os, sys, readline

sys.path.insert(0,
  '/System/Library/PrivateFrameworks/LLDB.framework/Resources/Python')
  #'/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Resources/Python')
import lldb

debugger = None
cli = None

def breakpoint_function_wrapper(frame, bp_loc, dict):
  print 'breakpoint_function_wrapper'

def disassemble_instructions(insts):
  for i in insts:
    print i

def get_stopped_threads(process, reason):
  '''Returns the thread(s) with the specified stop reason in a list.
  The list can be empty if no such thread exists.
  '''
  threads = []
  for t in process:
    if t.GetStopReason() == reason:
      threads.append(t)
  return threads

def rl_completer():
  # cli.HandleCompletion(self, str current_line, uint32_t cursor_pos, int match_start_point, 
  #   int max_return_elements, SBStringList matches) -> int
  pass

def init_cli():
  readline.set_completer(rl_completer)
  readline.parse_and_bind('tab: complete')
  readline.parse_and_bind('set editing-mode emacs')
  readline.parse_and_bind('set bind-tty-special-chars on')
  readline.parse_and_bind('set print-completions-horizontally on')
  # Get the SBCommandInterpreter
  global debugger, cli
  cli = debugger.GetCommandInterpreter()

def cli_run_command(command):
  global cli
  res = lldb.SBCommandReturnObject()
  cli.HandleCommand(command, res)
  if not res.Succeeded():
    print res.GetError()
  else:
    print res.GetOutput()
  return res

def enter_interactive_mode():
  global cli
  while True:
    try:
      line = raw_input('(lldb) ')
    except EOFError:
      print ''
      break
    except KeyboardInterrupt:
      print ''
      os._exit(1) # instant death
    if line == 'quit' or line == 'q':
      break
    cli_run_command(line)

if __name__ == '__main__':
  # Create a new debugger instance in your module if your module
  # can be run from the command line. When we run a script from
  # the command line, we won't have any debugger object in
  # lldb.debugger, so we can just create it if it will be needed
  debugger = lldb.SBDebugger.Create()

  # When we step or continue, don't return from the function until the process 
  # stops. Otherwise we would have to handle the process events ourselves which,
  # while doable is a little tricky. We do this by setting the async mode to
  # false.
  debugger.SetAsync(False)

  # Create a target from a file and arch
  target_path = sys.argv[1]
  target_args = sys.argv[2:]
  target = debugger.CreateTargetWithFileAndArch(target_path,
                                                lldb.LLDB_ARCH_DEFAULT)
  if not target:
    print 'Failed to create target'
    sys.exit(1)
  else:
    # If the target is valid set a breakpoint at main
    #main_bp = target.BreakpointCreateByName("main", target.GetExecutable().GetFilename());
    #print main_bp

    # Build env list (LaunchSimple needs env to be a list, not a map)
    env = []
    for k in os.environ:
      env.append(k + '=' + os.environ[k])

    # Find TTY name
    import subprocess
    ttyname = subprocess.check_output('tty').strip()

    # Launch the process. Since we specified synchronous mode, we won't return
    # from this function until we hit the breakpoint at main
    # process = target.LaunchSimple(target_args, env, os.getcwd())
    launch_error = lldb.SBError()
    process = target.Launch(debugger.GetListener(), None, None,
                            ttyname, ttyname, ttyname,
                            os.getcwd(), 0, False, launch_error)

    # Make sure the launch went ok
    if process:
      # Print some simple process info
      state = process.GetState()
      if state == lldb.eStateStopped or state == lldb.eStateCrashed:
        print process

        # Get the (first) thread which faulted
        thread = None
        for t in process:
          if t.GetStopReason() == lldb.eStopReasonException:
            thread = t
            break

        if thread:
          # Print some simple thread info
          print thread
          # Get the first frame
          frame = thread.GetFrameAtIndex(0)
          if frame:
            # Print some simple frame info
            #print frame
            function = frame.GetFunction()
            # See if we have debug info (a function)
            if function:
              pass
              # We do have a function, print some info for the function
              #print function
              # Now get all instructions for this function and print them
              #insts = function.GetInstructions(target)
              #disassemble_instructions(insts)
            else:
              # See if we have a symbol in the symbol table for where we stopped
              symbol = frame.GetSymbol();
              if symbol:
                # We do have a symbol, print some info for the symbol
                print symbol

          init_cli()
          cli_run_command('disassemble --pc -m -C 3')
          enter_interactive_mode()

# PYTHONPATH=/System/Library/PrivateFrameworks/LLDB.framework/Resources/Python
