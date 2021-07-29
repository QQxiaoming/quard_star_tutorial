(: Name: namespaceDecl-23 :)
(: Description: Verify that a namespace URI conatining "abc&amp;" is a valid namespace URI during namespace :)
(: declaration.  Test was modified on 07/06/06 in order to avoid serialization :)
(: of relative URI's as part of the result.                                    :)

declare namespace abc = "http://www.example.com/abc&amp;";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <abc:someElement>some content</abc:someElement>
return
 namespace-uri-from-QName(node-name($var)) eq xs:anyURI("http://www.example.com/abc&amp;")