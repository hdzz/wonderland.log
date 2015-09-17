#pragma once

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
  #undef NOMINMAX
  #undef WIN32_LEAN_AND_MEAN
#endif

#include <cstdint>
#include <string>
#include <array>
#include <forward_list>
#include <sstream>
#include <memory>
#include <utility>
#include <chrono>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <ctime>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <cmath>

#ifndef WRP_WONDERLAND_LOG_NO_MACRO
#ifndef WRP_WONDERLAND_LOG_DISABLE

#if defined( __Clang__ ) || defined( __GNUC__ )
#define WRP_WONDERLAND_LOG_FUNCTION __PRETTY_FUNCTION__
#else
#define WRP_WONDERLAND_LOG_FUNCTION __func__
#endif

#define WRP_WONDERLAND_LOG_SOURCE_PARAMS __FILE__, __LINE__, WRP_WONDERLAND_LOG_FUNCTION

#ifndef LOG_INSTANCE
  #define LOG_INSTANCE wonder_rabbit_project::wonderland::log::log_t::instance()
#endif

#define LOG  LOG_INSTANCE( WRP_WONDERLAND_LOG_SOURCE_PARAMS )
#define LOGD LOG_INSTANCE( wonder_rabbit_project::wonderland::log::level::debug, WRP_WONDERLAND_LOG_SOURCE_PARAMS )
#define LOGI LOG_INSTANCE( wonder_rabbit_project::wonderland::log::level::info , WRP_WONDERLAND_LOG_SOURCE_PARAMS )
#define LOGW LOG_INSTANCE( wonder_rabbit_project::wonderland::log::level::warn , WRP_WONDERLAND_LOG_SOURCE_PARAMS )
#define LOGE LOG_INSTANCE( wonder_rabbit_project::wonderland::log::level::error, WRP_WONDERLAND_LOG_SOURCE_PARAMS )
#define LOGF LOG_INSTANCE( wonder_rabbit_project::wonderland::log::level::fatal, WRP_WONDERLAND_LOG_SOURCE_PARAMS )

#define LOG_DEFAULT_LEVEL( a ) LOG_INSTANCE.default_level( a )
#define LOG_KEEP_LEVEL( a )    LOG_INSTANCE.keep_level( a )

#define LOG_HOOKS( a )       LOG_INSTANCE.hooks( a )
#define LOG_AT_DESTRUCT( a ) LOG_INSTANCE.at_destruct( a )

#define LOG_IF_FATAL( a )        LOG_INSTANCE.if_fatal( a )
#define LOG_START_TIME( a )      LOG_INSTANCE.start_time( a )
#define LOG_TIME_APPEARANCE( a ) LOG_INSTANCE.time_appearance( a )
#define LOG_TIME_FORMAT( a )     LOG_INSTANCE.time_format( a )

#else

#ifndef LOG_INSTANCE
  #define LOG_INSTANCE wonder_rabbit_project::wonderland::log::log_t::instance()
#endif

#define LOG  wonder_rabbit_project::wonderland::log::null_logger_t()
#define LOGD wonder_rabbit_project::wonderland::log::null_logger_t()
#define LOGI wonder_rabbit_project::wonderland::log::null_logger_t()
#define LOGW wonder_rabbit_project::wonderland::log::null_logger_t()
#define LOGE wonder_rabbit_project::wonderland::log::null_logger_t()
#define LOGF wonder_rabbit_project::wonderland::log::null_logger_t()

#define LOG_DEFAULT_LEVEL( a ) nullptr
#define LOG_KEEP_LEVEL( a )    nullptr

#define LOG_HOOK( a )        nullptr
#define LOG_AT_DESTRUCT( a ) nullptr

#define LOG_IF_FATAL( a )        nullptr
#define LOG_START_TIME( a )      nullptr
#define LOG_TIME_APPEARANCE( a ) nullptr
#define LOG_TIME_FORMAT( a )     nullptr

#endif
#endif

namespace
{
#ifdef WRP_WONDERLAND_LOG_DECLARE_ONLY
  extern
#endif
  std::mutex write_mutex;
}

namespace wonder_rabbit_project
{
  namespace wonderland
  {
    namespace log
    {
      enum class time_format
      {
        gmt
        , jst
      };
      
