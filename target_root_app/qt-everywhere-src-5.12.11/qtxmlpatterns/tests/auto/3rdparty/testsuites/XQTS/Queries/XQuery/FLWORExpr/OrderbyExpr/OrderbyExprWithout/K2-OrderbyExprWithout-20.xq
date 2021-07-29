(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-20                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Multiple nodes as sort key trigger a type error. :)
(:*******************************************************:)
let $i := (<e>1</e>, <e>3</e>, <e>2</e>)
order by $i
return $i