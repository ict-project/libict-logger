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
#define LOGGER_LAYER 
//! Makro dodające kolejną warstwę na stosie logerów w danym wątku (z parametrami). Średnik konieczny na końcu.
#define LOGGER_L(...) 
//! Makro dodające kolejny wątek. Średnik konieczny na końcu.
#define LOGGER_THREAD