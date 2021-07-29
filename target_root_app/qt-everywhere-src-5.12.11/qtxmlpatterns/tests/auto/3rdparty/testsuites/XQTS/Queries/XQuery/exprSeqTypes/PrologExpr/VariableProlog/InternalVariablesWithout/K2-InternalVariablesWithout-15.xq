(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-15                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Multiple assignments is invalid.             :)
(:*******************************************************:)
declare variable $var := 1 := 2; 3