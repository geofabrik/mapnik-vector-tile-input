/*
 * mvt_messages.hpp
 *
 *  Created on:  2023-10-24
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef PLUGINS_INPUT_MBTILES_VECTOR_MVT_MESSAGE_HPP_
#define PLUGINS_INPUT_MBTILES_VECTOR_MVT_MESSAGE_HPP_

namespace mvt_message
{
    enum class tile : protozero::pbf_tag_type
    {
        layer = 3
    };
    enum class layer : protozero::pbf_tag_type
    {
        version = 15,
        name = 1,
        features = 2,
        keys = 3,
        values = 4,
        extent = 5
    };
    enum class value : protozero::pbf_tag_type
    {
        string_value = 1,
        float_value = 2,
        double_value = 3,
        int_value = 4,
        uint_value = 5,
        sint_value = 6,
        bool_value = 7
    };
    enum class feature : protozero::pbf_tag_type
    {
        id = 1,
        tags = 2,
        type = 3,
        geometry = 4
    };
    enum class geom_type : int32_t
    {
        unknown = 0,
        point = 1,
        linestring = 2,
        polygon = 3
    };
};



#endif /* PLUGINS_INPUT_MBTILES_VECTOR_MVT_MESSAGE_HPP_ */
