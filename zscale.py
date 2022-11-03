#!/usr/bin/env bash

import numpy as np
import json
import sys

def main():
  gdalinfo = json.load(sys.stdin)
  if sys.argv[1] == "1":
    zscale_for_hmm(gdalinfo)
  elif sys.argv[1] == "2":
    scale_for_postprocess(gdalinfo)
  else:
    raise Exception("arg should be 1 or 2, but its " + sys.argv[1])


def scale_for_postprocess(gdalinfo):
  geotransform = gdalinfo['geoTransform']

  xpixel_size = geotransform[1]
  ypixel_size = geotransform[5]

  assert xpixel_size > 0, 'x pixel size expected to be positive'
  assert ypixel_size < 0, 'y pixel size expected to be negative'

  # test that relative error is small
  relerr = (xpixel_size - np.abs(ypixel_size)) / xpixel_size
  assert relerr < 0.01, 'pixels not scaled properly'

  scale_factor = xpixel_size
  print(scale_factor)


def zscale_for_hmm(gdalinfo):
  geotransform = gdalinfo['geoTransform']
  xpixel_size = geotransform[1]
  ypixel_size = geotransform[5]

  assert xpixel_size > 0, 'x pixel size expected to be positive'
  assert ypixel_size < 0, 'y pixel size expected to be negative'

  # test that relative error is small
  relerr = (xpixel_size - np.abs(ypixel_size)) / xpixel_size
  assert relerr < 0.01, 'pixels not scaled properly'

  # min/max
  computed_min = gdalinfo['bands'][0]['computedMin']
  computed_max = gdalinfo['bands'][0]['computedMax']

  scale_factor = (computed_max - computed_min) / xpixel_size

  print(scale_factor)


if __name__=='__main__':
  main()
