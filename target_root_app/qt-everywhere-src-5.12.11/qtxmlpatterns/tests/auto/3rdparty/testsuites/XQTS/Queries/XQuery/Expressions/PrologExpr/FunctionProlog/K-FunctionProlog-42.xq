(:*******************************************************:)
(: Test: K-FunctionProlog-42                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Type error(cardinality) in return value of user function caused by the argument value. :)
(:*******************************************************:)

declare function local:myFunction($local:arg) as item()
{
	1, $local:arg
};
local:myFunction(()), local:myFunction(1)
