#include <iostream>

#include <wonder_rabbit_project/wonderland/log.hxx>

auto main()
  -> int
  try
{
  using namespace wonder_rabbit_project::wonderland;
  
  auto& log = log::log_t::instance();
  
  // set a `tee` type hook with hook_tie helper function.
  log.hook( log::hook_tie( std::cerr ) );
  
  // set a `tee` type hook with lambda-expression.
  //log.hook
  //( []( log_line_t& a )
  //    -> log_line_t&
  //  {
  //    std::cerr << to_string( a );
  //    return a;
  //  }
  //);
  
  // logging with default level
  //   - LOG[D,I,W,E,F] generate an ostream object then put any stringable object.
  //   - LOG[D,I,W,E,F] is not defined if define WRP_WONDERLAND_LOG_NO_MACRO
  //   - LOG[D,I,W,E,F] to nothing if define WRP_WONDERLAND_LOG_DISABLE
  LOG << "hello, " << "LOG ( logger address is " << std::hex << &log << " ) " << "!";
  
  // change default level
  log.default_level( log::level::debug );
  LOG << "hello, logging with debug default level";
  
  // not throw if fatal ( default: true )
  log.throw_if_fatal( false );
  
  // logging with spcialized level
  LOGD << "hello, LOGD macro";
  LOGI << "hello, LOGI macro";
  LOGW << "hello, LOGW macro";
  LOGE << "hello, LOGE macro";
  LOGF << "hello, LOGF macro";
  // you can use logger directly if you not want to use macros.
  /*
  log( log::level::debug ) << __FILE__ << " " << __LINE__ << " " << "hello, debug";
  log( log::level::info  ) << __FILE__ << " " << __LINE__ << " " << "hello, info";
  log( log::level::warn  ) << __FILE__ << " " << __LINE__ << " " << "hello, warn";
  log( log::level::error ) << __FILE__ << " " << __LINE__ << " " << "hello, error";
  log( log::level::fatal ) << __FILE__ << " " << __LINE__ << " " << "hello, fatal";
  */
  
  // destruct hook
  log.at_destruct
  ( []( const log::log_t::data_type& data )
    {
      std::cerr << "===== at_destruct =====\n";
      auto data_reverse = data;
      data_reverse.reverse();
      for ( const auto& line : data_reverse )
        std::cerr << line.to_string();
    }
  );
  
  log.throw_if_fatal( true );
  LOGF << "＼(^o^)／";
}
catch( const std::exception& e )
{ std::cerr << "exception: " << e.what(); }
catch( ... )
{ std::cerr << "unknown exception\n"; }