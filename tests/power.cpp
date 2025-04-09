DECLARE_RESOURCES

int main(int argc, char** argv) {
  INIT_MAIN

  double x = -0.8;
  double y = pow(x, 2.0);
  DPRINT(x);
  DPRINT(y);

  END_MAIN
  return 0;
}
