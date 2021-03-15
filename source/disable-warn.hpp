#ifdef LOGGER_WARN
#undef LOGGER_WARN
#endif
//!Strumień wejściowy (char) dla poziomu WARNING_ w najwyższej warstwie logowania dla danego wątku.
#define LOGGER_WARN ict::logger::input::dummy()