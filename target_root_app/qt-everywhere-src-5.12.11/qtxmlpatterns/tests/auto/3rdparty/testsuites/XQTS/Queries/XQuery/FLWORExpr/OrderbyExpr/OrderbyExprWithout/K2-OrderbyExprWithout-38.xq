(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-38                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: 'order by' combined with reverse().          :)
(:*******************************************************:)
for $i  in (1, 2, 3)
stable order by 1
return reverse(($i, "FO"))
