#ifdef LOGGER_NOTICE
#undef LOGGER_NOTICE
#endif
//!Strumień wejściowy (char) dla poziomu NOTICE_ w najwyższej warstwie logowania dla danego wątku.
#define LOGGER_NOTICE ict::logger::input::dummy()