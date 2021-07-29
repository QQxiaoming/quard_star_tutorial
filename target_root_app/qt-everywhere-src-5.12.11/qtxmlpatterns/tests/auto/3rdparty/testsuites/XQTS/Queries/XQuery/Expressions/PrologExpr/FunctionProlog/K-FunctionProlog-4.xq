(:*******************************************************:)
(: Test: K-FunctionProlog-4                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A function cannot be declared in the 'http://www.w3.org/2001/XMLSchema-instance' namespace. :)
(:*******************************************************:)

declare namespace my = "http://www.w3.org/2001/XMLSchema-instance";
declare function my:wrongNS()
{
	1
};
1
