{
  "targets": [
    {
      "target_name": "FaceVerification",
      "type": "shared_library",
      "sources": ["..\\..\\src\\FaceVerificationService.cpp", "..\\..\\src\\2DNormalization.cpp", "..\\..\\src\\FaceVerification.cpp", "..\\..\\src\\FeaturesExtractor.cpp", "..\\..\\src\\LandmarksDetection.cpp", "..\\..\\src\\Util.cpp"],
      "cflags": ["-Wall", "-std=c++11"], 
      "include_dirs" : ['..\\..\\src', 
                        'D:\Work\opencv\\build\install\include', 
                        'D:\\Work\\dlib-18.18_64\\dlib-18.18'],
      "link_settings" : {
        "libraries" : ['-ldlib.lib', 'opencv_core310d.lib', 'opencv_highgui310d.lib', 'opencv_imgproc310d.lib', 'opencv_objdetect310d.lib', 'opencv_imgcodecs310d.lib'],
        "library_dirs" : ['D:\\Work\\dlib-18.18_64\\dlib-18.18\\examples\\build\\dlib_build\\Debug', 
                          'D:\\Work\\opencv\\build\\install\\x64\\vc14\\lib']
      }
    }
  ]
}