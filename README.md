# Dithering 알고리즘에 관한 프로젝트 입니다.

# DBS(Direct Binary Search)
DBS(Direct Binary Search)에 관한 프로젝트 입니다.  
Clipping_Free_DBS 미완성..  

Given an error metric:  
d(I(x,y),b(x,y))  
example: d(I,b) = Σ((I(x,y)-b(x,y))2)  
Initialize binary image b(x,y) (example - choose random binary image).  
Randomly chose a pixel (x0,y0) in b(x,y)  
if d(I,b) < d(I,b) then assign b = b where b is b except for b(x0,y0) =1-b(x0,y0)  
Repeat last step until |d(I,b) - d(I,b)| is “small”.    

#  

# ERROR_DIFFUSION_DITHERING
에러 디퓨전 디더링에 대한 프로젝트 입니다.

# Floyd - Steinberg Dithering
플로이드-스타인버그 디더링은 1976년 로버트 플로이드와 루이스 스타인버그가 고안한 영상 디더링 알고리즘이다. 이는 영상 편집 소프트웨어에서 널리 쓰이는데, 예를 들어 최대 256색만을 지원하는 GIF 형식으로 영상을 저장하기 위해 변환할 때 쓰일 수 있다.

이 알고리즘은 픽셀의 양자화 오류를 주위의 픽셀로 분산시킴으로써 디더링을 수행하며, 그 분포는 다음과 같다.

   (matrix)

  0    0    0
 
  0    0    7   *  (1 / 16)
 
  3    5    1

알고리즘은 영상을 왼쪽에서 오른쪽, 위에서 아래로 따라가면서 각각의 픽셀을 양자화한다. 양자화 오류는 주위의 픽셀로 이전되지만, 이미 양자화된 픽셀은 영향을 받지 않는다. 또한, 픽셀의 값이 반올림 결과 내림이 된 픽셀이 많을수록 다음 픽셀은 올림이 될 가능성이 커지며, 이로써 평균적인 양자화 오류는 0에 가까워진다.

구현 중에는 홀수번째 줄과 짝수번째 줄의 디더링이 별도로 이루어지기도 하는데, 이를 가리켜 "serpentine scanning"이라 한다.

# Floyd - Steinberg Dithering 의사코드
for each y from top to bottom do

    for each x from left to right do

        oldpixel := pixel[x][y]

        newpixel := find_closest_palette_color(oldpixel) // 16비트 파일을 8비트 gray scale로 변경하는 함수.

        pixel[x][y] := newpixel

        quant_error := oldpixel - newpixel

        pixel[x + 1][y] := pixel[x + 1][y] + quant_error × 7 / 16

        pixel[x - 1][y + 1] := pixel[x - 1][y + 1] + quant_error × 3 / 16

        pixel[x][y + 1] := pixel[x][y + 1] + quant_error × 5 / 16

        pixel[x + 1][y + 1] := pixel[x + 1][y + 1] + quant_error × 1 / 16
