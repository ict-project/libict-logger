//! @file
//! @brief Logger module - Source file.
//! @author Mariusz Ornowski (mariusz.ornowski@ict-project.pl)
//! @version 1.0
//! @date 2012-2021
//! @copyright ICT-Project Mariusz Ornowski (ict-project.pl)
/* **************************************************************
Copyright (c) 2012-2021, ICT-Project Mariusz Ornowski (ict-project.pl)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the ICT-Project Mariusz Ornowski nor the names
of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************/
//============================================
#include "logger.hpp"
#include <ctime>
#include <sstream>
#include <string>
#include <streambuf>
#include <map>
#include <vector>
#include <set>
#include <stack>
#include <mutex>
#include <memory>
#include <iostream>
#include <filesystem>
#include "syslog.h"
//============================================
#define TRY_BEGIN try {
#define TRY_END } catch (...) { \
  std::cerr<<__LOGGER__<<"Logger exception!!!"<<std::endl; \
}
//============================================
namespace ict { namespace logger {
//===========================================
struct timestamp_t {
  timestamp_t():t(std::time(nullptr)){}
  void setTime(){t=std::time(nullptr);}
  std::time_t t; 
};
std::ostream & operator<<(std::ostream & os,const timestamp_t & t){
    //localtime_r zamiast std::localtime (wspólny statyczny bufor - wyścig danych między wątkami).
    std::tm tm{};
    ::localtime_r(&(t.t),&tm);
    char buf[64];
    const std::size_t size(std::strftime(buf,sizeof(buf),"%F %T(%z)",&tm));
    os.write(buf,size);
    return(os);
}
typedef std::map<const char *,std::string> path_map_t;
//! Mutex chroniący cache ścieżek plików i katalog bazowy.
static std::mutex & getPathMutex(){
  static std::mutex mutex;
  return(mutex);
}
static path_map_t & getPathMap(){
  static path_map_t map;
  return(map);
}
static std::string & getBaseDir(){
  static std::string base_dir;
  return(base_dir);
}
void setBaseDir(const std::string & file){
  std::lock_guard<std::mutex> lock(getPathMutex());
  getBaseDir()=std::filesystem::path(file).parent_path().native();
  getPathMap().clear();
}
std::ostream & operator<<(std::ostream & os,file_struct f){
    std::lock_guard<std::mutex> lock(getPathMutex());
    path_map_t::iterator it(getPathMap().find(f.path));
    if (it==getPathMap().end()) {
        std::string path(
          getBaseDir().size()?
            std::filesystem::relative(f.path,getBaseDir()).native():
            std::string(f.path)
        );
        it=getPathMap().emplace(f.path,std::move(path)).first;
    }
    os<<it->second;
    return(os);
}

//===========================================
  const flags_t critical(0x1<<0);
  const flags_t error(0x1<<1);
  const flags_t warning(0x1<<2);
  const flags_t notice(0x1<<3);
  const flags_t info(0x1<<4);
  const flags_t debug(0x1<<5);
  const flags_t errors    (critical|error);
  const flags_t warnings  (critical|error|warning);
  const flags_t notices   (critical|error|warning|notice);
  const flags_t infos     (critical|error|warning|notice|info);
  const flags_t all       (critical|error|warning|notice|info|debug);
  const flags_t nocritical         (error|warning|notice|info|debug);
  const flags_t noerrors                 (warning|notice|info|debug);
  const flags_t nowarnings                       (notice|info|debug);
  const flags_t nonotices                               (info|debug);
  const flags_t none                                           (0x0);
  const flags_t nodebug(infos);
  const flags_t defaultValue(0x1<<7);
  //==========================================================================
  const char * get_log_severity(flags_t severity){
    switch (severity){
      case critical: return("CRITICAL");
      case error: return("ERROR");
      case warning: return("WARNING");
      case notice: return("NOTICE");
      case info: return("INFO");
      case debug: return("DEBUG");
      default: return("");
    }
  }
  //==========================================================================
  class Syslog{
  private:
    //! Kopia identa - openlog() zapamiętuje wskaźnik, nie kopiuje łańcucha.
    std::string ident;
    flags_t filter;
  public:
    Syslog(const std::string & ident_in,flags_t filter_in):ident(ident_in),filter(filter_in){
      ::openlog(ident.c_str(),LOG_PID,LOG_USER);
    }
    ~Syslog(){
      ::closelog();
    }
    void log(flags_t severity,const std::string & str){
      int priority;
      switch (severity){
        case critical: priority=LOG_CRIT; break;//critical conditions
        case error: priority=LOG_ERR; break;//error conditions
        case warning: priority=LOG_WARNING; break;//warning conditions
        case notice: priority=LOG_NOTICE; break;//normal, but significant, condition
        case info: priority=LOG_INFO; break;// informational message
        case debug: priority=LOG_DEBUG; break;// debug-level message
        default: return;
      }
      if (filter&severity) ::syslog(priority,"%s",str.c_str());
    }
    flags_t getFilter(){return(filter);}
  };
  //==========================================================================
  template <typename charT>
  struct log_line_t {
    bool buffered=false;
    timestamp_t time;
    flags_t severity;
    std::basic_string<charT> line;
  };
  typedef log_line_t<char> log_string_t;
  typedef log_line_t<wchar_t> log_wstring_t;
  //==========================================================================
  //! Przekazuje linię loga (char) bez konwersji.
  inline const log_string_t & to_log_string(const log_string_t & in){
    return(in);
  }
  //! Konwertuje linię loga (wchar_t) do postaci char (znaki spoza ASCII zamieniane na '?').
  inline log_string_t to_log_string(const log_wstring_t & in){
    log_string_t out;
    out.buffered=in.buffered;
    out.time=in.time;
    out.severity=in.severity;
    out.line.reserve(in.line.size());
    for (const wchar_t c:in.line) out.line+=((0x20<=c)&&(c<0x7f))?static_cast<char>(c):'?';
    return(out);
  }
  //==========================================================================
  namespace output {
    struct StreamSink{
      std::ostream * stream;
      flags_t filter;
      std::mutex mutex;
      StreamSink(std::ostream * stream_in,flags_t filter_in):stream(stream_in),filter(filter_in){}
    };
    typedef std::map<std::ostream *,std::shared_ptr<StreamSink>> ostream_map_t;
    struct Data{
      //! Mutex chroniący konfigurację wyjść.
      std::mutex mutex;
      //! Zestaw strumieni wyjściowych ostream.
      ostream_map_t ostream_map;
      //! Wskaźnik na obieg obsługujący syslog.
      std::unique_ptr<Syslog> syslog;
    };
    static Data & data(){
      static Data data;
      return(data);
    }
    void set(std::ostream * ostream,flags_t filter){
      TRY_BEGIN
      std::lock_guard<std::mutex> lock(data().mutex);
      if (filter&&ostream){
        auto it=data().ostream_map.find(ostream);
        if (it==data().ostream_map.end()){
          data().ostream_map.emplace(ostream,std::make_shared<StreamSink>(ostream,filter));
        } else {
          it->second->filter=filter;
        }
      } else {
        data().ostream_map.erase(ostream);
      }
      TRY_END
    }
    void set(std::ostream & ostream,flags_t filter){
      set(&ostream,filter);
    }
    void set(const std::string & ident,flags_t filter){
      TRY_BEGIN
      std::lock_guard<std::mutex> lock(data().mutex);
      if (filter){
        //Najpierw closelog() starego obiektu, dopiero potem openlog() nowego.
        data().syslog.reset(nullptr);
        data().syslog.reset(new Syslog(ident,filter));
      } else if (data().syslog.get()) {
        data().syslog.reset(nullptr);
      }
      TRY_END
    }

