(: Name: fn-iri-to-uri-26:)
(: Description: Examines that the fn:iri-to-uri with an iri that contains a space. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:iri-to-uri("example example")