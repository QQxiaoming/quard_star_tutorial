(:*******************************************************:)
(: Test: K-FunctionProlog-29                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A function declaration duplicated; difference in arguments name is insignificant. :)
(:*******************************************************:)

declare function local:myName($myvar)
{
	1
};
declare function local:myName($myvar2)
{
	1
};
1
