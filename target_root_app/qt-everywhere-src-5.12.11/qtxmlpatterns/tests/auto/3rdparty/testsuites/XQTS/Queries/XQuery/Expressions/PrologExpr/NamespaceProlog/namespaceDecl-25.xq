(: Name: namespaceDecl-25 :)
(: Description: Verify that "abc&apos;" is a valid namespace URI during namespace declaration. :)

declare namespace abc = "abc&apos;";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<abc:someElement>some content</abc:someElement>