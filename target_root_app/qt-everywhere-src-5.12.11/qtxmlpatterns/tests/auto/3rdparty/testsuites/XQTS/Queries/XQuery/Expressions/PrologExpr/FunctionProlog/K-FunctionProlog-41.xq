(:*******************************************************:)
(: Test: K-FunctionProlog-41                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Type error in body of user function caused by the argument value. :)
(:*******************************************************:)

declare function local:myFunction($local:myVar)
{
	$local:myVar + 1
};
local:myFunction(1), local:myFunction("this will fail")
