//#define DEBUG_JSON
#include <json/jsonpp.hpp>

#include <iostream>
#include <fstream>
#include <locale>
#include <iterator>

template <typename Char, std::size_t BUFLEN=256>
class transcode_iterator {
  static const std::size_t BACKLEN = sizeof(char)*BUFLEN;
  static const std::size_t TRANLEN = sizeof(Char)*BUFLEN;
  typedef std::basic_istream<Char> stream_type;
public:
  transcode_iterator ()
    : stream_(0), cd_((iconv_t)-1)
    , tran_off_(0), back_off_(0) {
    this->zero();
  }
  transcode_iterator (stream_type& str)
    : stream_(&str), cd_((iconv_t)-1)
    , tran_off_(0), back_off_(0){
    this->zero();
  }
  transcode_iterator (transcode_iterator const& ti)
    : stream_(ti.stream_), cd_(ti.cd_) {
    this->copy(ti);
  }

protected:
  void zero () {
    std::memset(this->back_buffer_, 0, BACKLEN);
    std::memset(this->tran_buffer_, 0, TRANLEN);
  }
  void copy (transcode_iterator const& ti) {
    this->tran_off_ = ti.tran_off_;
    this->back_off_ = ti.back_off_;
    std::memcpy(this->back_buffer_, ti.back_buffer_, BACKLEN);
    std::memcpy(this->tran_buffer_, ti.tran_buffer_, TRANLEN);
  }
private:
  stream_type *stream_;
  char back_buffer_[BUFLEN];
  Char tran_buffer_[BUFLEN];
  std::size_t tran_off_, back_off_;
  iconv_t cd_;
};

int main (int argc, char *argv[]) {

  if (argc < 1)
    return 1;

  for (++argv; argc > 0; --argc, ++argv) {
    std::cout << *argv << std::endl;
    try {
      std::wifstream wifstr(*argv);

      transcode_iterator<wchar_t> st(wifstr);

      wifstr >> std::noskipws;
      std::istream_iterator<wchar_t,wchar_t> ctr(wifstr);
      std::istream_iterator<wchar_t,wchar_t> cnd;
      JSONpp::json_v json = JSONpp::parse(ctr, cnd);
      std::cout << JSONpp::std_ascii << JSONpp::printer(json) << std::endl;
    } catch (std::exception& e) {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}

