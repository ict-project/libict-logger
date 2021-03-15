#ifdef LOGGER_ERR
#undef LOGGER_ERR
#endif
//!Strumień wejściowy (char) dla poziomu ERROR w najwyższej warstwie logowania dla danego wątku.
#define LOGGER_ERR ict::logger::input::ostream(ict::logger::error)