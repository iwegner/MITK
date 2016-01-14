MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC tinyxml PRIVATE Poco|Foundation
  WARNINGS_AS_ERRORS
)
add_subdirectory(test)

