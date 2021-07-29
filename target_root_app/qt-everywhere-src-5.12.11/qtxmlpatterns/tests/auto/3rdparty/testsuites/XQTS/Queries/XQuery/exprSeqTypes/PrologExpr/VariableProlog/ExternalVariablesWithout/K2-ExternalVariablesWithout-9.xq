(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-9                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure a variable declaration doesn't violate stability rules. :)
(:*******************************************************:)
declare variable $e := current-time();
let $i := ($e, 1 to 50000, $e)
return $i[1] = $i[last()]