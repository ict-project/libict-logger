//! @file
//! @brief File rotation module - Header file.
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
#ifndef _ICT_LOGGER_ROTATE_HEADER
#define _ICT_LOGGER_ROTATE_HEADER
//============================================
#include <fstream>
#include <ostream>
#include <streambuf>
#include <string>
//============================================
namespace ict { namespace logger {
//===========================================
// Obiekt typu ostream (z pełnym interfejsem std::ostream), który zapisuje dane do pliku i obsługuje rotację plików (po każdej zapisanej linii, parametry: maksymalny rozmiar pliku, liczba kopii).
class rotate : public std::ostream {
public:
    //!
    //! @brief Construct a new rotate object that writes to the specified file and handles file rotation.
    //! 
    //! @param filename The name of the file to write to.
    //! @param max_size The maximum size of the file before rotation occurs.
    //! @param max_copies The maximum number of rotated copies to keep (if 0, keep all copies).
    //!
    rotate(const char* filename, std::size_t max_size=1048576, unsigned int max_copies=10);
private:
    //!
    //! @brief Stream buffer that writes to the file and performs rotation (checked after each written line).
    //!
    class buffer_t : public std::streambuf {
    public:
        buffer_t(const char* filename, std::size_t max_size, unsigned int max_copies);
    protected:
        int_type overflow(int_type c) override;
        int sync() override;
    private:
        //! Performs the file rotation (renames copies and reopens the base file).
        void rotateFiles();
        std::string filename_;
        std::size_t max_size_;
        unsigned int max_copies_;
        std::ofstream file_;
    };
    buffer_t buffer_;
};
//===========================================
} }
//===========================================
#endif