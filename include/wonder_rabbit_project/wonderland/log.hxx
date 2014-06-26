#pragma once

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

#ifndef WRP_WONDERLAND_LOG_NO_MACRO
  #ifndef WRP_WONDERLAND_LOG_DISABLE
  
    #ifndef WRP_WONDERLAND_LOG_NO_SOURCE_POSITION
      #define WRP_WONDERLAND_LOG_SOURCE_POSITION << __FILE__ << " " << __LINE__ << " "
    #else
      #define WRP_WONDERLAND_LOG_SOURCE_POSITION 
    #endif
  
    #define LOG  wonder_rabbit_project::wonderland::log::log_t::instance()() WRP_WONDERLAND_LOG_SOURCE_POSITION
    #define LOGD wonder_rabbit_project::wonderland::log::log_t::instance()( wonder_rabbit_project::wonderland::log::level::debug ) WRP_WONDERLAND_LOG_SOURCE_POSITION
    #define LOGI wonder_rabbit_project::wonderland::log::log_t::instance()( wonder_rabbit_project::wonderland::log::level::info  ) WRP_WONDERLAND_LOG_SOURCE_POSITION
    #define LOGW wonder_rabbit_project::wonderland::log::log_t::instance()( wonder_rabbit_project::wonderland::log::level::warn  ) WRP_WONDERLAND_LOG_SOURCE_POSITION
    #define LOGE wonder_rabbit_project::wonderland::log::log_t::instance()( wonder_rabbit_project::wonderland::log::level::error ) WRP_WONDERLAND_LOG_SOURCE_POSITION
    #define LOGF wonder_rabbit_project::wonderland::log::log_t::instance()( wonder_rabbit_project::wonderland::log::level::fatal ) WRP_WONDERLAND_LOG_SOURCE_POSITION
  #else
    #define LOG  0
    #define LOGD 0
    #define LOGI 0
    #define LOGW 0
    #define LOGE 0
    #define LOGF 0
  #endif
#endif

namespace wonder_rabbit_project
{
  namespace wonderland
  {
    namespace log
    {
      enum class level
        : std::uint8_t
      { none  = 0
      , debug = 1
      , info  = 2
      , warn  = 3
      , error = 4
      , fatal = 5
      };
      
      enum class time_appearance
      { f64_in_seconds_from_run
      , i64_in_seconds_from_epoch
      };
      
      struct fatal_exception
        : public std::runtime_error
      {
        fatal_exception( const std::string& message )
          : std::runtime_error( message )
        { }
      };
      
      static auto to_string( level a )
        -> std::string
      {
        switch( a )
        { case level::none : return "none";
          case level::debug: return "debug";
          case level::info : return "info";
          case level::warn : return "warn";
          case level::error: return "error";
          case level::fatal: return "fatal";
        }
        throw std::logic_error("unknown level value");
      }
      
      template < class T_clock = std::chrono::high_resolution_clock >
      auto to_string_iso8601( const typename T_clock::time_point& t )
        -> std::string
      {
        auto ct = T_clock::to_time_t( t );
        std::string r = "0000-00-00T00:00:00Z";
        std::strftime( const_cast< char* >( r.data() ), r.size(), "%FT%TZ", std::gmtime( &ct ) );
        return r;
      }
      
      class log_stream_t;
      
      struct log_line_t
      {
        std::chrono::time_point< std::chrono::high_resolution_clock > time;
        level       level;
        std::string message;
        
        auto to_string() const
          -> std::string
        ;
      };
      
      class log_t final
      {
        friend class  log_stream_t;
        friend struct log_line_t;
        
      public:
        using data_type = std::forward_list< log_line_t >;
        using hook_type = std::function< auto ( log_line_t& ) -> log_line_t& >;
        using destruct_hook_type = std::function< auto ( const data_type& ) -> void >;
        
        class log_stream_t
        {
          log_t& _master;
          level  _level;
          const std::shared_ptr<std::ostringstream> _stream;
          
        public:
          template<class T>
          auto operator<<( const T& value )
            -> log_stream_t&
          { 
            ( *_stream ) << value;
            return *this;
          }
          
          explicit log_stream_t( log_t& master, level level )
            : _master( master )
            , _level( level )
            , _stream( new std::ostringstream() )
          { }
          
          ~log_stream_t()
          {
            _master._append
            ( { std::chrono::high_resolution_clock::now()
              , _level
              , _stream -> str()
              }
            );
          }
        };
        
      private:
        
        data_type _log;
        
        level   _default_level;
        level   _keep_level;
        level   _hook_level;
        
        hook_type          _hook;
        destruct_hook_type _at_destruct_hook;
        
