(:*******************************************************:)
(: Test: K-FunctionProlog-66                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: If static typing is in use, XPTY004 is issued since local:myFunction() has static type item()*. However, if the function is invoked FOER0000 is issued. :)
(:*******************************************************:)

declare function local:myFunction()
{
	fn:error()
};
QName("http://example.com/ANamespace", local:myFunction())
