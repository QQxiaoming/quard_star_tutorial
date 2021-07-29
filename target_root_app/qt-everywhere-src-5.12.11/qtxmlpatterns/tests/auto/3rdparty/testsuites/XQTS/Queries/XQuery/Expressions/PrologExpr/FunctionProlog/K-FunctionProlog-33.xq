(:*******************************************************:)
(: Test: K-FunctionProlog-33                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A type error inside a function.              :)
(:*******************************************************:)

declare function local:myFunction()
{
	"a string" + 1
};
true()