        const std::chrono::high_resolution_clock::time_point _start_time;
        bool _throw_if_fatal;
        log::time_appearance _time_appearance;
        
        auto _append( log_line_t&& line )
          -> void
        {
          if ( std::uint8_t( line.level ) >= std::uint8_t( _hook_level ) )
            _hook(line);
          
          if ( std::uint8_t( line.level ) >= std::uint8_t( _keep_level ) )
            _log.emplace_front( std::move(line) );
          
          if ( line.level == level::fatal && _throw_if_fatal )
            throw fatal_exception( line.to_string() );
        }
        
        log_t()
          : _default_level( level::info )
          , _keep_level( level::debug )
          , _hook_level( level::none )
          , _hook( []( log_line_t& line ) -> log_line_t& { return line; } )
          , _at_destruct_hook( []( const data_type& ){} )
          , _start_time( std::chrono::high_resolution_clock::now() )
          , _throw_if_fatal( true )
          , _time_appearance( log::time_appearance::f64_in_seconds_from_run )
        {
          ( *this )( level::info )
            << "login time: "
            << to_string_iso8601( _start_time )
            ;
        }
        
        log_t( const log_t& ) = delete;
        log_t( log_t&& ) = delete;
        
        auto operator=( const log_t& ) -> void = delete;
        auto operator=( log_t&& ) -> void = delete;
        
      public:
        
        static
        auto instance( )
          -> log_t&
        {
          static const auto i = std::unique_ptr< log_t >( new log_t() );
          return *i;
        }
        
        ~log_t()
        {
          ( *this )( level::info )
            << "logout time: "
            << to_string_iso8601( std::chrono::high_resolution_clock::now() )
            ;
          
          _at_destruct_hook( _log );
        }
        
        auto operator()()
          -> log_stream_t
        { return ( *this )( _default_level ); }

        auto operator()( level level )
          -> log_stream_t
        { return log_stream_t( *this, level ); }
        
        auto clear()
          -> void
        { _log.clear(); }
        
        auto begin()
          -> decltype( data_type().begin() )
        { return _log.begin(); }
        
        auto end()
          -> decltype( data_type().end() )
        { return _log.end(); }
        
        auto cbegin() const
          -> decltype( data_type().cbegin() )
        { return _log.cbegin(); }
        
        auto cend() const
          -> decltype( data_type().cend() )
        { return _log.cend(); }

        auto str() const
          -> std::string
        {
          std::ostringstream r;
          
          for(auto line: _log)
            r << line.to_string();
          
          return r.str();
        }
        
        auto default_level( level level )
          -> void
        { _default_level = level; }
        
        auto default_level() const
          -> level
        { return _default_level; }
        
        auto keep_level( level level )
          -> void
        { _keep_level = level; }
        
        auto keep_level() const
          -> level
        { return _keep_level; }
        
        auto hook_level( level level )
          -> void
        { _hook_level = level; }
        
        auto hook_level() const
          -> level
        { return _hook_level; }
        
        auto hook( hook_type&& h )
          -> void
        { _hook = std::move( h ); }
        
        auto at_destruct( destruct_hook_type&& h ) -> void
        { _at_destruct_hook = std::move( h ); }
        
        auto throw_if_fatal( bool enable )
          -> void
        { _throw_if_fatal = enable; }
        
        auto throw_if_fatal()
          -> bool
        { return _throw_if_fatal; }
        
        auto start_time()
          -> decltype( _start_time )
        { return _start_time; }
        
        auto time_appearance( log::time_appearance a )
          -> void
        { _time_appearance = a; }
        
        auto time_appearance()
          -> log::time_appearance
        { return _time_appearance; }
      };
      
      template < class T = void >
      static auto hook_tie( std::ostream& s )
        -> log_t::hook_type
      {
        return [ &s ]( log_line_t& log_line )
          -> log_line_t&
        {
          s << log_line.to_string();
          return log_line;
        };
      }
      
      std::string log_line_t::to_string() const
      {
        std::ostringstream r;
        
        constexpr auto level_string_length = 11;
        using namespace std::chrono;
        
        r << std::fixed;
        
        auto& l = log_t::instance();
        
        switch( l._time_appearance )
        { case log::time_appearance::f64_in_seconds_from_run:
            r << duration_cast< duration< double > >( time - l._start_time ).count();
            break;
          default:
            ;
            //r << time;
        }
        
        r << " [s]\t"
          << std::setw( level_string_length )
          << std::left
          << log::to_string( level )
          << "\t"
          << message
          << "\n"
          ;
        
        return r.str();
      }
      
    }
  }
}