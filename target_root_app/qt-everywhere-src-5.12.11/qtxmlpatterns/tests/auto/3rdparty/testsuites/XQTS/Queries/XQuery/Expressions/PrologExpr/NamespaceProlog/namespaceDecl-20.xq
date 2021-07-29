(: Name: namespaceDecl-20 :)
(: Description: Verify that namespace URI may contain a number. :)
(: Test was modified on 07/06/06 in order to avoid serialization :)
(: of relative URI's as part of the result.                      :)

declare namespace xx = "http://www.example.com/abc123";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <xx:someElement>some content</xx:someElement>
return
 namespace-uri-from-QName(node-name($var)) eq xs:anyURI("http://www.example.com/abc123")