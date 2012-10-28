notes

variables are just names for program fragments
ns rsms.datastore

The actor model consists of a few key principles:

- No shared state
- Lightweight processes
- Asynchronous message-passing
- Mailboxes to buffer incoming messages
- Mailbox processing with pattern matching

Some key assumptions are built into this model:

- Processes are cheap (in memory) and fast to create
- Processes are small and thus large numbers of processes can be created
- Processes are not bound to kernel threads and can be efficiently scheduled in user space
- The scheduler requires the ability to pause and continue process execution
- Messages can be retrieved by efficiently pattern-matching messages in the mailbox


The Erlang view of the world can be summarized in the following statements:

- Everything is a process.
- Processes are strongly isolated.
- Process creation and destruction is a lightweight operation.
- Message passing is the only way for processes to interact.
- Processes have unique names.
- If you know the name of a process you can send it a message. • Processes share no resources.
- Error handling is non-local.
- Processes do what they are supposed to do or fail.

Erlang has three ways of spawning processes:

1. I don't care if my child process dies:
    spawn(...)

2. I want to crash if my child process crashes:
    spawn_link(...)

3. I want to receive a message if my child process terminates (normally or not):
    process_flag(trap_exit, true),
    spawn_link(...)



local linda = lanes.linda()

local function loop( max )
  for i=1,max do
      print( "sending: "..i )
      linda:send( "x", i )    -- linda as upvalue
  end
end

a= lanes.gen("",loop)( 10000 )

while true do
  local val= linda:receive( 3.0, "x" )    -- timeout in seconds 
  if val==nil then
      print( "timed out" )
      break
  end
  print( "received: "..val )
end

producer count =
  for n in [1..count]
    number n -> parent

main =
  producer_task = spawn producer 1000
  receive
    producer_task ->
      | number n
        print "Received number: " n
        receive
