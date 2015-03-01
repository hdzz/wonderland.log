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
{
  using namespace wonder_rabbit_project::wonderland;
  
  // you can get logger instance reference object if you want.
  //   but, it is not necessary if use macro.
  //   - merit of use macro: easy for use, and easy for enable/disable logger switching.
  //   - demerit           : evil.
  // auto& log = log::log_t::instance();
  
  // set hook as like a `tee` with make_string_output_hook helper function.
  //   log_t has one of std::cerr output hook for default construct.
  //   the type of hooks is std::vector< hook_t >
  //   and the type of hook_t is std::function< auto ( log::log_line_t&) -> void >.
  //LOG_HOOKS( { log::make_string_output_hook( std::ofstream( "log.txt" ) ) } );
  
  // set hook as like a `tee` with lambda-expression.
  //HOG_HOOKS( { []( log::log_line_t& log_line ) { std::ofstream( "log.txt" ) << log_line; } } );
  
  // logging with default level
  //   - LOG[D,I,W,E,F] generate an ostream object then put any stringable object.
  //   - LOG[D,I,W,E,F] is not defined if define WRP_WONDERLAND_LOG_NO_MACRO
  //   - LOG[D,I,W,E,F] to nothing if define WRP_WONDERLAND_LOG_DISABLE
  LOG << "hello, " << "LOG ( logger address is " << std::hex << &LOG_INSTANCE << " ) " << "!";
  
  // change default level
  LOG_DEFAULT_LEVEL( log::level::debug );
  LOG << "hello, logging with debug default level";
  
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
  LOG_AT_DESTRUCT( []{ LOGD << "===== LOG_AT_DESTRUCT =====\n"; } );
  
  LOG_IF_FATAL( log::if_fatal::exit );
  LOG << "LOG_EXIT_IF_FATAL: " << LOG_IF_FATAL();
  
  LOGF << "＼(^o^)／";
}