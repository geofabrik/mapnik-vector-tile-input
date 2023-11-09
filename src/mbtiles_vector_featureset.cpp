/*
 * mbtiles_vector_featureset.cpp
 *
 *  Created on:  2023-09-15
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "mbtiles_vector_featureset.hpp"
#include "vector_tile_compression.hpp"
#include <boost/format.hpp>
#include <math.h>

mbtiles_vector_featureset::mbtiles_vector_featureset(std::shared_ptr<sqlite_connection> database,
        mapnik::context_ptr const& ctx, const int zoom,
        mapnik::box2d<double> const& extent, const std::string & layer) :
               database_(database),
               context_(ctx),
               zoom_(zoom),
               extent_(extent),
               layer_(layer),
               vector_tile_(nullptr)
{
    int tile_count = 1 << zoom;
    constexpr double width = 2.0 * 6378137 * M_PI;
    xmin_ = static_cast<int>((extent_.minx() + width / 2) * (tile_count / width));
    xmax_ = static_cast<int>((extent_.maxx() + width / 2) * (tile_count / width));
    ymin_ = static_cast<int>(((width / 2) - extent_.maxy()) * (tile_count / width));
    ymax_ = static_cast<int>(((width / 2) - extent_.miny()) * (tile_count / width));
    x_ = xmin_;
    y_ = ymin_;
    open_tile();
}

mbtiles_vector_featureset::~mbtiles_vector_featureset() { }

bool mbtiles_vector_featureset::valid() const
{
    return vector_tile_.get() != nullptr;
}

mapnik::feature_ptr mbtiles_vector_featureset::next()
{
    // If current tile is processed completely, go forward to the next tile.
    // else step forward to the next feature
    mapnik::feature_ptr f = mapnik::feature_ptr();
    if (!valid())
    {
        return f;
    }
    f = vector_tile_->next();
    if (f)
    {
        return f;
    }
    while (next_tile() && open_tile() && valid())
    {
        return vector_tile_->next();
    }
    return mapnik::feature_ptr();
}

int mbtiles_vector_featureset::convert_y(const int y) const
{
    return (1 << zoom_) - 1 - y;
}

bool mbtiles_vector_featureset::next_tile()
{
    ++x_;
    if (x_ <= xmax_)
    {
        return true;
    }
    x_ = xmin_;
    ++y_;
    return y_ <= ymax_;
}

bool mbtiles_vector_featureset::open_tile()
{
    std::string sql = (boost::format("SELECT tile_data FROM tiles WHERE zoom_level = %1% AND tile_column = %2% AND tile_row = %3%") % zoom_ % x_ % convert_y(y_)).str();
    std::shared_ptr<sqlite_resultset> result (database_->execute_query(sql));
    int size = 0;
    char const* blob = nullptr;
    if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_BLOB)
    {
        blob = result->column_blob(0, size);
    }
    else
    {
        return false;
    }
    if (mapnik::vector_tile_impl::is_gzip_compressed(blob, size) ||
        mapnik::vector_tile_impl::is_zlib_compressed(blob, size))
    {
        std::string decompressed;
        mapnik::vector_tile_impl::zlib_decompress(blob, size, decompressed);
        vector_tile_.reset(new mvt_io(std::move(decompressed), context_, x_, y_, zoom_, layer_));
    } else {
        vector_tile_.reset(new mvt_io(std::string(blob, size), context_, x_, y_, zoom_, layer_));
    }
    return true;
}
