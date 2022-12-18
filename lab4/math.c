int count = 0;
int validate(int x) {
   int x_max = 46340; // overflow?
   return x <= x_max && x >= -x_max;
}

int square(int x) {
  if (!validate(x)) return -1;
  count++;
  return x * x;
}
