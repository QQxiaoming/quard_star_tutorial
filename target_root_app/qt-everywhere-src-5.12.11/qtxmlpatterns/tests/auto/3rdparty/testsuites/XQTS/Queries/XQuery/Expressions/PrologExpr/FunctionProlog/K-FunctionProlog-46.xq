(:*******************************************************:)
(: Test: K-FunctionProlog-46                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: '1' doesn't match the empty-sequence().      :)
(:*******************************************************:)

declare function local:myFunction($arg as empty-sequence())
{
	$arg
};
local:myFunction(1)
