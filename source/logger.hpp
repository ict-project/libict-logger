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
//! Makro ustawiajace katalog bazowy do ścieżek plików.
#define LOGGER_BASEDIR ict::logger::setBaseDir(__FILE__)
//! Makro ustawiające strumień wyjściowy.
#define LOGGER_SET(stream,...) ict::logger::output::set(stream,##__VA_ARGS__)
//! Makro sprawdzające ustawienia strumienia wyjściowego.
#define LOGGER_TEST(stream) ict::logger::output::test(stream) 
//! Makro restartujące loggera (cały stos jest kasowany).
#define LOGGER_RESTART ict::logger::restart()
//! Makro ustawiające wartości domyślne dla warstw logowania.
#define LOGGER_DEFAULT(...) ict::logger::input::setDefault(__VA_ARGS__)
//! Makro - Informacja o pliku.
#define __LOGGER_FILE__ ict::logger::file(__FILE__)
//! Makro - Informacja o linii w pliku.
#define __LOGGER_LINE__ __LINE__
//! Makro - Informacja o funkcji.
#define __LOGGER_FUNCTION__ "("<<__PRETTY_FUNCTION__<<")"
//! Makro ładujące informacje o miejscu w kodzie.
#define __LOGGER__ __LOGGER_FILE__<<":"<<__LOGGER_LINE__<<" "<<__LOGGER_FUNCTION__<<" "
//============================================
namespace ict { namespace logger {
//===========================================
//! Typ zawierający wskazanie poziomów logowania loggera.
typedef uint8_t flags_t;

//! Stałe wskazujące poziomy logowania loggera (mogą być łączone sumą bitową).
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

//! Elementy pozwalające na podłączenie i manipulację wyjścia logowania.
namespace output {
  //!
  //! @brief Ustawia strumień wyjściowy dla logera.
  //!
  //! @param ostream Wskaźnik na strumień wyjściowy. 
  //! @param filter Filtr logów - zapisywane są tylko te, które mieszczą się w podanej masce. 
  //!  Jeśli podana zostanie wartość 0x0, to strumień zostanie usunięty.
  //!
  void set(std::ostream & ostream,flags_t filter=all);
  //!
  //! @brief Ustawia wyjście do syslog dla logera.
  //!
  //! @param ident String identyfikujący wpisy tej aplikacji w syslog.
  //! @param filter Filtr logów - zapisywane są tylko te, które mieszczą się w podanej masce. 
  //!  Jeśli podana zostanie wartość 0x0, to syslog zostanie usunięty.
  //!
  void set(const std::string & ident,flags_t filter=all);
  //!
  //! @brief Sprawdza, czy podany wskaźnik strumienia jest już ustawiony.
  //!
  //! @param ostream Wskaźnik na strumień wyjściowy. 
  //! @return Ustawienia filtra dla podanego strumienia. Jeśli 0x0, to strumień nie jest ustawiony.
  //!
  flags_t test(std::ostream * ostream);
  //!
  //! @brief Sprawdza, czy syslog jest już ustawiony.
  //!
  //! @return Ustawienia filtra dla syslog. Jeśli 0x0, to syslog nie jest ustawiony.
  //!
  flags_t test();
}
//! Elementy pozwalające na podłączenie się i manipulację do wejścia logowania.
namespace input {
  //!
  //! @brief Ustawia domyślne parametry (dla ict::logger::defaultValue).
  //! 
  //! @param [in] direct_in Poziomy logowania bez buforowania na danej warstwie.
  //! @param [in] buffered_in Poziomy logowania z buforowaniem na danej warstwie.
  //! @param [in] dump_in Poziomy logowania, które powodują opróżnienie bufora na danej warstwie.
  //!
  void setDefault(
    ict::logger::flags_t direct_in=ict::logger::notices,
    ict::logger::flags_t buffered_in=ict::logger::nonotices,
    ict::logger::flags_t dump_in=ict::logger::errors
  );
  //! Obiekt tworzący warstwę logowania. Musi być utworzony co najmniej jeden w danym wątku, by logowanie było możliwe.
  class Layer {
  public:
    //!
    //! @brief Konstruktor.
    //! 
    //! @param [in] direct_in Poziomy logowania bez buforowania na tej warstwie.
    //! @param [in] buffered_in Poziomy logowania z buforowaniem na tej warstwie.
    //! @param [in] dump_in Poziomy logowania, które powodują opróżnienie bufora na tej warstwie.
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
  //! @brief Podaje referencję do strumienia wyjścia (char) logowania dla zadanego poziomu logowania w najwyższej warstwie logowania w danym wątku.
  //!
  //! @param [in] severity Wskazanie poziomu logowania.
  //! @return Referencja do strumienia wyjścia (char) logowania.
  //!
  std::ostream & ostream(flags_t severity);
  //! Strumień na niby.
  class dummy_stream: public std::ostream {
  public:
    //! Nic nie robi.
    template <typename Any> dummy_stream & operator<<(Any a){
        return(*this);
    }
    //! Nic nie robi.
    dummy_stream & operator<<(std::ostream & (*f)(std::ostream&)){
      return(*this);
    }
    //! Nic nie robi.
    dummy_stream & operator<<(std::ios & (*f)(std::ios&)){
      return(*this);
    }
  };
  //! Strumień na niby.
  dummy_stream & dummy();
}
//!
//! @brief Restartuje loggera (cały stos jest kasowany).
//!
void restart();
//!
//! @brief Ustawia bazowy katalog do ścieżek plików w logerze.
//!
void setBaseDir(const std::string & file);
//! Wskaźnik do nazwy pliku.
struct file_struct{const char * path;};
inline file_struct file(const char * path){return {path};}
//! Ładuje nazwę pliku
std::ostream & operator<<(std::ostream & os,file_struct f);
//===========================================
} }
//===========================================
#endif