    flags_t test(std::ostream * ostream){
      TRY_BEGIN
      std::lock_guard<std::mutex> lock(data().mutex);
      auto it=data().ostream_map.find(ostream);
      if (it!=data().ostream_map.end()) return(it->second->filter);
      TRY_END
      return(0x0);
    }
    flags_t test(){
      TRY_BEGIN
      std::lock_guard<std::mutex> lock(data().mutex);
      if (data().syslog.get()) {
        return(data().syslog->getFilter());
      }
      TRY_END
      return(0x0);
    }
    static void log_out(const log_line_t<char> & in){
      TRY_BEGIN
      std::vector<std::shared_ptr<StreamSink>> streams;
      bool syslog_enabled=false;
      {
        std::lock_guard<std::mutex> lock(data().mutex);
        for (const auto & entry:data().ostream_map){
          if (in.severity&entry.second->filter) streams.push_back(entry.second);
        }
        syslog_enabled=data().syslog&&((in.severity)&data().syslog->getFilter());
      }
      if (streams.empty()&&!syslog_enabled) return;

      std::ostringstream timestamp_out;
      timestamp_out<<in.time;
      const std::string timestamp(timestamp_out.str());
      const char * severity(get_log_severity(in.severity));

      if (!streams.empty()){
        std::ostringstream message_out;
        message_out<<timestamp<<' ';
        if (in.buffered) message_out<<"| ";
        message_out<<severity<<' '<<in.line<<'\n';
        const std::string message(message_out.str());
        for (const std::shared_ptr<StreamSink> & sink:streams){
          std::lock_guard<std::mutex> lock(sink->mutex);
          *sink->stream<<message;
          sink->stream->flush();
        }
      }

      if (syslog_enabled){
        std::string message;
        if (in.buffered){//Ten sam format, co dla strumieni: timestamp przed "| ".
          message=timestamp;
          message+=" | ";
        }
        message+=severity;
        message+=' ';
        message+=in.line;
        std::lock_guard<std::mutex> lock(data().mutex);
        if (data().syslog) data().syslog->log(in.severity,message);
      }
      TRY_END
    }
  }
  //==========================================================================
  //! Klasa obsługująca pusty bufor.
  template <
    typename charT=char,
    typename traits=std::char_traits<charT>
  > 
  class BlackHole:public std::basic_streambuf<charT,traits> {
  public:
    typedef typename traits::int_type int_type_t;
  public:
    //!
    //! @brief Konstruktor.
    //! 
    BlackHole(){}
    //!
    //! @brief Destruktor.
    //! 
    ~BlackHole(){}
    //!
    //! @brief Przetwarza jeden znak.
    //! 
    //! @param [in] c Przetwarzany znak.
    //! @return Przetworzony znak.
    //!
    int_type_t overflow (int_type_t c){
      //Zakończ.
      return(c);
    }
  };
  //==========================================================================
  //! Klasa obsługująca bufor logowania na wybranym poziomie.
  template <
    typename charT=char,
    typename traits=std::char_traits<charT>
  > 
  class Buffer:public std::basic_streambuf<charT,traits> {
  public:
    typedef std::basic_string<charT> basic_string_t;
    typedef std::basic_ostream<charT,traits> basic_ostream_t;
    typedef typename traits::int_type int_type_t;
    typedef std::vector<log_line_t<charT>> log_line_vector_t;
  private:
    //! Pojedyncza linia loga.
    log_line_t<charT> log_line;
    //! Bufor linii loga.
    log_line_vector_t * log_buffer;
    //! Informacja o tym, że ostatnio została złamana linia (rozpoczyna się nowy wpis loga).
    bool newline=true;
  public:
    //!
    //! @brief Konstruktor.
    //! 
    //! @param [in] severity_in Poziom logowania.
    //! @param [in] buffered_in Informacja, czy poziom jest buforowany.
    //! @param [in] buffered_in Informacja, czy poziom jest buforowany.
    //!
    Buffer(ict::logger::flags_t severity_in,bool buffered_in,log_line_vector_t * log_buffer_in):log_buffer(log_buffer_in){
      TRY_BEGIN
      log_line.buffered=buffered_in;
      log_line.severity=severity_in;
      TRY_END
    }
    //!
    //! @brief Destruktor.
    //! 
    ~Buffer(){}
    //!
    //! @brief Filtruje znaki.
    //! 
    //! @param [in] c Przetwarzany znak.
    //! @return Przetworzony znak (0 oznacza koniec linii).
    //!
    static charT charFilter(charT c){
      switch (c){
        case '\t'://Zamiana na spacje.
        case '\v':
        case '\0':
          return(' ');
        case '\n'://Zero oznacza koniec linii.
        case '\r':
          return(0);
        default:
          return(c);
      }
    }
  private:
    //!
    //! @brief Rozpoczyna nowy wpis loga (jeśli jeszcze nie został rozpoczęty).
    //! 
    void startLine(){
      if (newline) {
        //Jeśli rozpoczyna się nowy wpis, to pobierz aktualny czas.
        log_line.time.setTime();
        //Oznacz, że linia się rozpoczyna.
        newline=false;
        //Wyczyść linię.
        log_line.line.clear();
      }
    }
    //!
    //! @brief Kończy bieżący wpis loga (ignoruje powtarzające się znaki nowej linii).
    //! 
    void endLine(){
      if (newline) {
        //Ignoruj powtarzające się jeden po drugim.
        return;
      }
      //Oznacz nową linię.
      newline=true;
      if (log_line.buffered){//Jeśli zapis jest buforowany.
        const static std::size_t max(1000);//Maksymalny rozmiar bufora.
          if (log_buffer){
            if (log_buffer->size()<max){//Jeśli mniejszy, niż maksymalny rozmiar.
              log_buffer->push_back(log_line);//Dodaj do bufora.
            } else {
                if (log_buffer->size()==max) {
                  log_line_t<char> log_warn;
                  log_warn.buffered=true;
                  log_warn.line="logger.cpp";
                  log_warn.line+=" (";
                  log_warn.line+=__PRETTY_FUNCTION__;
                  log_warn.line+=") ";
                  log_warn.line+="Bufor loggera osiągnął maksymalny rozmiar!";
                  log_warn.severity=warning;
                  output::log_out(log_warn);//Zapisz do skonfigurowanych wyjść.
                  log_buffer->push_back(log_line);//Dodaj do bufora.
                }
            }
          }
      } else {//Jeśli zapis nie jest buforowany.
        output::log_out(to_log_string(log_line));//Zapisz do skonfigurowanych wyjść.
      }
    }
    //!
    //! @brief Przetwarza jeden znak (po przefiltrowaniu).
    //! 
    //! @param [in] c Przetwarzany znak.
    //!
    void putChar(charT c){
      //! Przetwarzany znak.
      charT znak(charFilter(c));
      if (znak){//Nie było znaku nowej linii
        startLine();
        //Wstaw przetwarzany znak.
        log_line.line+=znak;
      } else {//Pojawił się znak nowej linii
        endLine();
      }
    }
  protected:
    //!
    //! @brief Przetwarza jeden znak.
    //! 
    //! @param [in] c Przetwarzany znak.
    //! @return Przetworzony znak.
    //!
    int_type_t overflow (int_type_t c){
      TRY_BEGIN
      if (traits::eq_int_type(c,traits::eof())) return(traits::not_eof(c));
      putChar(traits::to_char_type(c));
      TRY_END
      //Zakończ.
      return(c);
    }
    //!
    //! @brief Przetwarza blok znaków (zamiast wirtualnego wywołania overflow dla każdego znaku).
    //! 
    //! @param [in] s Wskaźnik na blok znaków.
    //! @param [in] n Liczba znaków.
    //! @return Liczba przetworzonych znaków.
    //!
    std::streamsize xsputn(const charT * s,std::streamsize n){
      TRY_BEGIN
      std::streamsize i=0;
      while (i<n){
        std::streamsize start(i);
        while ((i<n)&&(charFilter(s[i])==s[i])) ++i;//Ciąg zwykłych znaków.
        if (i>start){
          startLine();
          log_line.line.append(s+start,i-start);//Dodaj cały ciąg na raz.
        }
        while ((i<n)&&(charFilter(s[i])!=s[i])){//Znaki wymagające filtrowania.
          putChar(s[i]);
          ++i;
        }
      }
      TRY_END
      return(n);
    }
  };
  //==========================================================================
  //! Klasa obsługująca logowanie na wszystkich poziomach (pojedynczy logger).
  template <
    typename charT=char,
    typename traits=std::char_traits<charT>
  > 
  class Single:
    public std::basic_ostringstream<charT,traits> 
  {
  public:
    typedef logger::Buffer<charT,traits> logger_buffer_t;
    typedef std::basic_ostream<charT,traits> basic_ostream_t;
    typedef std::vector<log_line_t<charT>> log_line_vector_t;
  private:
    //! Loger dla pojedynczego poziomu logowania.
    class StreamPack{
    public:
      logger_buffer_t buffer;
      basic_ostream_t stream;
      StreamPack(ict::logger::flags_t severity,bool buffered,log_line_vector_t * log_buffer):
        buffer(severity,buffered,log_buffer),stream(&buffer)
      {}
    };
    typedef std::map<ict::logger::flags_t,std::unique_ptr<StreamPack>> stream_map_t;
    //! Mapa logerów dla różnych poziomów.
    stream_map_t logger_map;
    //! Poziomy logowania bez buforowania na tej warstwie.
    ict::logger::flags_t direct;
    //! Poziomy logowania, które powodują opróżnienie bufora na tej warstwie.
    ict::logger::flags_t dump;
    //! Poziomy logowania, które są aktywne na tej warstwie.
    ict::logger::flags_t active;
    //! Poziomy logowania, które zostały wykonane na tej warstwie.
    ict::logger::flags_t done;
  public:
    //!
    //! @brief Konstruktor.
    //! 
    //! @param [in] direct_in Poziomy logowania bez buforowania na tej warstwie.
    //! @param [in] buffered_in Poziomy logowania z buforowaniem na tej warstwie.
    //! @param [in] dump_in Poziomy logowania, które powodują opróżnienie bufora na tej warstwie.
    //!
    Single(
      ict::logger::flags_t direct_in,
      ict::logger::flags_t buffered_in,
      ict::logger::flags_t dump_in
    ):direct(direct_in),dump(dump_in),active(direct_in|buffered_in),done(0){}
    ~Single(){
      TRY_BEGIN
      if (dump&done)//Jeśli pojawił się poziom, który wyzwala zrzut z buforów logujących
        doDump();//Zrób zrzut.
      TRY_END
    };
    //! Bufor linii loga.
    log_line_vector_t log_buffer;
    //!
    //! @brief Zrzuca cały bufor linii loga.
    //! 
    void doDump(){
      TRY_BEGIN
      for (log_line_t<charT> & l: log_buffer){//Cały bufor.
        output::log_out(to_log_string(l));//Zapisz do skonfigurowanych wyjść.
      }
      log_buffer.clear();//Wyczyść bufor.
      TRY_END
    }
    basic_ostream_t & getLogger(flags_t severity){
      //thread_local - wspólny statyczny strumień byłby współdzielony między wątkami bez synchronizacji.
      static thread_local BlackHole<charT> blackHoleBuff;
      static thread_local basic_ostream_t blackHole(&blackHoleBuff);
      TRY_BEGIN
      const static std::set<ict::logger::flags_t> severity_set({
        critical,error,warning,
        notice,info,debug
      });
      done|=severity;//Zaznacz, że był taki.
      if ((severity_set.count(severity))&&(active&severity)){//Jeśli poziom logowania jest prawidłowy i aktywny na tej warstwie.
        if (!logger_map.count(severity)){//Jeśli loger na takim poziomie nie istnieje
          logger_map[severity].reset(new StreamPack(severity,!(severity&direct),&log_buffer));//Stwórz logera.
        }
        return(logger_map[severity]->stream);//Zwróć go.
      }
      TRY_END
      return(blackHole);
    }
  };
  typedef Single<char> single_char_t;
  //==========================================================================
  //! Klasa obsługująca stos logerów (jedna instancja na wątek - nie wymaga synchronizacji).
  template <
    typename charT=char,
    typename traits=std::char_traits<charT>
  > 
  class Stack
  {
  public:
    typedef Single<charT,traits> single_t;
    typedef std::stack<std::unique_ptr<single_t>> stack_t;
  private:
    //! Stos logerów,
    stack_t stack;
  public:
    //!
    //! @brief Podaje referencję do najwyższego logera.
    //! 
    single_t & operator ()(){
      return(*(stack.top()));
    }
    //!
    //! @brief Dokłada nowego logger na stos.
    //! 
    //! @param [in] direct_in Poziomy logowania bez buforowania na tej warstwie.
    //! @param [in] buffered_in Poziomy logowania z buforowaniem na tej warstwie.
    //! @param [in] dump_in Poziomy logowania, które powodują opróżnienie bufora na tej warstwie.
    //! @return Liczba logerów na stosie.
    //!
    std::size_t push(
      ict::logger::flags_t direct_in,
      ict::logger::flags_t buffered_in,
      ict::logger::flags_t dump_in
    ){
      stack.emplace(new single_t(direct_in,buffered_in,dump_in));
      return(stack.size());
    }
    //!
    //! @brief Zdejmuje loggera ze stosu, jeśli nie jest to ostatni loger.
    //! 
    //! @return Liczba logerów na stosie.
    //!
    std::size_t pop(){
      if (stack.size()>0){
        stack.pop();
      }
      return(stack.size());
    }
    //!
    //! @brief Zwraca liczbę logerów na stosie.
    //! 
    //! @return Liczba logerów na stosie.
    //!
    std::size_t size(){
      return(stack.size());
    }
    //!
    //! @brief Kasuje cały stos.
    //! 
    void clear(){
      while (!stack.empty()) stack.pop();
    }
  };
  //==========================================================================
  //! Stos logerów dla bieżącego wątku (char).
  typedef Stack<char> stack_char_t;
  static stack_char_t & get_stack_char(){
    static thread_local stack_char_t stack;
    return(stack);
  }
  //==========================================================================
  namespace input {
    struct Data{
      //! Mutex dla strumieni wejściowych.
      std::mutex mutex;
      //! Wartość domyślna dla poziomów logowania bez buforowania na danej warstwie.
      ict::logger::flags_t directDefault=ict::logger::notices;
      //! Wartość domyślna dla poziomów logowania z buforowaniem na danej warstwie.
      ict::logger::flags_t bufferedDefault=ict::logger::nonotices;
      //! Wartość domyślna dla poziomów logowania, które powodują opróżnienie bufora na danej warstwie.
      ict::logger::flags_t dumpDefault=ict::logger::errors;
    };
    static Data & data(){
      static Data data;
      return(data);
    }
    void setDefault(
      ict::logger::flags_t direct_in,
      ict::logger::flags_t buffered_in,
      ict::logger::flags_t dump_in
    ){
      TRY_BEGIN
      std::lock_guard<std::mutex> lock(data().mutex);
      data().directDefault=direct_in;
      data().bufferedDefault=buffered_in;
      data().dumpDefault=dump_in;
      TRY_END
    }
    Layer::Layer(
      ict::logger::flags_t direct_in,
      ict::logger::flags_t buffered_in,
      ict::logger::flags_t dump_in
    ){
      TRY_BEGIN
      if (ict::logger::defaultValue&(direct_in|buffered_in|dump_in)){//Jeśli któryś parametr ma wartość domyślną.
        std::lock_guard<std::mutex> lock(data().mutex);
        if (ict::logger::defaultValue&direct_in) direct_in=data().directDefault;//Jeśli wartość domyślna.
        if (ict::logger::defaultValue&buffered_in) buffered_in=data().bufferedDefault;//Jeśli wartość domyślna.
        if (ict::logger::defaultValue&dump_in) dump_in=data().dumpDefault;//Jeśli wartość domyślna.
      }
      get_stack_char().push(direct_in,buffered_in,dump_in);//Dodaj logera char dla tej warstwy na stosie tego wątku.
      TRY_END
    }
    Layer::~Layer(){
      TRY_BEGIN
      //Zdejmij loggera ze stosu tego wątku.
      get_stack_char().pop();
      TRY_END
    }
    std::ostream & ostream(flags_t severity){
      //thread_local - wspólny statyczny strumień byłby współdzielony między wątkami bez synchronizacji.
      static thread_local BlackHole<char> blackHoleBuff;
      static thread_local std::basic_ostream<char> blackHole(&blackHoleBuff);
      TRY_BEGIN
      stack_char_t & stack(get_stack_char());//Stos logerów tego wątku.
      if (stack.size())//Jeśli są logery na stosie.
        return(stack().getLogger(severity));//Pobierz najwyższego loggera.
      TRY_END
      return(blackHole);
    }
    dummy_stream & dummy(){
      static dummy_stream d;
      return(d);
    }
  }
  void restart(){
    TRY_BEGIN
    //Kasuje stos logerów bieżącego wątku (stosy są per-wątek).
    get_stack_char().clear();
    TRY_END
  }
//===========================================
} }
//===========================================
#ifdef ENABLE_TESTING
#include "test.hpp"
#include <sstream>
#include <regex>
#include <thread>

