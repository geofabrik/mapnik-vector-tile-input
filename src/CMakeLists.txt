add_library(input-mbtiles_vector MODULE)
set_target_properties(input-mbtiles_vector PROPERTIES
    OUTPUT_NAME mbtiles_vector
    POSITION_INDEPENDENT_CODE ON
    PREFIX ""
    SUFFIX ".input"
    #LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
)
#mapnik_install_plugin(input-mbtiles_vector)
target_sources(input-mbtiles_vector PRIVATE
    mbtiles_vector_datasource.cpp
    mbtiles_vector_featureset.cpp
    mvt_io.cpp
    vector_tile_compression.cpp
    vector_tile_geometry_decoder.cpp
)
target_include_directories(input-mbtiles_vector PRIVATE
    ${MAPNIK_INCLUDE_DIRS}
    ${MAPNIK_DEPS_INCLUDE_DIRS}
    ${PROTOZERO_INCLUDE_DIR}
)
target_link_libraries(input-mbtiles_vector PRIVATE
    ${MAPNIK_LIBRARIES}
    SQLite::SQLite3
    ZLIB::ZLIB
)
