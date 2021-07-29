(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-21                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A sort key that doesn't affect the result.   :)
(:*******************************************************:)
let $i := (<e>1</e>, <e>3</e>, <e>2</e>)
order by 1
return $i