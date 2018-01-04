#define _GLIBCXX_USE_CXX11_ABI 0
#include "string.h"

#include <iostream>

//https://stackoverflow.com/a/29865
void hexdump(void *ptr, int buflen) {
  unsigned char *buf = (unsigned char*)ptr;
  int i, j;
  for (i=0; i<buflen; i+=16) {
    printf("%06x: ", i);
    for (j=0; j<16; j++)
      if (i+j < buflen)
        printf("%02x ", buf[i+j]);
      else
        printf("   ");
    printf(" ");
    for (j=0; j<16; j++)
      if (i+j < buflen)
        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
    printf("\n");
  }
}

mcpe::string* mcpe::string::empty;

mcpe::string::string() {
    ptr = empty->ptr;
}
mcpe::string::string(const char *str) {
    if (str[0] == '\0') {
        ptr = empty->ptr;
    } else {
        ptr = createRep(str, strlen(str));
        ptr->addRef();
    }
}
mcpe::string::string(const char *str, size_t len) {
    if (len == 0) {
        ptr = empty->ptr;
    } else {
        ptr = createRep(str, len);
        ptr->addRef();
    }
}
mcpe::string::string(const string &str) {
    if (str.ptr == empty->ptr) {
        ptr = empty->ptr;
    } else {
      str.ptr->addRef();
      ptr = str.ptr;
    }
}
mcpe::string::~string() {
    if (ptr == empty->ptr)
        return;
    // ((std::string*) this)->~basic_string<char>();
}

size_t mcpe::string::length() const {
    if (ptr == empty->ptr)
        return 0;

    return strlen((const char*) this->ptr);
    // return ((std::string*) this)->length();
}

mcpe::string mcpe::string::operator+(const string &str) {
  std::cout << " ADDED BOI \n";
    // return *((std::string*) this) + *((std::string*) &str);
}

mcpe::string& mcpe::string::operator=(const mcpe::string &str) {
  // std::cout << "THE OPERATOR IS CALLING ========\n";
  // std::cout << "ptr " << (unsigned) std::addressof(ptr) << "\n";
  // hexdump(ptr, 100);
// std::cout << "empty" << empty->ptr << "\n";
  // hexdump(str.ptr, 100);
  // std::cout << "empty " << (unsigned)std::addressof(empty->ptr) << "\n";
  // hexdump(empty->ptr, 100);

    if (ptr != empty->ptr) { // if it's not empty
        if (str.ptr == empty->ptr) {
            // ((std::string*) this)->~basic_string<char>();
            ptr->removeRef();
            ptr = str.ptr;
            // new (this)std::string(*((std::string*)&str));
            //ptr->addRef();
        } else {
          ptr->removeRef();
          ptr = str.ptr;
            // *((std::string*) this) = *((const std::string*) &str);
          ptr->addRef();
        }
    } else {
        ptr = str.ptr; //new (this)std::string(*((std::string*)&str));
        if (ptr != empty->ptr)
          ptr->addRef();
    }
}

const char *mcpe::string::c_str() const {
    if (ptr == empty->ptr)
        return "";

    // std::cout << "yo c_str\n";

    std::cout << "----------------c_str()\n";
    std::cout << "REAL LENGTH ?! " << strlen((const char*) this->ptr) << "\n";
    std::cout << this->ptr->length << "\n";
    std::cout << this->ptr->capacity << "\n";
    std::cout << this->ptr->refcount << "\n";
    std::cout << "----------------c_str()\n";

    hexdump(this->ptr, 100);
    printf("%s\n", (const char*) this->ptr);
    return ((const char*) this->ptr);

}

std::ostream& operator<< (std::ostream& os, const mcpe::string& obj) {
    os << obj.c_str();
    return os;
}

static mcpe::string::_Rep* createRep(const char* c, size_t l) {
  mcpe::string::_Rep* rp = (mcpe::string::_Rep*) malloc(sizeof(mcpe::string::_Rep) + l + 1);
  rp->length = rp->capacity = l + 1;
  rp->refcount = 0;

  memcpy((void*) (rp + 0xc), c, l);
  ((char*) (void*) (rp + 0xc))[l] = 0;
  std::cout << rp->length << " " << rp->capacity << " " << rp->refcount << "\n";
  return rp;
}
