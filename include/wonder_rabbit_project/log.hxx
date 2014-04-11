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

namespace wonder_rabbit_project
{
  namespace log
  {
    enum class level_e
      : std::uint8_t
    { none  = 0
    , debug = 1
    , info  = 2
    , warn  = 3
    , error = 4
    , fatal = 5
    };
    
    static auto to_string(level_e a) -> std::string
    {
      switch(a)
      { case level_e::none : return "none";
        case level_e::debug: return "debug";
        case level_e::info : return "info";
        case level_e::warn : return "warn";
        case level_e::error: return "error";
        case level_e::fatal: return "fatal";
      }
      throw std::logic_error("unknown level_e value");
    }
    
    class log_stream_t;
    
    struct log_line_t
    {
      std::chrono::time_point<std::chrono::high_resolution_clock> time;
      level_e     level;
      std::string message;
      
      auto to_string() const -> std::string
      {
        std::ostringstream r;
        r << std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count()
          << "\t"
          << std::setw(11)
          << std::left
          << wonder_rabbit_project::log::to_string(level)
          << "\t"
          << message
          << "\n"
          ;
        return r.str();
      }
    };
    
    class log_t
    {
      friend class log_stream_t;
      
    public:
      using data_type = std::forward_list<log_line_t>;
      using hook_type = std::function<log_line_t&(log_line_t&)>;
      using destruct_hook_type = std::function<void(const data_type&)>;
      
      class log_stream_t
      {
        log_t&                                    _master;
        level_e                                   _level;
        const std::shared_ptr<std::ostringstream> _stream;
        
      public:
        template<class T>
        auto operator<<(const T& value) -> log_stream_t&
        { 
          (*_stream) << value;
          return *this;
        }
        
        explicit log_stream_t(log_t& master, level_e level)
          : _master(master)
          , _level(level)
          , _stream(new std::ostringstream())
        { }
        
        ~log_stream_t()
        {
          _master._append
          ( { std::chrono::high_resolution_clock::now()
            , _level
            , _stream->str()
            }
          );
        }
      };
      
    protected:
      
      data_type _log;
      
      level_e   _default_level;
      level_e   _keep_level;
      level_e   _hook_level;
      
      hook_type          _hook;
      destruct_hook_type _at_destruct_hook;
      
      auto _append(log_line_t&& line) -> void
      {
      if(std::uint8_t(line.level) >= std::uint8_t(_hook_level))
        _hook(line);
      
      if(std::uint8_t(line.level) >= std::uint8_t(_keep_level))
        _log.emplace_front( std::move(line) );
      }
      
    public:
      log_t()
        : _default_level(level_e::info)
        , _keep_level(level_e::debug)
        , _hook_level(level_e::none)
        , _hook( [](log_line_t& line) -> log_line_t& { return line; } )
        , _at_destruct_hook( [](const data_type&){} )
      { }
      
      ~log_t()
      { _at_destruct_hook(_log); }
      
      auto operator()() -> log_stream_t
      { return (*this)(_default_level); }

      auto operator()(level_e level) -> log_stream_t
      { return log_stream_t(*this, level); }
      
      auto clear()  -> void
      { _log.clear(); }
      
      auto begin()  -> decltype(data_type().begin())
      { return _log.begin(); }
      
      auto end()    -> decltype(data_type().end())
      { return _log.end(); }
      
      auto cbegin() -> decltype(data_type().cbegin())
      { return _log.cbegin(); }
      
      auto cend()   -> decltype(data_type().cend())
      { return _log.cend(); }

      auto str()    -> std::string
      {
        std::ostringstream r;
        
        for(auto line: _log)
          r << line.to_string();
        
        return r.str();
      }
      
      auto default_level(level_e level) -> void
      { _default_level = level; }
      
      auto keep_level(level_e level) -> void
      { _keep_level = level; }
      
      auto hook_level(level_e level) -> void
      { _hook_level = level; }
      
      auto hook(hook_type&& h) -> void
      { _hook = std::move(h); }
      
      auto at_destruct(destruct_hook_type&& h) -> void
      { _at_destruct_hook = std::move(h); }
    };
    
    template<class T = void>
    static auto hook_tie(std::ostream& s) -> log_t::hook_type
    {
      return [&s](log_line_t& log_line) -> log_line_t&
             {
               s << log_line.to_string();
               return log_line;
             };
    }
  }
}