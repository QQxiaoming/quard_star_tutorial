(:*******************************************************:)
(: Test: K-FunctionProlog-30                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Overloading user functions based on arity.   :)
(:*******************************************************:)

declare function local:myName($var as xs:integer) as xs:integer
{
	$var
};
declare function local:myName() as xs:integer
{
	1
};
(local:myName(4) - 3)  eq local:myName()
