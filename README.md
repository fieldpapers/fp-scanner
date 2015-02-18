# Field Papers Scanner

This repository contains (or will) Field Papers' snapshot processing pipeline.

This pipeline involves reading QR codes present in photos / scans (to determine
the atlas and page to which they correspond), identifying, extracting and
rectifying the source page, and geo-registering it.

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
