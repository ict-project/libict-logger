//! @file
//! @brief Logger module - Header file.
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
#ifndef _ICT_LOGGER_HEADER
#define _ICT_LOGGER_HEADER
//============================================
#include <cstdint>
#include <ostream>
#include "enable-all.hpp"
#include "enable-layer.hpp"
//============================================
//! Makro ustawiajace katalog bazowy do ??cie??ek plik??w.
#define LOGGER_BASEDIR ict::logger::setBaseDir(__FILE__)
//! Makro ustawiaj??ce strumie?? wyj??ciowy.
#define LOGGER_SET(stream,...) ict::logger::output::set(stream,##__VA_ARGS__)
//! Makro sprawdzaj??ce ustawienia strumienia wyj??ciowego.
#define LOGGER_TEST(stream) ict::logger::output::test(stream) 
//! Makro restartuj??ce loggera (ca??y stos jest kasowany).
#define LOGGER_RESTART ict::logger::restart()
//! Makro ustawiaj??ce warto??ci domy??lne dla warstw logowania.
#define LOGGER_DEFAULT(...) ict::logger::input::setDefault(__VA_ARGS__)
//! Makro - Informacja o pliku.
#define __LOGGER_FILE__ ict::logger::file(__FILE__)
//! Makro - Informacja o linii w pliku.
#define __LOGGER_LINE__ __LINE__
//! Makro - Informacja o funkcji.
#define __LOGGER_FUNCTION__ "("<<__PRETTY_FUNCTION__<<")"
//! Makro ??aduj??ce informacje o miejscu w kodzie.
#define __LOGGER__ __LOGGER_FILE__<<":"<<__LOGGER_LINE__<<" "<<__LOGGER_FUNCTION__<<" "
//============================================
namespace ict { namespace logger {
//===========================================
//! Typ zawieraj??cy wskazanie poziom??w logowania loggera.
typedef uint8_t flags_t;

//! Sta??e wskazuj??ce poziomy logowania loggera (mog?? by?? ????czone sum?? bitow??).
extern const flags_t critical;
extern const flags_t error;
extern const flags_t warning;
extern const flags_t notice;
extern const flags_t info;
extern const flags_t debug;
extern const flags_t errors;
extern const flags_t warnings;
extern const flags_t notices;
extern const flags_t infos;
extern const flags_t all;
extern const flags_t nocritical;
extern const flags_t noerrors;
extern const flags_t nowarnings;
extern const flags_t nonotices;
extern const flags_t none;
extern const flags_t nodebug;
extern const flags_t defaultValue;

//! Elementy pozwalaj??ce na pod????czenie i manipulacj?? wyj??cia logowania.
namespace output {
  //!
  //! @brief Ustawia strumie?? wyj??ciowy dla logera.
  //!
  //! @param ostream Wska??nik na strumie?? wyj??ciowy. 
  //! @param filter Filtr log??w - zapisywane s?? tylko te, kt??re mieszcz?? si?? w podanej masce. 
  //!  Je??li podana zostanie warto???? 0x0, to strumie?? zostanie usuni??ty.
  //!
  void set(std::ostream & ostream,flags_t filter=all);
  //!
  //! @brief Ustawia wyj??cie do syslog dla logera.
  //!
  //! @param ident String identyfikuj??cy wpisy tej aplikacji w syslog.
  //! @param filter Filtr log??w - zapisywane s?? tylko te, kt??re mieszcz?? si?? w podanej masce. 
  //!  Je??li podana zostanie warto???? 0x0, to syslog zostanie usuni??ty.
  //!
  void set(const std::string & ident,flags_t filter=all);
  //!
  //! @brief Sprawdza, czy podany wska??nik strumienia jest ju?? ustawiony.
  //!
  //! @param ostream Wska??nik na strumie?? wyj??ciowy. 
  //! @return Ustawienia filtra dla podanego strumienia. Je??li 0x0, to strumie?? nie jest ustawiony.
  //!
  flags_t test(std::ostream * ostream);
  //!
  //! @brief Sprawdza, czy syslog jest ju?? ustawiony.
  //!
  //! @return Ustawienia filtra dla syslog. Je??li 0x0, to syslog nie jest ustawiony.
  //!
  flags_t test();
}
//! Elementy pozwalaj??ce na pod????czenie si?? i manipulacj?? do wej??cia logowania.
namespace input {
  //!
  //! @brief Ustawia domy??lne parametry (dla ict::logger::defaultValue).
  //! 
  //! @param [in] direct_in Poziomy logowania bez buforowania na danej warstwie.
  //! @param [in] buffered_in Poziomy logowania z buforowaniem na danej warstwie.
  //! @param [in] dump_in Poziomy logowania, kt??re powoduj?? opr????nienie bufora na danej warstwie.
  //!
  void setDefault(
    ict::logger::flags_t direct_in=ict::logger::notices,
    ict::logger::flags_t buffered_in=ict::logger::nonotices,
    ict::logger::flags_t dump_in=ict::logger::errors
  );
  //! Obiekt tworz??cy warstw?? logowania. Musi by?? utworzony co najmniej jeden w danym w??tku, by logowanie by??o mo??liwe.
  class Layer {
  public:
    //!
    //! @brief Konstruktor.
    //! 
    //! @param [in] direct_in Poziomy logowania bez buforowania na tej warstwie.
    //! @param [in] buffered_in Poziomy logowania z buforowaniem na tej warstwie.
    //! @param [in] dump_in Poziomy logowania, kt??re powoduj?? opr????nienie bufora na tej warstwie.
    //!
    Layer(
      ict::logger::flags_t direct_in=ict::logger::defaultValue,
      ict::logger::flags_t buffered_in=ict::logger::defaultValue,
      ict::logger::flags_t dump_in=ict::logger::defaultValue
    );
    //!
    //! @brief Destruktor.
    //! 
    ~Layer();
  };
  //!
  //! @brief Podaje referencj?? do strumienia wyj??cia (char) logowania dla zadanego poziomu logowania w najwy??szej warstwie logowania w danym w??tku.
  //!
  //! @param [in] severity Wskazanie poziomu logowania.
  //! @return Referencja do strumienia wyj??cia (char) logowania.
  //!
  std::ostream & ostream(flags_t severity);
  //! Strumie?? na niby.
  class dummy_stream  {//! Nic nie robi.
  public:
    template <typename Any> dummy_stream & operator<<(Any a){return(*this);}
    dummy_stream & operator<<(std::ostream & (*f)(std::ostream&)){return(*this);}
    dummy_stream & operator<<(std::ios & (*f)(std::ios&)){return(*this);}
    dummy_stream & put (char c){return(*this);}
    dummy_stream & write (const char* s, std::size_t n){return(*this);}
    std::size_t tellp(){return(0);}
    dummy_stream & seekp (std::size_t pos){return(*this);}
    dummy_stream & seekp (std::size_t off, std::ios_base::seekdir way){return(*this);}
    dummy_stream & flush(){return(*this);}
  };
  //! Strumie?? na niby.
  dummy_stream & dummy();
}
//!
//! @brief Restartuje loggera (ca??y stos jest kasowany).
//!
void restart();
//!
//! @brief Ustawia bazowy katalog do ??cie??ek plik??w w logerze.
//!
void setBaseDir(const std::string & file);
//! Wska??nik do nazwy pliku.
struct file_struct{const char * path;};
inline file_struct file(const char * path){return {path};}
//! ??aduje nazw?? pliku
std::ostream & operator<<(std::ostream & os,file_struct f);
//===========================================
} }
//===========================================
#endif