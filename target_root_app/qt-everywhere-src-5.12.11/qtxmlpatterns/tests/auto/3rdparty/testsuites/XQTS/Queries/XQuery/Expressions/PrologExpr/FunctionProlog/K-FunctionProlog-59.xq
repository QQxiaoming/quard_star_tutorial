(:*******************************************************:)
(: Test: K-FunctionProlog-59                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A call to a user function where the argument in the callsite corresponding to an unused argument contains a type error. :)
(:*******************************************************:)

declare function local:myFunction($unusedArg)
{
	true()
};
local:myFunction(1 + "a string")