      namespace detail
      {
#ifdef _WIN32
        struct windows_high_performance_clock
        {
          using rep = long long;
          using period = std::pico;
          using duration = std::chrono::duration< rep, period >;
          using time_point = std::chrono::time_point< windows_high_performance_clock >;
          static const bool is_steady = true;
          static time_point now()
          {
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency( &frequency );
            
            LARGE_INTEGER count;
            QueryPerformanceCounter( &count );
            return time_point( duration( count.QuadPart * static_cast< rep >( period::den ) / frequency.QuadPart ) );
          }
         };
#endif
        template < typename T >
        inline static auto to_time_t( const T& t )
        {
          using std::chrono::system_clock;
          using std::chrono::duration_cast;
          const auto& dt = t - T::clock::now();
          auto et = system_clock::now() + duration_cast<system_clock::duration>( dt );
          return std::time_t( system_clock::to_time_t( et ) );
        }

#ifdef _WIN32
          // TDM-GCC-5.1.0 is not support %F and %T
          //   note: mingw is supported. but we cannot predicate TDM or not.
        constexpr auto format_date_time_gmt = "%Y-%m-%dT%H:%M:%SZ";
        constexpr auto format_date_time     = "%Y-%m-%dT%H:%M:%S";
#else
        constexpr auto format_date_time_gmt = "%FT%TZ";
        constexpr auto format_date_time     = "%FT%T";
#endif
        constexpr auto format_time_zone = "%H:%M";
        
        template < typename T >
        inline static auto to_string_iso8601_gmt ( const T& t = T::clock::now() )
        {
          using namespace std::chrono;
        
          const time_t& ct = detail::to_time_t( t );
        
          std::string r = "0000-00-00T00:00:00Z.";
          std::strftime ( const_cast< char* > ( r.data( ) ), r.size( ), format_date_time_gmt, std::gmtime ( &ct ) );
        
          return r;
        }
        
        template < typename T = std::chrono::minutes >
        inline static auto time_zone_difference()
        {
          std::time_t current_time = time(0);
          std::tm     local_tm     = *gmtime( &current_time );
          std::time_t utc_time     = mktime( &local_tm );
          return std::chrono::duration_cast< T >( std::chrono::duration< double >( difftime( current_time, utc_time ) ) );
        }
        
        template < typename T >
        inline static auto to_string_iso8601_jst ( const T& t = T::clock::now() )
        {
          using namespace std::chrono;
          
          const time_t ct = detail::to_time_t( t );
          
          std::string r = "0000-00-00T00:00:00.";
          std::strftime ( const_cast< char* > ( r.data( ) ), r.size( ), format_date_time, std::localtime ( &ct ) );
          
          const auto z = time_zone_difference();
          
          std::stringstream rz;
          rz
            << ( std::signbit( z.count() ) ? "-" : "+" )
            << std::setw( 2 ) << std::setfill( '0' )
            << std::to_string( std::abs( duration_cast<hours>(z).count() ) )
            << ":"
            << std::setw( 2 ) << std::setfill( '0' )
            << std::to_string( std::abs( duration_cast<minutes>(z).count() ) % 60 )
            ;
          
          return std::string( r.data(), r.data() + r.size() - 1 ) + rz.str();
        }
        
        template < typename T >
        inline static auto to_string_iso8601 ( const T& t = T::clock::now(), const time_format f = time_format::gmt )
        {
          switch ( f )
          {
            case time_format::jst:
              return to_string_iso8601_jst( t );
            case time_format::gmt:
            default: 
              return to_string_iso8601_gmt( t );
          }
        }
      }

      using clock_t =
#ifdef WRP_WONDERLAND_LOG_CLOCK
        WRP_WONDERLAND_LOG_CLOCK
#elif defined( _WIN32 )
        detail::windows_high_performance_clock
#else
        std::chrono::high_resolution_clock
#endif
        ;
      
      enum class level
        : std::uint8_t
      {
        none  = 0
        , debug = 1
        , info  = 2
        , warn  = 3
        , error = 4
        , fatal = 5
      };
      
      template < class T = void >
      auto to_string ( level ) -> std::string;
      template < class T = void >
      auto operator<< ( std::ostream& o, level ) -> std::ostream&;
      
      enum class if_fatal
        : std::uint8_t
      {
        none
        , exit
        , quick_exit
        , exception
      };
      
      template < class T = void >
      auto to_string ( if_fatal ) -> std::string;
      template < class T = void >
      auto operator<< ( std::ostream& o, if_fatal ) -> std::ostream&;
      
      enum class time_appearance
      {
        f64_in_seconds_from_run
        , i64_in_seconds_from_epoch
      };

      struct fatal_exception
        : public std::runtime_error
      {
        fatal_exception ( const std::string& message )
          : std::runtime_error ( message )
        { }
      };
      
      class log_stream_t;
      
      struct log_line_t
      {
        std::chrono::time_point< clock_t > time;
        log::level    level;
        std::string   source_file;
        std::uint32_t source_line;
        std::string   source_function;
        std::string   message;
      };
      
