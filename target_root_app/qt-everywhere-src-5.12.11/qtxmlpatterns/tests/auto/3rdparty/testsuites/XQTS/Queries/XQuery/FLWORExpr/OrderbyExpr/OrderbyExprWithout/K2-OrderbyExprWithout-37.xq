(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-37                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Unused order by.                             :)
(:*******************************************************:)
for $i in (1, 3, 2)
stable order by ()
return $i
