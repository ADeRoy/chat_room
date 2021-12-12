#include"common.h"
int getSeqNum()
{
  static int num = 0;
  if (num++ >= 0xEFFFFFFF - 1)
  {
    num = 0;
  }
  return num;
}

int getAccountNum()
{
    static int account = 10000;
    if (account++ >= 0xEFFFFFFF - 1)
    {
        account = 10000;
    }
    return account;
}