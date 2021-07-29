(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-9                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Ensure a variable declaration doesn't violate stability rules. :)
(:*******************************************************:)
declare variable $e := current-time();
let $i := ($e, 1 to 50000, $e)
return $i[1] treat as xs:time = $i[last()] treat as xs:time
