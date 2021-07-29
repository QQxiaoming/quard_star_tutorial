(:*******************************************************:)
(: Test: K-FunctionProlog-11                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A user declared function whose return value simply doesn't match the return type(#2). :)
(:*******************************************************:)

declare function local:myFunction() as item()
{
	()
};
local:myFunction()
