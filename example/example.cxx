#include <iostream>

#include <wonder_rabbit_project/log.hxx>

auto main()
  -> int
  try
{
  using wonder_rabbit_project::log::log_t;
  using wonder_rabbit_project::log::log_line_t;
  using log_level = wonder_rabbit_project::log::level_e;
  using wonder_rabbit_project::log::to_string;
  using wonder_rabbit_project::log::hook_tie;
  
  log_t log;
  
  // set a `tie` type hook with hook_tie helper function.
  log.hook(hook_tie(std::cerr));
  
  /* // set a `tie` type hook with lambda-expression.
  log.hook
  ( [](log_line_t& a) -> log_line_t&
    {
      std::cerr << to_string(a);
      return a;
    }
  );
  */
  
  log() << "hello, " << "logging" << "!";
  
  log.default_level(log_level::debug);
  log() << "hello, logging with debug default level";
  
  log(log_level::debug) << "hi, level::debug";
  
  log(log_level::debug) << "easy: hello, debug";
  log(log_level::info)  << "easy: hello, info";
  log(log_level::warn)  << "easy: hello, warn";
  log(log_level::fatal) << "easy: hello, fatal";
  
}
catch( const std::exception& e)
{ std::cerr << "exception: " << e.what(); }
catch( ... )
{ std::cerr << "unknown exception\n"; }
