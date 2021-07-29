(:*******************************************************:)
(: Test: K-FunctionProlog-25                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A function and a variable can have the same name. :)
(:*******************************************************:)

declare variable $local:myName := 1;
declare function local:myName() as xs:integer
{
	1
};
$local:myName eq local:myName()