      template < class T = void >
      auto to_string ( const log_line_t& ) -> std::string;
      template < class T = void >
      auto operator<< ( std::ostream& o, const log_line_t& ) -> std::ostream&;
      
      class log_t final
      {
        friend class  log_stream_t;
        friend struct log_line_t;
        
      public:
        using hook_type          = std::function < auto ( log_line_t& ) -> void >;
        using hooks_type         = std::vector< hook_type >;
        using destruct_hook_type = std::function < auto ( ) -> void >;
        
        class log_stream_t
        {
          log_t& _master;
          const level  _level;
          const std::string&  _source_file;
          const std::uint32_t _source_line;
          const std::string&  _source_function;
          const std::shared_ptr<std::ostringstream> _stream;
          
        public:
          template<class T>
          auto operator<< ( const T& value )
          -> log_stream_t&
          {
            ( *_stream ) << value;
            return *this;
          }
          
          explicit log_stream_t
          ( log_t&              master
            , const level         level
            , const std::string&& source_file
            , const std::uint32_t source_line
            , const std::string&& source_function
          )
            : _master ( master )
            , _level ( level )
            , _source_file ( std::move ( source_file ) )
            , _source_line ( source_line )
            , _source_function ( std::move ( source_function ) )
            , _stream ( new std::ostringstream() )
          { }
          
          ~log_stream_t()
          {
            try
            {
              _master._append
              ( { clock_t::now()
                  , _level
                  , _source_file
                  , _source_line
                  , _source_function
                  , _stream -> str()
                }
              );
            }
            catch ( const std::exception& e )
            {
              std::cerr
                  << "[WARNING] occurred exception, but here is a destructor of log_stream_t class."
                  " then the exception reserve the next log_t any function call timing.\n"
                  "  exception type name: " << typeid ( e ).name() << "\n"
                  "  exception what     : " << e.what() << "\n"
                  ;
#ifndef EMSCRIPTEN
              _master._exception_ptr = std::current_exception();
#else
              _master._pseudo_exception_ptr.reset ( new fatal_exception ( e.what() ) );
#endif
            }
            catch ( ... )
            {
              std::cerr
                  << "[WARNING] occurred exception, but here is a destructor of log_stream_t class."
                  " then the exception reserve the next log_t any function call timing.\n"
                  "  exception type name: ... \n"
                  "  exception what     : \n"
                  ;
#ifndef EMSCRIPTEN
              _master._exception_ptr = std::current_exception();
#else
              _master._pseudo_exception_ptr.reset ( new fatal_exception ( "..." ) );
#endif
            }
          }
        };
        
      private:
      
        level   _default_level;
        level   _keep_level;
        
        hooks_type         _hooks;
        destruct_hook_type _at_destruct_hook;
        
        const clock_t::time_point _start_time;
        log::time_appearance _time_appearance;
        log::time_format     _time_format;
        
        log::if_fatal      _if_fatal;
#ifndef EMSCRIPTEN
        std::exception_ptr _exception_ptr;
#else
        std::unique_ptr< fatal_exception > _pseudo_exception_ptr;
#endif
        
        std::function < auto ( log_line_t&& line ) -> void > _append;
        
        log_t( )
          : _default_level ( level::info )
          , _keep_level ( level::debug )
          , _hooks
            (
        {
          [ ] ( log_line_t& log_line )
          {
            std::lock_guard<std::mutex> l ( write_mutex );
            std::cerr << log_line;
          }
        }
          )
        , _at_destruct_hook ( [ ]( ) { } )
        , _start_time ( clock_t::now() )
        , _time_appearance ( log::time_appearance::f64_in_seconds_from_run )
        , _time_format ( log::time_format::gmt )
        , _if_fatal ( log::if_fatal::none )
#ifndef EMSCRIPTEN
        , _exception_ptr ( nullptr )
#else
        , _pseudo_exception_ptr ( nullptr )
#endif
        {
          const auto append_default = [ this ]
                                      ( log_line_t&& line )
          {
            this -> rethrow();
            
            if ( std::uint8_t ( line.level ) >= std::uint8_t ( this -> _keep_level ) )
              for ( const auto& hook : this -> _hooks )
                hook ( line );
                
            if ( line.level == level::fatal )
            {
              std::cerr << "[WARNING] detect a `fatal` level log and the if_fatal flag is ";
              
              switch ( this -> _if_fatal )
              {
                case log::if_fatal::none:
                  std::cerr << " `if_fatal::none` then nothing to do.\n";
                  break;
                  
                case log::if_fatal::exit:
                  std::cerr << " `if_fatal::exit` then call std::exit( EXIT_FAILURE ) now.\n";
                  std::exit ( EXIT_FAILURE );
                  
                case log::if_fatal::quick_exit:
                  std::cerr << " `if_fatal::quick_exit` then call std::quick_exit( EXIT_FAILURE ) now.\n";
#if defined( EMSCRIPTEN ) || defined( __MINGW32__ )
                  std::cerr
                      << "[WARNING] Emscripten or MINGW is not support std::quick_exit yet,"
                      " then call std::exit alternatively.\n"
                      ;
                  std::exit ( EXIT_FAILURE );
#else
                  std::quick_exit ( EXIT_FAILURE );
#endif
                  
                case log::if_fatal::exception:
                  std::cerr << " `if_fatal::exception` then throw fatal_exception now.\n";
                  throw fatal_exception ( to_string ( line ) );
              }
            }
          };
          _append = [ this, append_default ]
                    ( log_line_t&& line )
          {
            append_default
            ( { _start_time
                , level::info
                , ""
                , 0
                , ""
                , std::string ( "login time: " ) + detail::to_string_iso8601( _start_time, _time_format )
              }
            );
            append_default ( std::move ( line ) );
            this -> _append = append_default;
          };
        }
        
