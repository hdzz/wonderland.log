#include <iostream>

// use NDEBUG macro trick if you want.
#ifdef NDEBUG
  // to disable logger if define it.
  //   recommend: use macro usage, then disable all of features define it only.
  #define WRP_WONDERLAND_LOG_DISABLE
#endif

#include <wonder_rabbit_project/wonderland/log.hxx>

auto main()
  -> int
  try
{
  using namespace wonder_rabbit_project::wonderland;
  
  // you can get logger instance reference object if you want.
  //   but, it is not necessary if use macro.
  //   - merit of use macro: easy for use, and easy for enable/disable logger switching.
  //   - demerit           : evil.
  // auto& log = log::log_t::instance();
  
  // set a `tee` type hook with hook_tie helper function.
  LOG_HOOK( log::hook_tee( std::cerr ) );
  
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
  LOG << "hello, " << "LOG ( logger address is " << std::hex << &LOG_INSTANCE << " ) " << "!";
  
  // change default level
  LOG_DEFAULT_LEVEL( log::level::debug );
  LOG << "hello, logging with debug default level";
  
  // not throw if fatal ( default: true )
  LOG_THROW_IF_FATAL( false );
  LOG << "LOG_THROW_IF_FATAL: " << std::boolalpha << LOG_THROW_IF_FATAL();
  
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
  
  {
    struct A
    {
      A()  { LOGD << "hello A :)"; }
      ~A() { LOGD << "bye A -)"; }
    } a;
  }
  
  [](){ LOGD << "hello lambda-expression!"; }();
  
  // destruct hook
  LOG_AT_DESTRUCT
  ( []( const log::log_t::data_type& data )
    {
      std::cerr << "===== at_destruct =====\n";
      auto data_reverse = data;
      data_reverse.reverse();
      for ( const auto& line : data_reverse )
        std::cerr << line.to_string();
    }
  );
  
  LOG_THROW_IF_FATAL( true );
  LOG << "LOG_THROW_IF_FATAL: " << std::boolalpha << LOG_THROW_IF_FATAL();
  LOGF << "＼(^o^)／";
}
catch( const std::exception& e )
{ std::cerr << "exception: " << e.what(); }
catch( ... )
{ std::cerr << "unknown exception\n"; }