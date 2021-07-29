(:*******************************************************:)
(: Test: K-FunctionProlog-51                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Too few arguments passed to a user function. :)
(:*******************************************************:)

declare function local:myFunction($arg)
{
	$arg
};
local:myFunction()
