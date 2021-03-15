#ifdef LOGGER_INFO
#undef LOGGER_INFO
#endif
//!Strumień wejściowy (char) dla poziomu INFO w najwyższej warstwie logowania dla danego wątku.
#define LOGGER_INFO ict::logger::input::dummy()