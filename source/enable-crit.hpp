#ifdef LOGGER_CRIT
#undef LOGGER_CRIT
#endif
//!Strumień wejściowy (char) dla poziomu CRITICAL w najwyższej warstwie logowania dla danego wątku.
#define LOGGER_CRIT ict::logger::input::ostream(ict::logger::critical)