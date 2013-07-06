# Feature Detection

This project is about feature detection by using image/template cross-correlation and absolute differences.

## Implementation details

### Warning

Do **not** attempt to run this project in `DEBUG` (i.e. unoptimized) configuration unless you really need to do all the dishes in the neighbourhood or build a house.

### Image loading

Image loading is realized in and with the `FloatImage` class, `OpenCV` window management has moved to `OpenCvWindow` using a `std::unique_ptr` approach.

## Methods used

### Image and template cross-correlation

The cross-correlation implementation here realizes the normalized 2D cross-correlation in a very verbose manner.

### Image and template difference

Alternatively an *absolute difference* method is used to demonstrate the performance benefits over the cross-correlation approach. Know that this method easily leads to false positives when used in the wild.