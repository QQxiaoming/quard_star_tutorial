(: Name: namespaceDecl-22 :)
(: Description: Verify that "gopher://spinaltap.micro.umn.edu/00/Weather/California/somefile" is a valid namespace URI during namespace declaration. :)

declare namespace abc = "gopher://spinaltap.micro.umn.edu/00/Weather/California/somefile";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<abc:someElement>some content</abc:someElement>