# Original version of this file created by Brad Howes, and available from http://wiki.codesynthesis.com/Using_XSD_with_CMake
# Modified by Claudio Pizzolato, Elena Ceseracciu

# Attempt to find the xsd application in various places. If found, the full
# path will be in XSD_EXECUTABLE. Look in the usual locations, as well as in
# the 'bin' directory in the path given in the XSD_ROOT environment variable.
find_program( XSD_EXECUTABLE
              NAMES
                xsd
                xsdcxx
              HINTS ${RWSL_DEPS}/xsd/bin $ENV{XSD_ROOT}/bin
              PATHS /usr/local/xsd-3.2.0-i686-macosx/bin
                    /usr/local/xsd-3.2.0-x86_64-linux-gnu/bin
                    /usr/local/bin
                    /opt/xsd-3.2.0-i686-macosx/bin
                    /opt/xsd-3.2.0-x86_64-linux-gnu/bin
                    /usr/bin
                    "C:/Program Files (x86)/CodeSynthesis XSD 3.3/bin"
                    "C:/Program Files (x86)/CodeSynthesis XSD 4.0/bin"
                    ENV PATH )

if( XSD_EXECUTABLE )

  # Obtain the include directory that one can use with INCLUDE_DIRECTORIES() to
  # access the xsd include files.
  get_filename_component( XSD_BIN_DIR "${XSD_EXECUTABLE}" PATH )
  get_filename_component( XSD_ROOT_DIR "${XSD_BIN_DIR}" PATH )
  set(XSD_INCLUDE_POSSIBLE_DIRS "${XSD_ROOT_DIR}/include")

endif()

find_path(XSD_INCLUDE_DIR xsd/cxx/config.hxx
                          HINTS ${XSD_INCLUDE_POSSIBLE_DIRS}
                          PATHS "C:/Program Files (x86)/CodeSynthesis XSD 3.3/include"
                                "C:/Program Files (x86)/CodeSynthesis XSD 4.0/include"
                                $ENV{XSD_ROOT}/libxsd)

# General CMake package configuration.

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( XSD DEFAULT_MSG XSD_EXECUTABLE XSD_INCLUDE_DIR )

if( XSD_FOUND )
  set( XSD_INCLUDE_DIRS ${XSD_INCLUDE_DIR} )
  if (NOT XSD_FIND_QUIETLY)
    message( STATUS "XSD Include Dir ${XSD_INCLUDE_DIR}")
  endif()
else()
  set( XSD_INCLUDE_DIRS )
endif()

mark_as_advanced( XSD_INCLUDE_DIR XSD_EXECUTABLE )

#
# Macro that attempts to generate C++ files from an XML schema. The NAME
# argument is the basename of the CMake variable to use to store paths to the
# derived C++ source file. An "_HEADERS" suffix is appended to the basename to define
# the variable that contains the list of generated headers, while "_SOURCES" is appended to define the variable
# that contains the list of generated source files, and "_INLINES" is appended to define the variable
# that contains the list of generated inline definition files. The FILE argument is the path of the schema file to
# process. Additional arguments should be XSD command-line options.
#
# Example:
#
# XSD_SCHEMA( FOO_SRCS Foo.xsd --root-element-first --generate-serialization )
#
# On return, FOO_SRCS will contain Foo.cxx.

macro( XSD_SCHEMA NAME FILE )

  # Make a full path from the source directory
  set( xs_SRC "${FILE}" )

  # XSD will generate two or three C++ files (*.cxx,*.hxx,*.ixx). Get the
  # destination file path sans any extension and then build paths to the
  # generated files.
  ## TODO: check --hxx-suffix / --hxx-regex options and so on, OR use --file-list option (but we lose info on headers/sources?)

  get_filename_component( xs_FILE "${FILE}" NAME_WE )
  set(generated_files)
  set( ${NAME}_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${xs_FILE}.cxx" )
  list(APPEND generated_files ${${NAME}_SOURCES})
  set( ${NAME}_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${xs_FILE}.hxx" )
  list(APPEND generated_files ${${NAME}_HEADERS})

  list(FIND ARGN "--generate-inline" generate_inline)
  if(generate-inline)
    set( ${NAME}_INLINES "${CMAKE_CURRENT_BINARY_DIR}/${xs_FILE}.ixx" )
    list(APPEND generated_files ${${NAME}_INLINES})
  endif()

  # Set up a generator for the output files from the given schema file using
  # the XSD cxx-tree command.

  add_custom_command( OUTPUT ${generated_files}
                      COMMAND ${XSD_EXECUTABLE}
                      ARGS "cxx-tree" ${ARGN} ${xs_SRC}
                      DEPENDS ${xs_SRC} )

  # Don't fail if a generated file does not exist.

  set_source_files_properties( ${generated_files}
                                PROPERTIES GENERATED TRUE )

endmacro( XSD_SCHEMA )
