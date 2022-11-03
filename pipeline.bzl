
def process_terrains(topos):
    for name in topos:
        topo = topos[name]
        _process_terrain(name, topo)

def _process_terrain(name, topo):
    # add the name to the dict for convenience with string formatting
    topo['name'] = name


    # merge DEMs if there are more than one
    dem_srcs = topo['dems']
    if len(dem_srcs) == 1:
        # if there is only one, skip the merge and use the original DEM
        merged_geotiff_name = dem_srcs[0] # "{name}_dems".format(name)
    else:
        # if there is more than one, merge them into one tile
        merged_geotiff_name = "{name}_merged_geotiff".format(**topo)
        native.genrule(
            name = merged_geotiff_name,
            srcs = dem_srcs,
            outs = ["{name}.tif".format(**topo)],
            cmd = """\
gdal_merge.py -o $@ $(SRCS)
du -hs $@
""",
        )


    # optionally extract a region of interest
    if "region_of_interest" not in topo:
        region_of_interest_name = merged_geotiff_name
    else:
        region_of_interest_name = "{name}_region_of_interest".format(**topo)

        ul = topo["region_of_interest"]["ul"]
        lr = topo["region_of_interest"]["lr"]

        ulx = ul[0]
        uly = ul[1]
        lrx = lr[0]
        lry = lr[1]

        native.genrule(
            name = region_of_interest_name,
            srcs = [merged_geotiff_name],
            outs = [region_of_interest_name + ".tif"],
            #cmd = "gdal_translate -srcwin {xoff} {yoff} {xsize} {ysize} -outsize {xsize} {ysize} $< $@".format(
            #    xoff=xoff,
            #    yoff=yoff,
            #    xsize=xsize,
            #    ysize=ysize,
            #),
            cmd = "gdal_translate -projwin {ulx} {uly} {lrx} {lry} $< $@".format(
                ulx=ulx,
                uly=uly,
                lrx=lrx,
                lry=lry,
            ),
        )


    # resize the geotiff
    if "resize_args" not in topo:
        resized_name = region_of_interest_name
    else:
        resized_name = "{name}_resized".format(**topo)
        native.genrule(
            name = resized_name,
            srcs = [region_of_interest_name],
            outs = ["{}.tif".format(resized_name)],
            cmd = "gdal_translate {resize_args} $< $@".format(**topo),
        )


    # run gdalinfo to get min/max height and extents for PNG scaling
    gdalinfo_name = "{name}_gdalinfo".format(**topo)
    native.genrule(
        name = gdalinfo_name,
        srcs = [resized_name],
        outs = [gdalinfo_name + ".json"],
        cmd = """\
# parse the info
gdalinfo -json -mm $< > $@

# assert there is only one band
# first print helpful messages because the failure message is not helpful
echo "checking that number of bands is 1..."
jq ".bands | length" $@
test `jq ".bands | length" $@` -eq 1
""",
    )


    # convert to PNG
    png_name = "{name}_png".format(**topo)
    native.genrule(
        name = png_name,
        srcs = [
            resized_name,
            gdalinfo_name,
        ],
        outs = ["{name}.png".format(**topo)],
        # use gdalinfo to scale to proper min/max
        cmd = """\
gdal_translate -of PNG -ot UInt16 -scale \
  `jq ".bands[0].computedMin" $(location {gdalinfo})` \
  `jq ".bands[0].computedMax" $(location {gdalinfo})` \
  0 65535 $(location {resized_name}) $@
du -hs $@
""".format(gdalinfo=gdalinfo_name, resized_name=resized_name),
        )


    # mesh it
    unscaled_stl_name = "{name}_unscaled_stl".format(**topo)
    native.genrule(
        name = unscaled_stl_name,
        srcs = [
            png_name,
            gdalinfo_name,
        ],
        outs = ["{name}_unscaled.stl".format(**topo)],
        cmd = """\
# triangulate
$(location @hmm//:hmm) $(location {png}) $@ --zscale `cat $(location {gdalinfo}) | $(location :zscale) 1` {hmm_args}

echo "--------------------------------------"

# print dimensions
$(location //src/meshtools:print_stl_dimensions) $@

echo "--------------------------------------"
# print file size
du -hs $@
""".format(png=png_name, gdalinfo=gdalinfo_name, **topo),
        tools = [
            "@hmm",
            ":zscale",
            "//src/meshtools:print_stl_dimensions",
        ]
    )


    # scale the mesh to true coordinates
    geoscaled_stl_name = "{name}_geoscaled_stl".format(**topo)
    native.genrule(
        name = geoscaled_stl_name,
        srcs = [
            unscaled_stl_name,
            gdalinfo_name,
        ],
        outs = ["{name}_geoscaled.stl".format(**topo)],
        cmd = """\
# scale mesh
cat $(location {gdalinfo}) | $(location :zscale) 2 | xargs $(location //src/meshtools:scale_stl) $(location {unscaled_stl}) $@

echo "--------------------------------------"

# print new dimensions
$(location //src/meshtools:print_stl_dimensions) $@

echo "--------------------------------------"

# print file size
du -hs $@
""".format(gdalinfo=gdalinfo_name, unscaled_stl=unscaled_stl_name, **topo),
        tools = [
            "//src/meshtools:scale_stl",
            ":zscale",
            "//src/meshtools:print_stl_dimensions",
        ]
    )


    # scale the mesh to user-defined coordinates
    if 'target_size' in topo:
        stl_name = "{name}_scaled_to_target_stl".format(**topo)
        native.genrule(
            name = stl_name,
            srcs = [
                unscaled_stl_name,
                gdalinfo_name,
            ],
            outs = ["{name}_scaled_to_target.stl".format(**topo)],
            cmd = """\
# scale mesh
$(location //src/meshtools:size_stl) $(location {unscaled_stl}) $@ {target_size}

# print new dimensions
$(location //src/meshtools:print_stl_dimensions) $@

# print file size
du -hs $@
""".format(gdalinfo=gdalinfo_name, unscaled_stl=unscaled_stl_name, **topo),
             tools = [
                 "//src/meshtools:size_stl",
                 "//src/meshtools:print_stl_dimensions",
             ]
         )


    # optionally make a contour
    if 'contour_level' in topo:
        # TODO(greg): translate this when the STL X-Y are rescaled
        native.genrule(
            name = "{name}_contour".format(**topo),
            srcs = [
                # region_of_interest_name, # use the full DEM
                resized_name, # use the resized DEM
            ],
            outs = [
                #'{name}_contour.shp'.format(name),
                #'{name}_contour.shx'.format(name),
                #'{name}_contour.dbf'.format(name),
                #'{name}_contour.prj'.format(name),
                '{name}_contour.dxf'.format(**topo),
            ],
            #cmd = "gdal_contour -fl 0 $< $(location :contour.shp)",
            cmd = "gdal_contour -fl {contour_level} $< $@".format(**topo),
        )


    native.sh_test(
        name = "{name}_stl_roundtrip".format(**topo),
        srcs = ["roundtrip_stl.sh"],
        data = [
            unscaled_stl_name,
            "//src/meshtools:roundtrip_stl",
        ],
        args = [
            "$(location //src/meshtools:roundtrip_stl)",
            "$(location {stl_name})".format(stl_name=unscaled_stl_name),
        ],
    )
