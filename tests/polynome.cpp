DECLARE_RESOURCES

int main(void)
{
  INIT_MAIN
  float x,y,z,t;

  x = FBETWEEN(0,1);
  y = (x-1)*(x-1)*(x-1)*(x-1);
  FAFFPRINT(y);
  FAFFPRINT(x);
  z = x*x;
  FAFFPRINT(z);
  z = z*z - 4*x*z + 6*z - 4*x + 1;
  FAFFPRINT(z);
  t = z-y;
  FAFFPRINT(t);

  END_MAIN
  return 0;
}
