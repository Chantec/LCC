### next()
if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos))
判断标识符是否存在时 使用的这个 如果是前缀一样呢？
当然了 还使用了hash 如果前缀相同的话 hash也不一样 确实是这样 
但是还是感觉符号表的设计太过简洁，一方面不好理解
先按照他这个写吧 现在没太多时间找bug了 等暑假重构一下
### global_decl()
考虑int a b;这种情况
emmmmm太消耗时间了 先不找bug了 等有空了再找
修改：及时的match(',')