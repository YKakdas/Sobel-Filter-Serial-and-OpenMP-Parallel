# OpenMP Sobel Filter

Sobel Filter is an algorithm for edge detection For more information [click](https://en.wikipedia.org/wiki/Sobel_operator "Sobel Filter"). This c++ program takes an input image with .ppm extension, applies Sobel Filter for both serial and parallel(OpenMP) versions, and generates edge filtered images alongside the comparison of execution times.

# Requirements
- C++(gcc or g++) compilers are needed.
- OpenMP is needed.
## Compilation

- For IDE, you will need to make sure that OpenMP is enabled in the settings.

- For command-line compilation, please use the following structure,

```bash
g++ -o name_of_the_output -fopenmp SobelFilter.cpp
```

- Run after the compilation
```bash
./name_of_the_output.exe
```

# Samples

Before Sobel<br /><br />
![Input 2](/SampleInputImages/sample2.jpg)
<br /><br />
After Sobel<br /><br />
![Output 2](/SampleOutputs/sample2_parallel.jpg)

<br /><br />

Before Sobel<br /><br />
![Input 1](/SampleInputImages/sample.jpg)
<br /><br />
After Sobel<br /><br />
![Output 1](/SampleOutputs/sample_parallel.jpg)
