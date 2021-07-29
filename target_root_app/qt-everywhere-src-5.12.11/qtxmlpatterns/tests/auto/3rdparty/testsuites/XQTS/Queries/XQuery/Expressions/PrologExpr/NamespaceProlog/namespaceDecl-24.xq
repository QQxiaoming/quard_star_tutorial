(: Name: namespaceDecl-24 :)
(: Description: Verify that "mailto:someuser@someserver.com" is a valid namespace URI during namespace declaration. :)

declare namespace abc = "mailto:someuser@someserver.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<abc:someElement>some content</abc:someElement>