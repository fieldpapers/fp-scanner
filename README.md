# Field Papers Scanner

This repository contains (or will) Field Papers' snapshot processing pipeline.

This pipeline involves reading QR codes present in photos / scans (to determine
the atlas and page to which they correspond), identifying, extracting and
rectifying the source page, and geo-reference it.

## How It Works

All atlases produced by Field Papers (and by Walking Papers before that)
include a QR code containing the URL of the atlas that they're from (including
the page number). Given this information, we know what bounding area they
cover, what map style was used, as well as the orientation, layout, and page
size. We also have the original PDF.

In computer vision (CV) parlance, this is the "object", and we know a number of
things about it. This allows us to identify instances of the object in
a user-provided "scene" (the uploaded snapshot).

Using the [OpenCV](http://opencv.org/) implementation of
a [SURF](http://en.wikipedia.org/wiki/SURF) feature detector, we can identify
the page _and_ calculate a perspective transformation matrix. This matrix is
typically used to introduce alternate objects into the scene at known locations
(augmented reality). However, its inverse can be used to extract the matched
area from the scene, producing an image with the same dimensions and content
(more or less--it will also include anything that's been written or drawn).

The more features that can be matched between the object and the scene, the
better the result is. In practice, corners are sufficient. (Successful matching
will occur with even fewer features, although the resulting image is usually
warped to the point of being unusable.)

This means that we can either use a rasterized version of the corresponding
page from the source PDF to identify the object in the scene _or_ just
a simplified template image containing common components present in all pages
with the same layout, orientation, and page size. Choice of which source to use
depends on the expected quality of images we need to process.

Given the extracted object with dimensions matching the source image
(effectively rectified), we can geo-register it using the bounding area
information from the atlas with `gdal_translate`.

## Usage

```bash
./extract letter-landscape.png snapshot.png extracted.png
```

## Building

```bash
cmake .
make
```

### OS X Dependencies

You'll need `opencv` and `cmake` (both in Homebrew):

```bash
brew install cmake opencv
```

## History

Walking Papers [originally used a SIFT-based blob
detector](http://mike.teczno.com/notes/walking-papers.html) to locate the
corners of individual prints (using images of gargoyles). While this worked
great for scans, the increasing prevalence of reasonably-qualified cameras on
cell phones made it more important to handle images in less predictable
orientations and lighting conditions.

Following this need and some inspiration from the [Astrometry
project](http://code.flickr.net/2009/02/18/found-in-space/), the gargoyles were
[replaced by blob detection of dots and the use of
trigonometry](http://mike.teczno.com/notes/walking-papers-cheaply.html). In
brief, the dots form triangles whose orientations and angles [can be used to
find corners and determine paper
sizes](https://www.flickr.com/photos/mmigurski/5548950825/).

Once the dots have been identified and their relationships used to information
about the print, their pixel locations can be combined with their expected
geolocation to produce ground control points for use by `gdal_translate` and
`gdalwarp`, ultimately producing a geo-referenced image that can be tiled and
browsed.
