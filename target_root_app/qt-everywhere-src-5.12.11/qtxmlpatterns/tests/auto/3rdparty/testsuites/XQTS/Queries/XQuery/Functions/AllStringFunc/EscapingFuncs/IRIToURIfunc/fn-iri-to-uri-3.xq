(: Name: fn-iri-to-uri-3 :)
(: Description: Examines that the fn:iri-to-uri function does not escapes the "#" symbol.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:iri-to-uri("example#example")