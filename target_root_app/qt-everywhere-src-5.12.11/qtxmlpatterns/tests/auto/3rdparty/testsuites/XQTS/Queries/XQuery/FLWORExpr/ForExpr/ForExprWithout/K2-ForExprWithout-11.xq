(:*******************************************************:)
(: Test: K2-ForExprWithout-11                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The return expression does not depend on the for iteration. :)
(:*******************************************************:)
for $a in (3, 2, 1, 1)
stable order by $a
return 5