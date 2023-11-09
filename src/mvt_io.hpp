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
    // We have to store the features as strings because they go out of scope otherwise (leading to PBF parsing errors).
    std::vector<std::string> features_;
    size_t feature_index_ = 0;
    std::vector<std::string> keys_;
    std::vector<pbf_attr_value_type> values_;
    mapnik::context_ptr const& context_;
    std::size_t num_keys_ = 0;
    std::size_t num_values_ = 0;
    mapnik::transcoder tr_;
    double tile_x_;
    double tile_y_;
    double resolution_;
    double scale_ = 0;
    std::string name_;
    uint32_t extent_ = 4096;
public:
    explicit mvt_layer(const uint32_t x, const uint32_t y, const uint32_t zoom, mapnik::context_ptr const& ctx);
    void add_feature(const protozero::data_view& feature);
    bool has_features() const;
    void add_key(std::string&& key);
    void add_value(pbf_attr_value_type value);

    void set_name(std::string&& name);
    void set_extent(uint32_t extent);
    mapnik::feature_ptr next_feature();
    uint32_t extent() const;
    void finish_reading();
};

class mvt_io
{
    protozero::pbf_reader reader_;
    mapnik::context_ptr context_;
    const uint32_t x_;
    const uint32_t y_;
    const uint32_t zoom_;
    std::string layer_name_;
    const int tile_extent_ = -1;
    std::unique_ptr<mvt_layer> layer_;

    /**
     * Read a layer from PBF. Returns true if requested layer was parsed.
     */
    bool read_layer(protozero::pbf_message<mvt_message::layer>& l);

public:
    explicit mvt_io(std::string&& data, mapnik::context_ptr const& ctx, const uint32_t x, const uint32_t y, const uint32_t zoom, std::string layer_name);
    mapnik::feature_ptr next();
};



#endif /* PLUGINS_INPUT_MBTILES_VECTOR_MVT_IO_HPP_ */
