(:*******************************************************:)
(: Test: K-FunctionProlog-53                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A global variable referenced from inside a function. :)
(:*******************************************************:)

declare namespace my = "http://example.com/MyNamespace/";
declare variable $my:error-qname := QName("http:example.org/", "prefix:ncname");

declare function my:error($choice, $msg as xs:string) as empty-sequence()
{
	if($choice) then
		error($my:error-qname, concat('No luck: ', $msg))
	else
		()
};
empty((my:error(false(), "msg"), my:error(false(), "The message")))
