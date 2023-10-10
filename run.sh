./binary/hdf5-test_hdf5_no_staging
./binary/hdf5-test_hdf5_staging_disk_optimized
./binary/hdf5-test_hdf5_staging_memory_optimized

export ABT_DIR=/home/fabian/libraries/argobots
export HDF5_VOL_DIR=/home/fabian/libraries/vol
export LD_LIBRARY_PATH=/usr/local/HDF_Group/HDF5/1.15.0/lib:$HDF5_VOL_DIR/lib:$ABT_DIR/lib:$LD_LIBRARY_PATH
export HDF5_PLUGIN_PATH="$HDF5_VOL_DIR/lib"
export HDF5_VOL_CONNECTOR="cache_ext config=cache_1.cfg;under_vol=512;under_info={under_vol=0;under_info={}}"

export LD_PRELOAD=$ABT_DIR/lib/libabt.so

./binary/hdf5-test_hdf5_vol_prepared


echo "ABT_DIR $ABT_DIR"
echo "HDF5_VOL_DIR $HDF5_VOL_DIR"
echo "LD_LIBRARY_PATH $LD_LIBRARY_PATH"
echo "HDF5_PLUGIN_PATH $HDF5_PLUGIN_PATH"
echo "HDF5_VOL_CONNECTOR $HDF5_VOL_CONNECTOR"