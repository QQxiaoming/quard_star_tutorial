(: Name: namespaceDecl-21 :)
(: Description: Verify that namespace URI and prefix can contain the same value. :)
(: Test was modified on 07/06/06 in order to avoid serialization :)
(: of relative URI's as part of the result.                      :)

declare namespace abc = "http://www.example.com/abc";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <abc:someElement>some content</abc:someElement>
return
 namespace-uri-from-QName(node-name($var)) eq xs:anyURI("http://www.example.com/abc")