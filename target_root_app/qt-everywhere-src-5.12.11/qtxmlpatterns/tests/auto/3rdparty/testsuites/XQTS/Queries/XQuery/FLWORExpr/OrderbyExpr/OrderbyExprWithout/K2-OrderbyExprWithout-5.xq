(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-5                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A simple sorting of integers.                :)
(:*******************************************************:)
for $i in (1, 3, 2)
order by $i
return $i