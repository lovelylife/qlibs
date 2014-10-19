
#ifndef text_archive_h__
#define text_archive_h__

/*
  this class is a simple serializer for text not support binary

  example:

  struct test_struct {
    int a;
    int b;
    long c;
    std::string body;

	template<typename Archive>
	void serialize(Archive& ar)
	{
      ar & a;
      ar & b;
      ar & c;
      ar & body;
    }
  };

  // out_text for archive output
  test_struct out_text;
  out_text.a = 1;
  out_text.b = 2;
  out_text.c = 15;
  out_text.body = "test body is ok!";
 
  std::string os;
  text_oarchiver ar(os);
  ar << out_text;
  std::cout << "text_oarchiver output:\n" << os << std::endl;

  std::string is(os);
  text_iarchiver ar2(is);
  test_struct test_input;
  ar2 >> test_input;
  std::cout 
    << "" << test_input.a 
    << "\n" << test_input.b 
    << "\n" << test_input.c 
    << "\n" << test_input.body << std::endl;
  
  // code end


*/


class text_oarchiver
{
public:
	text_oarchiver(std::string& o) : o_(o) {

	}

public:
	template<typename T>
	void operator &(const T& t) {
		(*this) << t ;
		o_.append(1, (char)0xFE);
	}

	void operator &(const int& t) {
		o_.append((char*)&t, sizeof(int));
		o_.append(1, (char)0xFE);
	}

	void operator &(const long& t) {
		o_.append((char*)&t, sizeof(long));
		o_.append(1, (char)0xFE);
	}

	void operator &(const std::string& t) {
		o_+= t;
		o_.append(1, (char)0xFE);
	}

	template<typename T>
	void operator << (T& t) 
	{
		t.serialize(*this);
	}

private:
	std::string& o_;
};

class text_iarchiver
{
public:
	text_iarchiver(const std::string& i) : i_(i), last_pos_(0) {

	}

public:
	template<typename T>
	void operator &(T& t) {
		(*this) >> t;
	}

	void operator &(void*& t)
	{
		// not suport void*
		assert(0);
	}


	void operator &(int& t) {
		assert(i_.size() >= (sizeof(int)+last_pos_));
		t = *(int*)(i_.c_str() + last_pos_);
		last_pos_ = last_pos_+sizeof(int)+1;
	}

	void operator &(long& t) {
		assert(i_.size() > (sizeof(long)+last_pos_));
		t = *(long*)(i_.c_str() + last_pos_);
		last_pos_ = last_pos_+sizeof(long)+1;
	}

	void operator &(std::string& t) {
		size_t new_pos = i_.find((char)0xFE, last_pos_);
		t = i_.substr(last_pos_, new_pos-last_pos_);
		last_pos_ = new_pos+1;
	}

	template<typename T>
	void operator >> (T& t) 
	{
		t.serialize(*this);
	}

private:
	std::string i_;
	size_t last_pos_;
};

#endif

