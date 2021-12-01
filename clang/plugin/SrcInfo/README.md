# Linux Build
  ## Prerequisit (Ubuntu 20.04 기준)
    * cmake, ninja-build, build-essential

  ## LLVM+CLANG Build
    1. git clone 및 12.x 브랜치 선택
      clone llvm-project into llvm-project
      git switch release/12.x

    2. 빌드용 디렉토리 생성
      cd llvm-project
      mkdir build & cd build

    3. llvm / plugin 빌드
      cmake -GNinja -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_EXPORT_SYMBOLS_FOR_PLUGINS=ON -DCLANG_BUILD_PLUGIN=ON -DCMAKE_BUILD_TYPE=MinSizeRel ../llvm
      ninja
    
    4. llvm 설치
      sudo ninja install

# Windows Build
  ## LLVM+CLANG Build
    1. VS command prompt 실행

    2. git clone 및 12.x 브랜치 선택
      clone llvm-project into llvm-project
      git switch release/12.x

    3. 빌드용 디렉토리 생성
      cd llvm-project
      mkdir build & cd build

    4. llvm / plugin 빌드
      cmake -GNinja -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_EXPORT_SYMBOLS_FOR_PLUGINS=ON -DCLANG_BUILD_PLUGIN=ON -DCMAKE_BUILD_TYPE=MinSizeRel ../llvm
      ninja

    5. llvm 설치(관리자 모드)
      ninja install