        log_t ( const log_t& ) = delete;
        log_t ( log_t&& ) = delete;
        
        auto operator= ( const log_t& ) -> void = delete;
        auto operator= ( log_t&& ) -> void = delete;
        
        auto rethrow() const
        -> void
        {
#ifndef EMSCRIPTEN
        
          if ( _exception_ptr != nullptr )
            std::rethrow_exception ( _exception_ptr );
            
#else
            
          if ( _pseudo_exception_ptr )
          {
            std::cerr
                << "[WARNING] Emscripten is not support std::exception_ptr yet,"
                " then pseudo rethrow as throw exception.\n"
                ;
            throw* _pseudo_exception_ptr;
          }
            
#endif
        }
        
      public:
      
        static inline
        auto instance( )
        -> log_t&
        {
          static auto instance = std::unique_ptr< log_t >( new log_t );
          instance->rethrow();
          return *instance;
        }
        
        ~log_t( )
        {
#ifndef EMSCRIPTEN
        
          if ( _exception_ptr != nullptr )
#else
          if ( _pseudo_exception_ptr )
#endif
          {
            std::cerr
                << "[WARNING] log_t has reserved exception, but here is destructor of log_t class."
                " then show the exception detail if exception based on std::exception,"
                " and std::quick_exit( EXIT_FAILURE ) now.\n  "
                ;
#ifndef EMSCRIPTEN
                
            try
            {
              std::rethrow_exception ( _exception_ptr );
            }
            catch ( const std::exception& e )
            {
              std::cerr << typeid ( e ).name() << " what: " << e.what() << "\n";
            }
            catch ( ... )
            {
              std::cerr << "unknown exception object.";
            }
            
#else
            std::cerr << "fatal_exception what: " << _pseudo_exception_ptr -> what() << "\n";
#endif
#if defined( EMSCRIPTEN ) || defined( __MINGW32__ )
            std::cerr
                << "[WARNING] Emscripten or MINGW is not support std::quick_exit yet,"
                " then call std::exit alternatively.\n"
                ;
            std::exit ( EXIT_FAILURE );
#else
            std::quick_exit ( EXIT_FAILURE );
#endif
          }
          
          ( *this ) ( level::info )
              << "logout time: "
              << detail::to_string_iso8601 ( clock_t::now(), _time_format )
              ;
          _at_destruct_hook( );
        }
        
        auto operator()
        ( std::string&& source_file     = ""
                                          , uint32_t      source_line     = 0
                                              , std::string && source_function = ""
        )
        -> log_stream_t
        { return ( *this ) ( _default_level, std::move ( source_file ), source_line, std::move ( source_function ) ); }
        
        auto operator()
        ( level level
          , std::string&& source_file     = ""
                                            , uint32_t      source_line     = 0
                                                , std::string && source_function = ""
        )
        -> log_stream_t
        {
          rethrow();
          return log_stream_t ( *this, level, std::move ( source_file ), source_line, std::move ( source_function ) );
        }
        
        auto default_level ( level level )
        -> void
        {
          rethrow();
          _default_level = level;
        }
        
        auto default_level() const
        -> level
        {
          rethrow();
          return _default_level;
        }
        
        auto keep_level ( level level )
        -> void
        {
          rethrow();
          _keep_level = level;
        }
        
        auto keep_level() const
        -> level
        {
          rethrow();
          return _keep_level;
        }
        
