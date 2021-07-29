(:*******************************************************:)
(: Test: K-FunctionProlog-35                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A call to a user declared function which almost is spelled correctly(#2). :)
(:*******************************************************:)

declare function local:MyFunction()
{
	1
};
local:myFunctionn()
