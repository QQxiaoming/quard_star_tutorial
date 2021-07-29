(:*******************************************************:)
(: Test: K-FunctionProlog-55                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A more realistic case involving fn:error(), #2. :)
(:*******************************************************:)

declare namespace my = "http://example.com/MyNamespace/";
declare variable $my:error-qname := QName("http://example.com/MyErrorNS", "my:qName");

declare function my:error($choice, $msg as xs:string) as empty-sequence()
{
	if($choice) then
		error($my:error-qname, concat('No luck: ', $msg))
	else
		()
};
my:error(false(), "msg"), my:error(true(), "The message")
