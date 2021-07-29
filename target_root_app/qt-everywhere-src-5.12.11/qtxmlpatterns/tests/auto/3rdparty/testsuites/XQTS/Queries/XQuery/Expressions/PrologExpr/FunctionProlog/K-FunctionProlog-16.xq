(:*******************************************************:)
(: Test: K-FunctionProlog-16                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: When declaring a function, the paranteses must be present even though it doesn't have any arguments. :)
(:*******************************************************:)

declare function local:myFunction as xs:integer
{
	1
};
true()
