#ifdef LOGGER_LAYER
#undef LOGGER_LAYER
#endif
#ifdef LOGGER_L
#undef LOGGER_L
#endif
#ifdef LOGGER_THREAD
#undef LOGGER_THREAD
#endif
//! Makro dodające kolejną warstwę na stosie logerów w danym wątku. Średnik konieczny na końcu.
#define LOGGER_LAYER ict::logger::input::Layer _ict_logger_layer_
//! Makro dodające kolejną warstwę na stosie logerów w danym wątku (z parametrami). Średnik konieczny na końcu.
#define LOGGER_L(...) ict::logger::input::Layer _ict_logger_layer_(__VA_ARGS__)
//! Makro dodające kolejny wątek. Średnik konieczny na końcu.
#define LOGGER_THREAD ict::logger::input::Layer _ict_logger_layer_(ict::logger::all,ict::logger::none,ict::logger::none)