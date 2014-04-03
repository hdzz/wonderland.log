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
    
    namespace level
    {
      constexpr level_e none  = level_e::none;
      constexpr level_e debug = level_e::debug;
      constexpr level_e info  = level_e::info;
      constexpr level_e warn  = level_e::warn;
      constexpr level_e error = level_e::error;
      constexpr level_e fatal = level_e::fatal;
    }
    
    auto to_string(level_e a) -> std::string
    {
      switch(a)
      { case level_e::none : return "none";
        case level_e::debug: return "debug";
        case level_e::info : return "info";
        case level_e::warn : return "warn";
        case level_e::error: return "error";
        case level_e::fatal: return "fatal";
      }
    }
    
    class log_stream_t;
    
    struct log_line_t
    {
      std::chrono::time_point<std::chrono::high_resolution_clock> time;
      level_e     level;
      std::string message;
    };
    
    class log_t
    {
      friend class log_stream_t;
      
    public:
      using data_type = std::forward_list<log_line_t>;
      using hook_type = std::function<log_line_t&(log_line_t&)>;
      using destruct_hook_type = std::function<void(const data_type&)>;
      
    protected:
      
      data_type _log;
      
      level_e   _default_level;
      level_e   _keep_level;
      level_e   _hook_level;
      
      hook_type          _hook;
      destruct_hook_type _at_destruct_hook;
      
      auto _append(log_line_t&& a) -> void;
      
    public:
      log_t();
      ~log_t();
      auto operator()()              -> log_stream_t;
      auto operator()(level_e level) -> log_stream_t;
      auto clear()  -> void;
      auto begin()  -> decltype(data_type().begin());
      auto end()    -> decltype(data_type().end());
      auto cbegin() -> decltype(data_type().cbegin());
      auto cend()   -> decltype(data_type().cend());
      auto str()    -> std::string;
      auto default_level(level_e level) -> void;
      auto keep_level(level_e level) -> void;
      auto hook_level(level_e level) -> void;
      auto hook(hook_type&& h) -> void;
      auto at_destruct(destruct_hook_type&& h) -> void;
    };
    
    auto to_string(const log_line_t& line) -> std::string
    { 
      std::ostringstream r;
      r << std::chrono::duration_cast<std::chrono::nanoseconds>(line.time.time_since_epoch()).count()
        << "\t"
        << std::setw(11)
        << std::left
        << to_string(line.level)
        << "\t"
        << line.message
        << "\n"
        ;
      return r.str();
    }
    
    class log_stream_t
    {
      log_t&                                    _master;
      level_e                                   _level;
      const std::shared_ptr<std::ostringstream> _stream;
      
    public:
      template<class T>
      auto operator<<(const T& a) -> log_stream_t&
      { 
        (*_stream) << a;
        return *this;
      }
      
      explicit log_stream_t(log_t& a, level_e b)
        : _master(a)
        , _level(b)
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

    log_t::log_t()
      : _default_level(level_e::info)
      , _keep_level(level_e::debug)
      , _hook_level(level_e::none)
      , _hook( [](log_line_t& line) -> log_line_t& { return line; } )
      , _at_destruct_hook( [](const data_type&){} )
    { }
    
    log_t::~log_t() { _at_destruct_hook(_log); }
    
    auto log_t::_append(log_line_t&& line) -> void
    {
      if(std::uint8_t(line.level) >= std::uint8_t(_hook_level))
        _hook(line);
      
      if(std::uint8_t(line.level) >= std::uint8_t(_keep_level))
        _log.emplace_front( std::move(line) );
    }
    
    auto log_t::operator()()
      -> log_stream_t { return (*this)(_default_level); }
    auto log_t::operator()(level_e level)
      -> log_stream_t { return log_stream_t(*this, level); }
    
    auto log_t::clear()
      -> void { _log.clear(); }
      
    auto log_t::begin()
      -> decltype(data_type().begin())
    { return _log.begin(); }

    auto log_t::end()
      -> decltype(data_type().end())
    { return _log.end(); }
    
    auto log_t::cbegin()
      -> decltype(data_type().cbegin())
    { return _log.cbegin(); }
    
    auto log_t::cend()
      -> decltype(data_type().cend())
    { return _log.cend(); }
    
    auto log_t::str() -> std::string
    {
      std::ostringstream r;
      
      for(auto line: _log)
        r << to_string(line);
      
      return r.str();
    }
    
    auto log_t::default_level(level_e level) -> void { _default_level = level; }
    auto log_t::keep_level(level_e level) -> void { _keep_level = level; }
    auto log_t::hook_level(level_e level) -> void { _hook_level = level; }
    
    auto log_t::hook(hook_type&& h)
      -> void { _hook = std::move(h); }
    
    auto log_t::at_destruct(destruct_hook_type&& h)
      -> void { _at_destruct_hook = std::move(h); }

    auto hook_tie(std::ostream& s) -> log_t::hook_type
    {
      return [&s](log_line_t& log_line) -> log_line_t&
             {
               s << to_string(log_line);
               return log_line;
             };
    }
  }
}