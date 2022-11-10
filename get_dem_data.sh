#!/usr/bin/env bash

set -euo pipefail

#################################### zion #################################
source "zion_dem_names.sh"
zion_data_dir="data/zion"
mkdir -p "$zion_data_dir"

# transform names to URLs
zion_ql2_dem_urls=$zion_dem_names_ql2
for i in ${!zion_dem_names_ql2[@]}; do
    zion_ql2_dem_urls[i]="https://storage.googleapis.com/state-of-utah-sgid-downloads/lidar/zion-np-2016/QL2/DEMs/${zion_dem_names_ql2[i]}.zip"
done

zion_ql1_dem_urls=$zion_dem_names_ql1
for i in ${!zion_dem_names_ql1[@]}; do
    zion_ql1_dem_urls[i]="https://storage.googleapis.com/state-of-utah-sgid-downloads/lidar/zion-np-2016/QL1/DEMs/${zion_dem_names_ql1[i]}.zip"
done

# use xargs to download them all in parallel using wget
printf '%s\0' "${zion_ql2_dem_urls[@]}" "${zion_ql1_dem_urls[@]}" | xargs -P16 -n 1 -0 wget -P "$zion_data_dir"

# unzip them all
for zipfile in "$zion_data_dir"/*.zip; do
    unzip -d "$zion_data_dir" "$zipfile"
    rm "$zipfile"
done


####################### U.S. Coastal Relief Model - Southern California Version 2 ##################
socal_data_dir="data/socal"
mkdir -p "$socal_data_dir"
# 1 arc-second
#wget "https://www.ngdc.noaa.gov/thredds/fileServer/crm/crm_socal_1as_vers2.nc" -P ${socal_data_dir}
#gdal_translate "${socal_data_dir}/crm_socal_1as_vers2.nc" "${socal_data_dir}/crm_socal_1as_vers2.tif"
# 3 arc-second
wget "https://www.ngdc.noaa.gov/thredds/fileServer/crm/crm_socal_3as_vers2.nc" -P ${socal_data_dir}
gdal_translate "${socal_data_dir}/crm_socal_3as_vers2.nc" "${socal_data_dir}/crm_socal_3as_vers2.tif"


############################ etopo 2022 ##########################
# this is a global bathymetry dataset
etopo_data_dir="data/etopo"
mkdir -p ${etopo_data_dir}
wget -np -r -nH -L \
    --cut-dirs=7 \
    --no-check-certificate \
    "https://www.ngdc.noaa.gov/mgg/global/relief/ETOPO2022/data/15s/15s_bed_elev_gtif/" \
    -A tif \
    -P ${etopo_data_dir}

################################## CUDEM hawaii ##########################
# ninth arc-second (3m) digital elevation model of the Hawaiian Islands
# https://chs.coast.noaa.gov/htdata/raster2/elevation/NCEI_ninth_Topobathy_Hawaii_9428/
# this is mostly the islands over land

# third arc-second (10m) digital elevation model of the Hawaiian Islands
# https://chs.coast.noaa.gov/htdata/raster2/elevation/NCEI_third_Topobathy_Hawaii_9429/
# this is mostly the islands under land

# download both data sets
hawaii_data_dir="data/cudem_hawaii"
mkdir -p ${hawaii_data_dir}
for hawaii_dataset in "NCEI_ninth_Topobathy_Hawaii_9428" "NCEI_third_Topobathy_Hawaii_9429"
wget -np -r -nH -L --cut-dirs=5 --no-check-certificate \
    "https://chs.coast.noaa.gov/htdata/raster2/elevation/${hawaii_dataset}/tiles/" \
    -A tif \
    -P ${hawaii_data_dir}

##################################### copernicus #############################
# See https://portal.opentopography.org/datasetMetadata?otCollectionID=OT.032021.4326.1
# download dems
aws s3 cp s3://raster/COP90/COP90_hh data/copernicus --recursive --endpoint-url https://opentopography.s3.sdsc.edu --no-sign-request \
    --exclude "*" \
    --include "Copernicus_DSM_COG_30_N[234567]*_E[0123]*_DEM.tif" \
    --include "Copernicus_DSM_COG_30_N[234567]*_W[01][0123]*_DEM.tif"

# sync (download missing dems)
aws s3 sync s3://raster/COP90/COP90_hh data/copernicus --recursive --endpoint-url https://opentopography.s3.sdsc.edu --no-sign-request \
    --exclude "*" \
    --include "Copernicus_DSM_COG_30_N[234567]*_E[0123]*_DEM.tif" \
    --include "Copernicus_DSM_COG_30_N[234567]*_W[01][0123]*_DEM.tif"

####################################### gebco 2022 #################################
# download the global coverage grid from
# https://www.gebco.net/data_and_products/gridded_bathymetry_data/#global
# Get the one that says "sub-ice topo/bathy" or the one that says "ice surface elevation"
gebco_data_dir="data/gebco_2022_sub_ice"
gebco_zip_file="${gebco_data_dir}/gebco_2022_sub_ice_topo_geotiff.zip"
mkdir -p ${gebco_data_dir}
unzip ${gebco_zip_file} -d ${gebco_data_dir}
rm ${gebco_zip_file}

gebco_data_dir="data/gebco_2022"
gebco_zip_file="${gebco_data_dir}/gebco_2022_topo_geotiff.zip"
mkdir -p ${gebco_data_dir}
unzip ${gebco_zip_file} -d ${gebco_data_dir}
rm ${gebco_zip_file}

############################### SF bay topography/bathymetery #################################
# https://www.usgs.gov/core-science-systems/eros/coned/science/topobathymetric-elevation-model-san-francisco-bay-area?qt-science_center_objects=0#qt-science_center_objects
# https://topotools.cr.usgs.gov/topobathy_viewer/dwndata.htm
sfbay_data_dir="data/TOPOBATHY_SAN_FRANCISCO_ELEV_METERS"
sfbay_zip_file="${sfbay_data_dir}/TOPOBATHY_SAN_FRANCISCO_ELEV_METERS.zip"
mkdir -p ${sfbay_data_dir}
wget https://edcintl.cr.usgs.gov/downloads/sciweb1/shared/topo/downloads/Topobathy/TOPOBATHY_SAN_FRANCISCO_ELEV_METERS.zip -P ${sfbay_data_dir}
unzip ${sfbay_zip_file} -d ${sfbay_data_dir}
rm ${sfbay_zip_file}

################################# greenland (measures project) #################################
# The data that we want is here:
# https://nsidc.org/data/nsidc-0715/versions/2
#
# To download it, first follow instructions here to set up an account and put credentials in a .netrc file:
# https://nsidc.org/data/user-resources/help-center/programmatic-data-access-guide

# check that the urs.earthdata.nasa.gov is in the .netrc file
if grep -q urs.earthdata.nasa.gov ~/.netrc ; then
    echo "found urs.earthdata.nasa.gov in ~/.netrc"
else
    echo "urs.earthdata.nasa.gov not found in ~/.netrc"
    echo "please follow instructions here to set up an account and put credentials in a .netrc file:"
    echo "https://nsidc.org/data/user-resources/help-center/programmatic-data-access-guide"
    exit 1
fi

# download the data
measures_greenland_data_dir="data/measures_greenland"
mkdir -p ${measures_greenland_data_dir}
wget --load-cookies ~/.urs_cookies --save-cookies ~/.urs_cookies --keep-session-cookies --no-check-certificate --auth-no-challenge=on -r --reject "index.html*" --no-clobber -np -e robots=off https://n5eil01u.ecs.nsidc.org/MEASURES/NSIDC-0715.002/2008.05.15 -A tif -P ${measures_greenland_data_dir}
