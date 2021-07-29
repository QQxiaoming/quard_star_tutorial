(:*******************************************************:)
(: Test: K2-LetExprWithout-26                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An element() test whose cardinality doesn't match. :)
(:*******************************************************:)
let $e := <element/>,
$outer as element() := $e/element()
return $outer