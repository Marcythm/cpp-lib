class Integer {
private:
	int data;
public:
	Integer(const int &value) : data(value) {}
	Integer(const Integer &other) : data(other.data) {}
	friend bool operator==(const Integer &s, const Integer &t)
	{
		return s.data == t.data;
	}
};
