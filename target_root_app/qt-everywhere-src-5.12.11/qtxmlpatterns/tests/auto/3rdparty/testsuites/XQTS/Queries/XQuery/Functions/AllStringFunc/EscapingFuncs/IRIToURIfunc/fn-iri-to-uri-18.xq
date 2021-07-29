(: Name: fn-iri-to-uri-18 :)
(: Description: Examines that the fn:iri-to-uri function does escape not the "&" symbol.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:iri-to-uri("example&amp;amp;example")