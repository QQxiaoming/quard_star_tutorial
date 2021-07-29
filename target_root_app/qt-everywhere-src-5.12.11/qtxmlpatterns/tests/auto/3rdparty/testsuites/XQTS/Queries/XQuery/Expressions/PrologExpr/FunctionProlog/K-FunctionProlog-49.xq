(:*******************************************************:)
(: Test: K-FunctionProlog-49                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: It is valid to declare an argument to be of type empty-sequence(). :)
(:*******************************************************:)

declare function local:myFunction($arg as empty-sequence())
{
	$arg
};
empty(local:myFunction(()))
