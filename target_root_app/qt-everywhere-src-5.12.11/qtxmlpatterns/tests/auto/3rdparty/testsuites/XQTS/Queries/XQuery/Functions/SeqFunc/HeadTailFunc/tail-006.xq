xquery version "1.1";
(:*******************************************************:)
(: Test: tail-006                                        :)
(: Written by: Michael Kay                               :)
(: Purpose: head/tail recursion                          :)
(:*******************************************************:)

declare function local:sum($n) {
  if (empty($n)) 
  then 0
  else head($n) + local:sum(tail($n))
};

local:sum(1 to 5)