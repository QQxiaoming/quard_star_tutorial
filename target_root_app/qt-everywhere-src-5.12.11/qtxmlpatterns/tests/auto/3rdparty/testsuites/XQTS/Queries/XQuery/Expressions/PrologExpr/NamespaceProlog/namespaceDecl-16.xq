(: Name: namespaceDecl-16 :)
(: Description: verify that a local namespace declaration overrides a querywide declaration. :)

declare namespace px = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <px:someElement xmlns:px = "http://www.examples.com/localexamples">some context</px:someElement>
return
 $var