        auto hooks ( hooks_type&& hs )
        -> void
        {
          rethrow();
          _hooks = std::move ( hs );
        }
        
        auto hooks ( const hooks_type& hs )
        -> void
        {
          rethrow();
          _hooks = hs;
        }
        
        auto hooks( )
        -> hooks_type&
        {
          rethrow();
          return _hooks;
        }
        
        auto at_destruct ( destruct_hook_type&& h ) -> void
        {
          rethrow();
          _at_destruct_hook = std::move ( h );
        }
        
        auto if_fatal ( log::if_fatal f )
        -> void
        {
          rethrow();
          _if_fatal = f;
#ifdef EMSCRIPTEN
          
          if ( f == log::if_fatal::quick_exit )
            std::cerr
                << "[WARNING] Emscripten is not support std::quick_exit yet,"
            " then call std::exit if detect fatal with if_fatal flag equals quick_exit.\n"
            ;
            
#endif
        }
        
        auto if_fatal()
        -> log::if_fatal
        {
          rethrow();
          return _if_fatal;
        }
        
        auto start_time()
        -> decltype ( _start_time )
        {
          rethrow();
          return _start_time;
        }
        
        auto time_appearance ( log::time_appearance a )
        -> void
        {
          rethrow();
          _time_appearance = a;
        }

        auto time_format ( log::time_format a )
        -> void
        {
          rethrow();
          _time_format = a;
        }
        
        auto time_appearance()
        -> log::time_appearance
        { return _time_appearance; }

        auto time_format()
        -> log::time_format
        { return _time_format; }
      };
      
      template < class T = void >
      static auto make_string_output_hook ( std::ostream& s )
      -> log_t::hook_type
      {
        return [ &s ] ( log_line_t& log_line )
        {
          s << log_line;
        };
      }
      /*
        template < class T = void >
        static auto make_string_output_hook( std::inserter& i )
        -> log_t::hook_type
        {
        return [ &i ]( log_line_t& log_line ) { i << log_line.to_string(); };
        }*/
      
      template < class T = void >
      static auto make_string_output_hook ( T& pushable )
      -> log_t::hook_type
      {
        return [ &pushable ] ( log_line_t& log_line )
        {
          pushable.push ( to_string ( log_line ) );
        };
      }
      
      template < class T >
      auto to_string ( const log_line_t& log_line )
      -> std::string
      {
        std::ostringstream r;
        
        constexpr auto level_string_length = 11;
        using namespace std::chrono;
        
        r << std::fixed;
        
        auto& l = log_t::instance( );
        
        switch ( l.time_appearance( ) )
        {
          case log::time_appearance::f64_in_seconds_from_run:
            r << duration_cast< duration< double > > ( log_line.time - l.start_time( ) ).count();
            break;
            
          default:
            ;
            //r << time;
        }
        
        r << " [s]\t"
        << std::setw ( level_string_length )
        << std::left
        << log::to_string ( log_line.level )
        << "\t"
        << log_line.source_file
        << "\t"
        << log_line.source_line
        << "\t"
        << log_line.source_function
        << "\t"
        << log_line.message
        << "\n"
        ;
        
        return r.str();
      }
      
      template < class T >
      auto operator<< ( std::ostream& o, const log_line_t& log_line )
      -> std::ostream&
      { return o << to_string ( log_line ); }
      
      struct null_logger_t
      { };
      
      template< class T >
      auto operator<< ( const null_logger_t& _, const T& )
      -> const null_logger_t&
      {
        return _;
      }
      
      template < class >
      auto to_string ( level l )
      -> std::string
      {
        switch ( l )
        {
          case level::none :
            return "none";
            
          case level::debug:
            return "debug";
            
          case level::info :
            return "info";
            
          case level::warn :
            return "warn";
            
          case level::error:
            return "error";
            
          case level::fatal:
            return "fatal";
        }
        
        throw std::logic_error ( "unknown level value" );
      }
      
      template < class T >
      auto operator<< ( std::ostream& o, level l )
      -> std::ostream&
      { return o << to_string ( l ); }
      
      template < class T >
      auto to_string ( if_fatal f )
      -> std::string
      {
        switch ( f )
        {
          case if_fatal::none:
            return "none";
            
          case if_fatal::exit:
            return "exit";
            
          case if_fatal::quick_exit:
            return "quick_exit";
            
          case if_fatal::exception :
            return "exception";
        }
        
        throw std::logic_error ( "unknown if_fatal value" );
      }
      
      template < class T >
      auto operator<< ( std::ostream& o, if_fatal f )
      -> std::ostream&
      { return o << to_string ( f ); }
      
    }
  }
}
