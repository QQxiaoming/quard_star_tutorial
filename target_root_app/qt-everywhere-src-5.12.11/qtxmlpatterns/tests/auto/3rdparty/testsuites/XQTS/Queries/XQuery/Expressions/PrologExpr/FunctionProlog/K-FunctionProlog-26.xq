(:*******************************************************:)
(: Test: K-FunctionProlog-26                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A function declaration duplicated.           :)
(:*******************************************************:)

declare function local:myName()
{
	1
};
declare function local:myName()
{
	1
};
1
