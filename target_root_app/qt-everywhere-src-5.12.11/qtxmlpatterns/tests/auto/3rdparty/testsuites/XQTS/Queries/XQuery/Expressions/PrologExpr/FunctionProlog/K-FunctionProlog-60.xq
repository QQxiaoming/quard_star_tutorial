(:*******************************************************:)
(: Test: K-FunctionProlog-60                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: An argument in a user function is not in scope in the query body. :)
(:*******************************************************:)

declare function local:myFunction($arg)
{
	1
};
$arg