REGISTER_TEST(logger,tc1){
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr);
  LOGGER_SET(std::cout);
  LOGGER_SET("test");
  #include "enable-all.hpp"
  LOGGER_CRIT<<__LOGGER__<<"Test string > string ... "<<0<<" "<<ict::test::test_string[0]<<std::endl;
  LOGGER_ERR<<__LOGGER__<<"Test string > string ... "<<1<<" "<<ict::test::test_string[1]<<std::endl;
  LOGGER_WARN<<__LOGGER__<<"Test string > string ... "<<2<<" "<<ict::test::test_string[2]<<std::endl;
  LOGGER_NOTICE<<__LOGGER__<<"Test string > string ... "<<3<<" "<<ict::test::test_string[3]<<std::endl;
  LOGGER_INFO<<__LOGGER__<<"Test string > string ... "<<4<<" "<<ict::test::test_string[4]<<std::endl;
  LOGGER_DEBUG<<__LOGGER__<<"Test string > string ... "<<5<<" "<<ict::test::test_string[5]<<std::endl;
  #include "disable-all.hpp"
  LOGGER_CRIT<<__LOGGER__<<"Test string > string ... "<<6<<" "<<ict::test::test_string[6]<<std::endl;
  LOGGER_ERR<<__LOGGER__<<"Test string > string ... "<<7<<" "<<ict::test::test_string[7]<<std::endl;
  LOGGER_WARN<<__LOGGER__<<"Test string > string ... "<<8<<" "<<ict::test::test_string[8]<<std::endl;
  LOGGER_NOTICE<<__LOGGER__<<"Test string > string ... "<<9<<" "<<ict::test::test_string[9]<<std::endl;
  LOGGER_INFO<<__LOGGER__<<"Test string > string ... "<<10<<" "<<ict::test::test_string[10]<<std::endl;
  LOGGER_DEBUG<<__LOGGER__<<"Test string > string ... "<<11<<" "<<ict::test::test_string[11]<<std::endl;
  #include "enable-all.hpp"
  {
    LOGGER_LAYER;
    LOGGER_CRIT<<__LOGGER__<<"Test string > string ... "<<12<<" "<<ict::test::test_string[12]<<std::endl;
    LOGGER_ERR<<__LOGGER__<<"Test string > string ... "<<13<<" "<<ict::test::test_string[13]<<std::endl;
    LOGGER_WARN<<__LOGGER__<<"Test string > string ... "<<14<<" "<<ict::test::test_string[14]<<std::endl;
    LOGGER_NOTICE<<__LOGGER__<<"Test string > string ... "<<15<<" "<<ict::test::test_string[15]<<std::endl;
    LOGGER_INFO<<__LOGGER__<<"Test string > string ... "<<16<<" "<<ict::test::test_string[16]<<std::endl;
    LOGGER_DEBUG<<__LOGGER__<<"Test string > string ... "<<17<<" "<<ict::test::test_string[17]<<std::endl;
  }
  return(0);
}

