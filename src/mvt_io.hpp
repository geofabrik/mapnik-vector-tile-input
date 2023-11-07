/*
 * mvt_io.hpp
 *
 *  Created on:  2023-09-18
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef PLUGINS_INPUT_MBTILES_VECTOR_MVT_IO_HPP_
#define PLUGINS_INPUT_MBTILES_VECTOR_MVT_IO_HPP_

#include <memory>
#include <type_traits>
#include <protozero/pbf_message.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include "mvt_message.hpp"

//enum class mvt_value_type : char {
//    unsupported = 0,
//    value_integer = 1,
//    value_double = 2,
//    value_unicode = 3,
//    value_bool = 4
//};

using key_value_index = std::vector<uint32_t>;

//struct mvt_feature
//{
//    key_value_index key_indexes_;
//    key_value_index value_indexes_;
//    std::vector<uint32_t> geom_encoded_;
//    mvt_message::geom_type geom_type_ = mvt_message::geom_type::unknown;
//};

using pbf_attr_value_type = mapnik::util::variant<std::string, float, double, int64_t, uint64_t, bool>;

struct value_visitor
{
    mapnik::transcoder & tr_;
    mapnik::feature_ptr & feature_;
    std::string const& name_;

    value_visitor(mapnik::transcoder & tr,
                  mapnik::feature_ptr & feature,
                  std::string const& name)
        : tr_(tr),
          feature_(feature),
          name_(name) {}

    void operator() (std::string const& val)
    {
        feature_->put(name_, tr_.transcode(val.data(), val.length()));
    }

    void operator() (bool const& val)
    {
        feature_->put(name_, static_cast<mapnik::value_bool>(val));
    }

    void operator() (int64_t const& val)
    {
        feature_->put(name_, static_cast<mapnik::value_integer>(val));
    }

    void operator() (uint64_t const& val)
    {
        feature_->put(name_, static_cast<mapnik::value_integer>(val));
    }

    void operator() (double const& val)
    {
        feature_->put(name_, static_cast<mapnik::value_double>(val));
    }

    void operator() (float const& val)
    {
        feature_->put(name_, static_cast<mapnik::value_double>(val));
    }
};

class mvt_layer
{
    std::vector<protozero::pbf_message<mvt_message::feature>> features_;
    size_t feature_index_ = 0;
    std::vector<std::string> keys_;
    std::vector<pbf_attr_value_type> values_;
    std::size_t num_keys_ = 0;
    std::size_t num_values_ = 0;
    mapnik::transcoder tr_;
    mapnik::context_ptr ctx_;
    double tile_x_;
    double tile_y_;
    double resolution_;
    double scale_ = 0;
    std::string name_;
    uint32_t extent_ = 4096;
public:
    explicit mvt_layer(const uint32_t x, const uint32_t y, const uint32_t zoom);
    void add_feature(protozero::pbf_message<mvt_message::feature> feature);
    bool has_features() const;
    void add_key(std::string&& key);
    void add_value(pbf_attr_value_type value);

    void set_name(std::string&& name);
    void set_extent(uint32_t extent);
    mapnik::feature_ptr next_feature();
//    std::vector<std::string>& keys() const;
//    std::vector<std::string>& values() const;
    uint32_t extent() const;
    void finish_reading();
};

class mvt_io
{
    protozero::pbf_reader reader_;
    const uint32_t x_;
    const uint32_t y_;
    const uint32_t zoom_;
    std::string layer_name_;
    const int tile_extent_ = -1;
    std::unique_ptr<mvt_layer> layer_;

    void read_layer(protozero::pbf_reader& l);
//    void read_layer(protozero::pbf_message<mvt_message::layer>& l);
//    void read_layers();

//    /// Transform pixel coordinates to Mercator coordinates, requires coordinate of top left corner of the tile.
//    double pixel_x_to_mercator(const double x);
//    double pixel_y_to_mercator(const double y);

public:
    explicit mvt_io(std::string&& data, const uint32_t x, const uint32_t y, const uint32_t zoom, std::string layer_name);
    mapnik::feature_ptr next();
};



#endif /* PLUGINS_INPUT_MBTILES_VECTOR_MVT_IO_HPP_ */
