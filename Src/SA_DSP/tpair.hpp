tpair::tpair()
{
}

tpair::tpair(const First & x, const Second & y)
: first(x), second(y)
{
}

int operator <(const tpair & x, const tpair & y)
{
  return x.first < y.first;
}