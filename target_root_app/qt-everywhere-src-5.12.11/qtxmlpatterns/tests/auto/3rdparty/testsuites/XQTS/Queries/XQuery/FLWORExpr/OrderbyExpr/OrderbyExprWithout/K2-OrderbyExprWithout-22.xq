(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-22                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: order by preceded by an unused let binding.  :)
(:*******************************************************:)
for $i in (1, 3, 2)
let $c := 3
stable order by ()
return $i
