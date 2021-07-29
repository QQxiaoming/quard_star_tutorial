(: Name: namespaceDecl-17 :)
(: Description: Verify that "abc" is a valid namespace declaration :)
(: Test was modified on 07/06/06 in order to avoid serialization :)
(: of relative URI's as part of the result.                      :)

declare namespace px = "http://www.example.com/abc";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <px:someElement>some context</px:someElement>
return
 namespace-uri-from-QName(node-name($var)) eq xs:anyURI("http://www.example.com/abc")
 