/**
* @file FlagWhenDied.h
*/
#ifndef DELEGATE_FLAGWHENDIED_H_INCLUDED
#define DELEGATE_FLAGWHENDIED_H_INCLUDED

class FlagWhenDied
{
public:
  void OnCollision(const struct Contact&);
  void SetFlagNo(int no) { flagNo = no; }
  int GetFlagNo() const { return flagNo; }

private:
  int flagNo = 0; // 操作するフラグ番号
};

#endif // DELEGATE_FLAGWHENDIED_H_INCLUDED
