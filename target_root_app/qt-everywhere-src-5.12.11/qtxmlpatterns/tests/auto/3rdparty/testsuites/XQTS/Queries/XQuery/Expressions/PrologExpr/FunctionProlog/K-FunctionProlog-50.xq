(:*******************************************************:)
(: Test: K-FunctionProlog-50                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: empty-sequence() as return type, but body doesn't match when run. :)
(:*******************************************************:)

declare function local:myFunction($arg) as empty-sequence()
{
	$arg
};
local:myFunction(1)
