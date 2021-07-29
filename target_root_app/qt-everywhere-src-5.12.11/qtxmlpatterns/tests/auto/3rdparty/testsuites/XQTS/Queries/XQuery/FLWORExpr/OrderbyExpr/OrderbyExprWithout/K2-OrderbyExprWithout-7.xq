(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-7                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A simple sorting of integers(#2).            :)
(:*******************************************************:)
for $i in (1, 3, 2)
order by $i empty INVALID
return ($i, 2)