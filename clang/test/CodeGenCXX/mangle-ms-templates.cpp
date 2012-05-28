// RUN: %clang_cc1 -fms-extensions -emit-llvm %s -o - -cxx-abi microsoft -triple=i386-pc-win32 | FileCheck %s

template<typename T>
class Class {
 public:
  void method() {}
};

class Typename { };

template<typename T>
class Nested { };

template<bool flag>
class BoolTemplate {
 public:
  BoolTemplate() {}
};

template<int param>
class IntTemplate {
 public:
  IntTemplate() {}
};

void template_mangling() {
  Class<Typename> c1;
  c1.method();
// CHECK: call {{.*}} @"\01?method@?$Class@VTypename@@@@QAEXXZ"

  Class<Nested<Typename> > c2;
  c2.method();
// CHECK: call {{.*}} @"\01?method@?$Class@V?$Nested@VTypename@@@@@@QAEXXZ"

  BoolTemplate<false> _false;
// CHECK: call {{.*}} @"\01??0?$BoolTemplate@$0A@@@QAE@XZ"

  BoolTemplate<true> _true;
// CHECK: call {{.*}} @"\01??0?$BoolTemplate@$00@@QAE@XZ"

  IntTemplate<5> five;
// CHECK: call {{.*}} @"\01??0?$IntTemplate@$04@@QAE@XZ"

  IntTemplate<11> eleven;
// CHECK: call {{.*}} @"\01??0?$IntTemplate@$0L@@@QAE@XZ"

  IntTemplate<65535> ffff;
// CHECK: call {{.*}} @"\01??0?$IntTemplate@$0PPPP@@@QAE@XZ"
}

namespace space {
  template<class T> const T& foo(const T& l) { return l; }
}
// CHECK: "\01??$foo@H@space@@YAABHABH@Z"

void use() {
  space::foo(42);
}
