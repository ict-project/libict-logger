//! @file
//! @brief File rotation module - Source file.
//! @author Mariusz Ornowski (mariusz.ornowski@ict-project.pl)
//! @copyright ICT-Project Mariusz Ornowski (ict-project.pl)
/* **************************************************************
Copyright (c) 2012-2026, ICT-Project Mariusz Ornowski (ict-project.pl)
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
#include "rotate.hpp"
#include <cstdio>
//============================================
namespace ict { namespace logger {
//===========================================
rotate::rotate(const char* filename, std::size_t max_size, unsigned int max_copies):
    std::ostream(nullptr),buffer_(filename,max_size,max_copies) {
    rdbuf(&buffer_);
}

rotate::buffer_t::buffer_t(const char* filename, std::size_t max_size, unsigned int max_copies):
    filename_(filename),max_size_(max_size),max_copies_(max_copies) {
    file_.open(filename_, std::ios::out | std::ios::app);
}

rotate::buffer_t::int_type rotate::buffer_t::overflow(int_type c) {
    if (traits_type::eq_int_type(c, traits_type::eof())) {
        return (sync() == 0) ? traits_type::not_eof(c) : traits_type::eof();
    }
    if (!file_.is_open()) return traits_type::eof();
    const char ch = traits_type::to_char_type(c);
    file_.put(ch);
    if (!file_.good()) return traits_type::eof();
    if (ch == '\n') {
        file_.flush();
        if (file_.tellp() >= static_cast<std::streampos>(max_size_)) {
            rotateFiles();
        }
    }
    return c;
}

int rotate::buffer_t::sync() {
    if (!file_.is_open()) return -1;
    file_.flush();
    return file_.good() ? 0 : -1;
}

void rotate::buffer_t::rotateFiles() {
    file_.close();
    std::string oldest = filename_ + "." + std::to_string(max_copies_);
    std::remove(oldest.c_str());
    for (unsigned int i = max_copies_; i > 0; --i) {
        std::string old_name = filename_ + "." + std::to_string(i - 1);
        std::string new_name = filename_ + "." + std::to_string(i);
        if (i == 1) {
            old_name = filename_;
        }
        std::rename(old_name.c_str(), new_name.c_str());
    }
    file_.open(filename_, std::ios::out | std::ios::app);
}
//===========================================
} }
//===========================================
#ifdef ENABLE_TESTING
#include "test.hpp"
#include <cstdio>
#include <sstream>
#include <string>

//! Usuwa plik bazowy oraz wszystkie kopie rotacyjne.
static void rotate_test_clean(const std::string & filename,unsigned int max_copies){
  std::remove(filename.c_str());
  for (unsigned int i=1;i<=max_copies+1;++i){
    std::remove((filename+"."+std::to_string(i)).c_str());
  }
}

//! Zapisuje linię (zakończoną '\n') za pomocą interfejsu std::ostream.
static void rotate_test_put_line(ict::logger::rotate & r,const std::string & line){
  r<<line;
  r.put('\n');
}

//! Odczytuje całą zawartość pliku. Zwraca false, gdy plik nie istnieje.
static bool rotate_test_read(const std::string & filename,std::string & content){
  std::ifstream file(filename);
  if (!file.is_open()) return(false);
  std::stringstream ss;
  ss<<file.rdbuf();
  content=ss.str();
  return(true);
}

REGISTER_TEST(rotate,tc1){
  //Zapis bez rotacji (duży max_size).
  int out=0;
  const std::string filename("rotate-tc1.log");
  std::string content;
  rotate_test_clean(filename,3);
  {
    ict::logger::rotate r(filename.c_str(),1048576,3);
    rotate_test_put_line(r,"first line");
    rotate_test_put_line(r,"second line");
  }
  if (rotate_test_read(filename,content)){
    if (content!="first line\nsecond line\n"){
      std::cout<<"content="<<content<<std::endl;
      out=1;
    }
  } else out=101;
  if (!out) if (rotate_test_read(filename+".1",content)) out=201;//Kopia nie powinna powstać.
  rotate_test_clean(filename,3);
  return(out);
}

REGISTER_TEST(rotate,tc2){
  //Rotacja po przekroczeniu max_size - kolejność kopii.
  int out=0;
  const std::string filename("rotate-tc2.log");
  std::string content;
  rotate_test_clean(filename,3);
  {
    ict::logger::rotate r(filename.c_str(),5,3);
    rotate_test_put_line(r,"aaaa");//5 bajtów - rotacja po tej linii.
    rotate_test_put_line(r,"bbbb");
    rotate_test_put_line(r,"cccc");
  }
  if (rotate_test_read(filename,content)){
    if (!content.empty()){
      std::cout<<"content="<<content<<std::endl;
      out=1;
    }
  } else out=101;
  if (!out){
    if (rotate_test_read(filename+".1",content)){
      if (content!="cccc\n"){
        std::cout<<"content="<<content<<std::endl;
        out=2;
      }
    } else out=102;
  }
  if (!out){
    if (rotate_test_read(filename+".2",content)){
      if (content!="bbbb\n"){
        std::cout<<"content="<<content<<std::endl;
        out=3;
      }
    } else out=103;
  }
  if (!out){
    if (rotate_test_read(filename+".3",content)){
      if (content!="aaaa\n"){
        std::cout<<"content="<<content<<std::endl;
        out=4;
      }
    } else out=104;
  }
  rotate_test_clean(filename,3);
  return(out);
}

REGISTER_TEST(rotate,tc3){
  //Limit liczby kopii (max_copies) - najstarsze kopie są nadpisywane.
  int out=0;
  const std::string filename("rotate-tc3.log");
  std::string content;
  rotate_test_clean(filename,2);
  {
    ict::logger::rotate r(filename.c_str(),5,2);
    rotate_test_put_line(r,"aaaa");
    rotate_test_put_line(r,"bbbb");
    rotate_test_put_line(r,"cccc");
    rotate_test_put_line(r,"dddd");
  }
  if (rotate_test_read(filename+".1",content)){
    if (content!="dddd\n"){
      std::cout<<"content="<<content<<std::endl;
      out=1;
    }
  } else out=101;
  if (!out){
    if (rotate_test_read(filename+".2",content)){
      if (content!="cccc\n"){
        std::cout<<"content="<<content<<std::endl;
        out=2;
      }
    } else out=102;
  }
  if (!out) if (rotate_test_read(filename+".3",content)) out=201;//Kopia ponad limit nie powinna powstać.
  rotate_test_clean(filename,2);
  return(out);
}

REGISTER_TEST(rotate,tc4){
  //Dopisywanie do istniejącego pliku (tryb append).
  int out=0;
  const std::string filename("rotate-tc4.log");
  std::string content;
  rotate_test_clean(filename,3);
  {
    std::ofstream file(filename);
    file<<"existing\n";
  }
  {
    ict::logger::rotate r(filename.c_str(),1048576,3);
    rotate_test_put_line(r,"new line");
  }
  if (rotate_test_read(filename,content)){
    if (content!="existing\nnew line\n"){
      std::cout<<"content="<<content<<std::endl;
      out=1;
    }
  } else out=101;
  rotate_test_clean(filename,3);
  return(out);
}

#endif