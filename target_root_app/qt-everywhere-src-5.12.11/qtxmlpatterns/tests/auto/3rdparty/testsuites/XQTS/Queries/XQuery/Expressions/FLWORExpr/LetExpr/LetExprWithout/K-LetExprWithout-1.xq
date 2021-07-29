(:*******************************************************:)
(: Test: K-LetExprWithout-1                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A 'let' variable is out-of-scope due to FLWOR has higher precendence than the comma operator. :)
(:*******************************************************:)
let $i := 5, $j := 20 * $i
return $i, $j