(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-9                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable depending on its self through the argument of a user function callsite. :)
(:*******************************************************:)
declare variable $local:myVar := local:myFunc(3);
declare function local:myFunc($arg)
{
local:myFunc($local:myVar)
};
$local:myVar