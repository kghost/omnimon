pkg_check_modules(ICUUC REQUIRED icu-uc)
pkg_check_modules(ICUI18N REQUIRED icu-i18n)

add_subdirectory(utils)
add_subdirectory(backend)
add_subdirectory(frontend)
