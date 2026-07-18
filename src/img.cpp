std::vector<float> gscalerr8bits(const std::vector<uint8_t> rgba, int w, int h, int chan = 4)
{
    const int width  = w;
    const int height = h;

    std::vector<float> gray(w * h, 255.0f);
    const uint8_t* src = rgba.data();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            int i = (y * width + x) * chan;

            uint8_t r = src[i + 0];
            uint8_t g = src[i + 1];
            uint8_t b = src[i + 2];
            uint8_t a = (chan == 4) ? src[i + 3] : 255;

            if (a < 128) {
                gray[y * width + x] = 255.0f; // transparent → white
            } else {
                gray[y * width + x] =
                    0.299f * r +
                    0.587f * g +
                    0.114f * b;
            }
        }
    }
    return gray;
}

