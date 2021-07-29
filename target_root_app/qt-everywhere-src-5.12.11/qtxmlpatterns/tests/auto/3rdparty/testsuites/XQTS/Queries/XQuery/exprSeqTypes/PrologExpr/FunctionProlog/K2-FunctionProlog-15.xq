(:*******************************************************:)
(: Test: K2-FunctionProlog-15                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a range variable inside the assignment expression of a global variable. :)
(:*******************************************************:)
declare variable $var1 := let $var1 := 1 return 1;
$var1 eq 1