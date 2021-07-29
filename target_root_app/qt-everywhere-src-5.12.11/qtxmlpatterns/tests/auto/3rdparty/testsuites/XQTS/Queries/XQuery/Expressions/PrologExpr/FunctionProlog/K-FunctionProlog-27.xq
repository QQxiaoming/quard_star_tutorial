(:*******************************************************:)
(: Test: K-FunctionProlog-27                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A function declaration duplicated; difference in return types is insignificant. :)
(:*******************************************************:)

declare function local:myName() as xs:integer
{
	1
};
declare function local:myName() as xs:nonPositiveInteger
{
	1
};
1
