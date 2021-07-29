(:*******************************************************:)
(: Test: K-FunctionProlog-52                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: empty-sequence() as return type, and a body containing fn:error(). :)
(:*******************************************************:)

declare function local:myFunction() as empty-sequence()
{
	fn:error()
};
local:myFunction()
