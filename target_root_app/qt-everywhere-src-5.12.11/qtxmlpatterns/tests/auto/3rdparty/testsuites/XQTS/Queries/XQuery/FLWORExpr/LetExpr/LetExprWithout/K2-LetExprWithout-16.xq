(:*******************************************************:)
(: Test: K2-LetExprWithout-16                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node identity is properly handled through a for binding. :)
(:*******************************************************:)
declare variable $e := <e/>; for $i in (<a/>, $e, <c/>) return $i is $e