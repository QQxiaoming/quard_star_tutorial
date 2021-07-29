(:*******************************************************:)
(: Test: K-FunctionProlog-31                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: One cannot declare a user function as fn:count. :)
(:*******************************************************:)

declare function fn:count($var)
{
	fn:count($var)
};
1
