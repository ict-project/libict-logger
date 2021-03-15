#ifdef LOGGER_DEBUG
#undef LOGGER_DEBUG
#endif
//!Strumień wejściowy (char) dla poziomu DEBUG w najwyższej warstwie logowania dla danego wątku.
#define LOGGER_DEBUG ict::logger::input::ostream(ict::logger::debug)