static std::regex getRegex(const std::string &sev,int no,bool b=false){
  std::string tmp(b?"\\| ":"");
  return std::regex(
    "\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\(\\+\\d{4}\\) "
    +tmp
    +sev
    +" logger\\.cpp:\\d+ "
    +".*"
    +"\\(\\)\\) Test "
    +std::to_string(no)
    +".*"
  );
}

REGISTER_TEST(logger,tc2){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  LOGGER_CRIT<<__LOGGER__<<"Test "<<1<<std::endl;
  LOGGER_ERR<<__LOGGER__<<"Test "<<2<<std::endl;
  LOGGER_WARN<<__LOGGER__<<"Test "<<3<<std::endl;
  LOGGER_NOTICE<<__LOGGER__<<"Test "<<4<<std::endl;
  LOGGER_INFO<<__LOGGER__<<"Test "<<5<<std::endl;
  LOGGER_DEBUG<<__LOGGER__<<"Test "<<6<<std::endl;
  #include "disable-all.hpp"
  LOGGER_CRIT<<__LOGGER__<<"Test "<<7<<std::endl;
  LOGGER_ERR<<__LOGGER__<<"Test "<<8<<std::endl;
  LOGGER_WARN<<__LOGGER__<<"Test "<<9<<std::endl;
  LOGGER_NOTICE<<__LOGGER__<<"Test "<<10<<std::endl;
  LOGGER_INFO<<__LOGGER__<<"Test "<<11<<std::endl;
  LOGGER_DEBUG<<__LOGGER__<<"Test "<<12<<std::endl;
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("CRITICAL",1))){
      std::cout<<"line="<<line<<std::endl;
      return(1); 
    }
  } else return(101);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("ERROR",2))){
      std::cout<<"line="<<line<<std::endl;
      return(2); 
    }
  } else return(102);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("WARNING",3))){
      std::cout<<"line="<<line<<std::endl;
      return(3); 
    }
  } else return(103);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("NOTICE",4))){
      std::cout<<"line="<<line<<std::endl;
      return(4); 
    }
  } else return(104);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("INFO",5))){
      std::cout<<"line="<<line<<std::endl;
      return(5); 
    }
  } else return(105);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("DEBUG",6))){
      std::cout<<"line="<<line<<std::endl;
      return(6); 
    }
  } else return(106);
  if (std::getline(stream,line)){
    return(200);
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc3){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  {
    LOGGER_LAYER;
    LOGGER_CRIT<<__LOGGER__<<"Test "<<1<<std::endl;
    LOGGER_ERR<<__LOGGER__<<"Test "<<2<<std::endl;
    LOGGER_WARN<<__LOGGER__<<"Test "<<3<<std::endl;
    LOGGER_NOTICE<<__LOGGER__<<"Test "<<4<<std::endl;
    LOGGER_INFO<<__LOGGER__<<"Test "<<5<<std::endl;
    LOGGER_DEBUG<<__LOGGER__<<"Test "<<6<<std::endl;
  }
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("CRITICAL",1))){
      std::cout<<"line="<<line<<std::endl;
      return(1); 
    }
  } else return(101);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("ERROR",2))){
      std::cout<<"line="<<line<<std::endl;
      return(2); 
    }
  } else return(102);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("WARNING",3))){
      std::cout<<"line="<<line<<std::endl;
      return(3); 
    }
  } else return(103);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("NOTICE",4))){
      std::cout<<"line="<<line<<std::endl;
      return(4); 
    }
  } else return(104);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("INFO",5,true))){
      std::cout<<"line="<<line<<std::endl;
      return(5); 
    }
  } else return(105);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("DEBUG",6,true))){
      std::cout<<"line="<<line<<std::endl;
      return(6); 
    }
  } else return(106);
  if (std::getline(stream,line)){
    return(200);
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc4){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  {
    LOGGER_LAYER;
    LOGGER_WARN<<__LOGGER__<<"Test "<<3<<std::endl;
    LOGGER_NOTICE<<__LOGGER__<<"Test "<<4<<std::endl;
    LOGGER_INFO<<__LOGGER__<<"Test "<<5<<std::endl;
    LOGGER_DEBUG<<__LOGGER__<<"Test "<<6<<std::endl;
  }
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("WARNING",3))){
      std::cout<<"line="<<line<<std::endl;
      return(3); 
    }
  } else return(103);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("NOTICE",4))){
      std::cout<<"line="<<line<<std::endl;
      return(4); 
    }
  } else return(104);
  if (std::getline(stream,line)){
    return(200);
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc5){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream,ict::logger::errors);//Tylko CRITICAL i ERROR.
  #include "enable-all.hpp"
  LOGGER_CRIT<<__LOGGER__<<"Test "<<1<<std::endl;
  LOGGER_ERR<<__LOGGER__<<"Test "<<2<<std::endl;
  LOGGER_WARN<<__LOGGER__<<"Test "<<3<<std::endl;
  LOGGER_NOTICE<<__LOGGER__<<"Test "<<4<<std::endl;
  LOGGER_INFO<<__LOGGER__<<"Test "<<5<<std::endl;
  LOGGER_DEBUG<<__LOGGER__<<"Test "<<6<<std::endl;
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("CRITICAL",1))){
      std::cout<<"line="<<line<<std::endl;
      return(1); 
    }
  } else return(101);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("ERROR",2))){
      std::cout<<"line="<<line<<std::endl;
      return(2); 
    }
  } else return(102);
  if (std::getline(stream,line)){
    return(200);//Nic więcej nie powinno przejść przez filtr.
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc6){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  #include "enable-all.hpp"
  if (LOGGER_TEST(&stream)!=ict::logger::none) return(1);//Jeszcze nie ustawiony.
  LOGGER_SET(stream,ict::logger::warnings);
  if (LOGGER_TEST(&stream)!=ict::logger::warnings) return(2);//Filtr zapamiętany.
  LOGGER_SET(stream,ict::logger::all);
  if (LOGGER_TEST(&stream)!=ict::logger::all) return(3);//Filtr zmieniony.
  LOGGER_SET(stream,ict::logger::none);
  if (LOGGER_TEST(&stream)!=ict::logger::none) return(4);//Strumień usunięty.
  LOGGER_ERR<<__LOGGER__<<"Test "<<1<<std::endl;
  if (std::getline(stream,line)){
    return(200);//Nic nie powinno trafić do usuniętego strumienia.
  }
  return(0);
}
REGISTER_TEST(logger,tc7){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  {
    LOGGER_L(ict::logger::none,ict::logger::all,ict::logger::warning);//Wszystko buforowane, zrzut po WARNING.
    LOGGER_INFO<<__LOGGER__<<"Test "<<1<<std::endl;
    LOGGER_DEBUG<<__LOGGER__<<"Test "<<2<<std::endl;
    LOGGER_WARN<<__LOGGER__<<"Test "<<3<<std::endl;
    if (std::getline(stream,line)){
      return(201);//Przed destrukcją warstwy bufor nie może być zrzucony.
    }
    stream.clear();
  }
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("INFO",1,true))){
      std::cout<<"line="<<line<<std::endl;
      return(1); 
    }
  } else return(101);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("DEBUG",2,true))){
      std::cout<<"line="<<line<<std::endl;
      return(2); 
    }
  } else return(102);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("WARNING",3,true))){
      std::cout<<"line="<<line<<std::endl;
      return(3); 
    }
  } else return(103);
  if (std::getline(stream,line)){
    return(200);
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc8){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  {
    LOGGER_LAYER;//Warstwa zewnętrzna.
    LOGGER_NOTICE<<__LOGGER__<<"Test "<<1<<std::endl;
    {
      LOGGER_LAYER;//Warstwa wewnętrzna - INFO/DEBUG buforowane.
      LOGGER_INFO<<__LOGGER__<<"Test "<<2<<std::endl;
      LOGGER_DEBUG<<__LOGGER__<<"Test "<<3<<std::endl;
    }//Brak błędu - bufor wewnętrznej warstwy porzucony.
    LOGGER_ERR<<__LOGGER__<<"Test "<<4<<std::endl;//Błąd już po zamknięciu wewnętrznej warstwy.
  }
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("NOTICE",1))){
      std::cout<<"line="<<line<<std::endl;
      return(1); 
    }
  } else return(101);
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("ERROR",4))){
      std::cout<<"line="<<line<<std::endl;
      return(4); 
    }
  } else return(104);
  if (std::getline(stream,line)){
    return(200);//Test 2 i 3 nie mogą się pojawić.
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc9){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  LOGGER_DEFAULT(ict::logger::all,ict::logger::none,ict::logger::none);//Wszystko bezpośrednio.
  {
    LOGGER_LAYER;//Korzysta z nowych wartości domyślnych.
    LOGGER_DEBUG<<__LOGGER__<<"Test "<<1<<std::endl;//Normalnie byłby buforowany i porzucony.
  }
  LOGGER_DEFAULT();//Przywrócenie wartości domyślnych.
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("DEBUG",1))){
      std::cout<<"line="<<line<<std::endl;
      return(1); 
    }
  } else return(101);
  if (std::getline(stream,line)){
    return(200);
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc10){
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  LOGGER_RESTART;//Kasuje stos warstw bieżącego wątku.
  LOGGER_ERR<<__LOGGER__<<"Test "<<1<<std::endl;//Brak warstwy - nic nie może się zapisać.
  if (std::getline(stream,line)){
    return(201);
  }
  stream.clear();
  {
    LOGGER_THREAD;//Nowa warstwa - logowanie znów działa.
    LOGGER_ERR<<__LOGGER__<<"Test "<<2<<std::endl;
  }
  if (std::getline(stream,line)){
    if (!std::regex_match(line,getRegex("ERROR",2))){
      std::cout<<"line="<<line<<std::endl;
      return(2); 
    }
  } else return(102);
  if (std::getline(stream,line)){
    return(200);
  }
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
REGISTER_TEST(logger,tc11){
  const int count=50;
  std::stringstream stream;
  std::string line;
  LOGGER_BASEDIR;
  LOGGER_THREAD;
  LOGGER_SET(std::cerr,ict::logger::none);
  LOGGER_SET(std::cout,ict::logger::none);
  LOGGER_SET("test",ict::logger::none);
  LOGGER_SET(stream);
  #include "enable-all.hpp"
  auto worker=[count](){
    LOGGER_THREAD;//Własna warstwa w każdym wątku.
    for (int k=0;k<count;k++) LOGGER_NOTICE<<__LOGGER__<<"Test "<<7<<std::endl;
  };
  std::thread t1(worker),t2(worker);
  t1.join();t2.join();
  int lines=0;
  //Osobny wzorzec - __PRETTY_FUNCTION__ w lambdzie ma inny format niż w zwykłej funkcji.
  const std::regex re(
    "\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\(\\+\\d{4}\\) "
    "NOTICE logger\\.cpp:\\d+ .* Test 7.*"
  );
  while (std::getline(stream,line)){
    if (!std::regex_match(line,re)){
      std::cout<<"line="<<line<<std::endl;
      return(1); 
    }
    lines++;
  }
  if (lines!=2*count) return(2);//Wszystkie linie z obu wątków, bez przeplotu.
  LOGGER_SET(stream,ict::logger::none);//Wyrejestrowanie lokalnego strumienia.
  return(0);
}
#endif
//===========================================
