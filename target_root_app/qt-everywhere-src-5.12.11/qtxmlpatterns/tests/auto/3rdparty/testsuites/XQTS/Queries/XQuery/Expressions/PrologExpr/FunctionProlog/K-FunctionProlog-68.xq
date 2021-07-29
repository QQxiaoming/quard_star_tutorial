(:*******************************************************:)
(: Test: K-FunctionProlog-68                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A call to a user function where the argument in the callsite corresponding to a used argument contains a type error. :)
(:*******************************************************:)

declare function local:myFunction($usedArg)
{
	$usedArg
};
local:myFunction(1 + "a string")
