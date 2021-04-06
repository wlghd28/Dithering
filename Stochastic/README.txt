* libtiff x64 빌드 방법
1. cmake_gui 소프트웨어 다운로드
(https://cmake.org/download/)

2. libtiff source code 다운로드
(http://download.osgeo.org/libtiff/)

3. libtiff 폴더생성 (원하는 위치)
4. libtiff 폴더 안에 build 폴더 생성
5. libtiff 폴더 안에 다운받은 libtiff source code를 압축 푼다
6. cmake_gui 실행
7. Where is the source code 칸에 폴더경로를 찾아 넣는다. (압축 푼 libtiff source code 폴더)
8. Where to build the binaries 칸에 폴더 경로 찾아 넣는다. (생성한 build 폴더)
9. Configure 클릭
10. 컴파일 버전을 Visual Studio 15 2017 (또는 현재 사용하는 버전)으로 등록
11. Generate 클릭
12. 11까지 하면 tiff.sln 파일이 생긴다. 프로젝트 실행
13. x64로 ALLBUILD를 빌드한다.