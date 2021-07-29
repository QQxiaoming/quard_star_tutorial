(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-13                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Evaluate the boolean value of a variable.    :)
(:*******************************************************:)
declare variable $e := <e>{current-time()}</e>/(string-length(.) > 0);
$e,
if($e)
then "SUCCESS"
else "FAILURE"