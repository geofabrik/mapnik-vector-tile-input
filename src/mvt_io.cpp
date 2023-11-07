/*
 * mvt_io.cpp
 *
 *  Created on:  2023-09-18
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "mvt_io.hpp"
#include "vector_tile_geometry_decoder.hpp"
#include <mapnik/feature_factory.hpp>
#include <mapnik/well_known_srs.hpp>
#include <stdexcept>


mvt_layer::mvt_layer(const uint32_t x, const uint32_t y, const uint32_t zoom)
    : keys_(),
    values_(),
    tr_("utf-8"),
    ctx_(std::make_shared<mapnik::context_type>())
{
    resolution_ = mapnik::EARTH_CIRCUMFERENCE / (1 << zoom);
    tile_x_ = -0.5 * mapnik::EARTH_CIRCUMFERENCE + x * resolution_;
    tile_y_ =  0.5 * mapnik::EARTH_CIRCUMFERENCE - y * resolution_;
}

void mvt_layer::add_feature(protozero::pbf_message<mvt_message::feature> feature)
{
    features_.push_back(feature);
}

bool mvt_layer::has_features() const
{
    return !features_.empty();
}

void mvt_layer::add_key(std::string&& key)
{
    keys_.push_back(key);
}

void mvt_layer::add_value(pbf_attr_value_type value)
{
    values_.push_back(value);
}

void mvt_layer::set_name(std::string&& name)
{
    name_ = name;
}

void mvt_layer::set_extent(uint32_t extent)
{
    extent_ = extent;
}

void mvt_layer::finish_reading()
{
    num_keys_ = keys_.size();
    num_values_ = values_.size();
    scale_ = static_cast<double>(extent_ / resolution_);
}

mapnik::feature_ptr mvt_layer::next_feature()
{
    while (feature_index_ < features_.size())
    {
        protozero::pbf_message<mvt_message::feature> f = features_.at(feature_index_);
        mapnik::feature_ptr feature = mapnik::feature_factory::create(ctx_, feature_index_);
        ++feature_index_;
        mvt_message::geom_type geometry_type = mvt_message::geom_type::unknown;
        bool has_geometry = false;
        bool has_geometry_type = false;
        mapnik::vector_tile_impl::GeometryPBF::pbf_itr geom_itr;
        while (f.next())
        {
            switch(f.tag())
            {
                case mvt_message::feature::id:
                    feature->set_id(f.get_uint64());
                    break;
                case mvt_message::feature::tags:
                    {
                        auto tag_iterator = f.get_packed_uint32();
                        for (auto _i = tag_iterator.begin(); _i != tag_iterator.end();)
                        {
                            std::size_t key_name = *(_i++);
                            if (_i == tag_iterator.end())
                            {
                                throw std::runtime_error("Vector Tile has a feature with an odd number of tags, therefore the tile is invalid.");
                            }
                            std::size_t key_value = *(_i++);
                            if (key_name < num_keys_ && key_value < num_values_)
                            {
                                std::string const& name = keys_.at(key_name);
                                if (feature->has_key(name))
                                {
                                    pbf_attr_value_type val = values_.at(key_value);
                                    value_visitor vv(tr_, feature, name);
                                    mapnik::util::apply_visitor(vv, val);
                                }
                            }
                        }
                    }
                    break;
                case mvt_message::feature::type:
                    has_geometry_type = true;
                    geometry_type = static_cast<mvt_message::geom_type>(f.get_enum());
                    switch (geometry_type)
                    {
                      case mvt_message::geom_type::point:
                      case mvt_message::geom_type::linestring:
                      case mvt_message::geom_type::polygon:
                        break;
                      default:
                        throw std::runtime_error("Vector tile has an unknown geometry type "
                                + std::to_string(static_cast<protozero::pbf_tag_type>(geometry_type)) + " in feature");
                    }
                    break;
                case mvt_message::feature::geometry:
                    if (has_geometry)
                    {
                        throw std::runtime_error("Vector Tile has a feature with multiple geometry fields, it must have only one of them");
                    }
                    has_geometry = true;
                    geom_itr = f.get_packed_uint32();
                    break;
                default:
                    throw std::runtime_error("Vector Tile contains unknown field type "
                            + std::to_string(static_cast<protozero::pbf_tag_type>(f.tag())) +" in feature");

            }
        }
        if (has_geometry)
        {
            if (!has_geometry_type)
            {
                throw std::runtime_error("Vector Tile has a feature that does not define the required geometry type.");
            }
            mapnik::vector_tile_impl::GeometryPBF geoms(geom_itr);
            mapnik::geometry::geometry<double> geom =
                    mapnik::vector_tile_impl::decode_geometry<double>(geoms, geometry_type,
                            2, tile_x_, tile_y_, scale_, -1.0 * scale_);
            if (geom.is<mapnik::geometry::geometry_empty>())
            {
                continue;
            }
//            #if defined(DEBUG)
//            mapnik::box2d<double> envelope = mapnik::geometry::envelope(geom);
//            if (!filter_.pass(envelope))
//            {
//                MAPNIK_LOG_ERROR(tile_featureset_pbf) << "tile_featureset_pbf: filter:pass should not get here";
//                continue;
//            }
//            #endif
            feature->set_geometry(std::move(geom));
            return feature;
        }
    }
    return mapnik::feature_ptr();
}

void mvt_io::read_layer(protozero::pbf_reader& pbf_layer)
//void mvt_io::read_layer(protozero::pbf_message<mvt_message::layer>& pbf_layer)
{
    layer_.reset(new mvt_layer(x_, y_, zoom_));
    std::cerr << "Message layer\n";
    bool ignore_layer = false;
    while (pbf_layer.next())
    {
        if (ignore_layer) {
            pbf_layer.skip();
            continue;
        }
        switch (pbf_layer.tag())
        {
        case static_cast<uint32_t>(mvt_message::layer::name):
        {
            std::string name = pbf_layer.get_string();
            std::cerr << "  name '" << name << "' expected '" << layer_name_ << "'\n";
            if (name != layer_name_)
            {
                std::cerr << "    skipping\n";
                ignore_layer = true;
            }
            layer_->set_name(std::move(name));
            break;
        }
        case static_cast<uint32_t>(mvt_message::layer::extent):
            std::cerr << "  extent\n";
            layer_->set_extent(pbf_layer.get_uint32());
            break;
        case static_cast<uint32_t>(mvt_message::layer::keys):
            std::cerr << "  keys\n";
            layer_->add_key(pbf_layer.get_string());
            break;
        case static_cast<uint32_t>(mvt_message::layer::values):
        {
            std::cerr << "  values\n";
            const auto data_view(pbf_layer.get_view());
            protozero::pbf_message<mvt_message::value> val_msg (data_view);
            while (val_msg.next())
            {
                switch(val_msg.tag()) {
                    case mvt_message::value::string_value:
                        layer_->add_value(val_msg.get_string());
                        break;
                    case mvt_message::value::float_value:
                        layer_->add_value(val_msg.get_float());
                        break;
                    case mvt_message::value::double_value:
                        layer_->add_value(val_msg.get_double());
                        break;
                    case mvt_message::value::int_value:
                        layer_->add_value(val_msg.get_int64());
                        break;
                    case mvt_message::value::uint_value:
                        layer_->add_value(val_msg.get_uint64());
                        break;
                    case mvt_message::value::sint_value:
                        layer_->add_value(val_msg.get_sint64());
                        break;
                    case mvt_message::value::bool_value:
                        layer_->add_value(val_msg.get_bool());
                        break;
                    default:
                        val_msg.skip();
                        throw std::runtime_error("unknown Value type " + std::to_string(static_cast<int>(val_msg.tag())) + " in layer->values");
                }
            }
            break;
        }
        case static_cast<uint32_t>(mvt_message::layer::features):
        {
            std::cerr << "  features\n";
            const auto data_view(pbf_layer.get_view());
            protozero::pbf_message<mvt_message::feature> f_msg (data_view);
            layer_->add_feature(f_msg);
            break;
        }
        case static_cast<uint32_t>(mvt_message::layer::version):
        {
            uint32_t version = pbf_layer.get_uint32();
            std::cerr << "  version " << version << '\n';
            if (version != 2) {
                throw std::runtime_error("Vector tile does not have major version 2.");
            }
            break;
        }
        default:
            std::cerr << "  other (tag " << static_cast<uint32_t>(pbf_layer.tag()) << " wire_type " << static_cast<uint32_t>(pbf_layer.wire_type()) << ")\n";
            pbf_layer.skip();
        }
    }
    layer_->finish_reading();
}

mapnik::feature_ptr mvt_io::next()
{
    if (!layer_ || !layer_->has_features())
    {
        return mapnik::feature_ptr();
    }
    return layer_->next_feature();
}

mvt_io::mvt_io(std::string&& data, const uint32_t x, const uint32_t y, const uint32_t zoom, std::string layer_name)
        : reader_(data),
        x_(x),
        y_(y),
        zoom_(zoom),
        layer_name_(layer_name)
{
    while (reader_.next())
    {
        switch (reader_.tag_and_type()) {
        case tag_and_type(static_cast<uint32_t>(mvt_message::tile::layer), protozero::pbf_wire_type::length_delimited): {
            std::cerr << "got layer\n";
            const auto data_view(reader_.get_view());
//            protozero::pbf_message<mvt_message::layer> msg_layer(reader_.get_message());
            protozero::pbf_reader msg_layer(data_view);
            read_layer(msg_layer);
//            read_layers();
            break;
        }
        default:
            std::cerr << "got tag " << reader_.tag() << " type " << static_cast<uint32_t>(reader_.wire_type()) << '\n';
            reader_.skip();
            break;
        }
    }
}

